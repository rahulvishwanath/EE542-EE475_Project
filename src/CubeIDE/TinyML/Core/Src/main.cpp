/**
 * @file main.cpp
 * @brief Main application entry point for the STM32 TinyML system.
 *
 * This file initializes system peripherals, creates FreeRTOS tasks,
 * and starts the scheduler. It handles IMU data collection, classification,
 * and haptic feedback.
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "global.h"
#include "sensorTask.h"
#include "hapticTask.h"

/* Hardware Constants */
#define MPU6500_ADDR 0xD0          		///< I2C address of the MPU6500 IMU
#define WHO_AM_I_REG 0x75          		///< WHO_AM_I register (should return 0x70)
#define PWR_MGMT_REG 0x6B          		///< Power management register
#define SMPRT_DIV_REG 0x19         		///< Sample rate divider register
#define ACCEL_CONFIG_REG 0x1C      		///< Accelerometer configuration register
#define GYRO_CONFIG_REG 0x1B      		///< Gyroscope configuration register
#define CLASSIFICATION_THRESHOLD 0.6f 	///< Confidence threshold for classification

/* Function Prototypes */
void MX_GPIOD_Init(void);
void MX_I2C1_Init(void);
void MX_USART2_UART_Init(void);
void ERROR_LED(int);
void MPU6500_Init(void);
static void MX_CRC_Init(void);

/* Global Variables */
I2C_HandleTypeDef hi2c1;				///< I2C handle for IMU and haptic driver
UART_HandleTypeDef huart2;				///< UART handle for debugging messages and transmitting guesture
CRC_HandleTypeDef hcrc;					///< CRC handle for Edge Impulse model validation

QueueHandle_t hapticQueue;				///< Queue to trigger haptic feedback
SemaphoreHandle_t classifierSemaphore;	///< Semaphore to synchronize classification task
bool debug_flag = false;   				// Debug flag for print/UART statements for debugging

/**
 * @brief Main entry point of the application.
 *
 * Initializes hardware peripherals, creates FreeRTOS tasks,
 * and starts the scheduler.
 *
 * @param None
 * @return int This function never returns (FreeRTOS scheduler takes over).
 */
int main()
{
	// Init HAL, needs to be called at the very beginning to call any other HAL functions
	HAL_Init();
	MX_GPIOD_Init();		// Configure the GPIO pins
	MX_I2C1_Init(); 		// Configure I2C1 for MPU6500 & DRV2605 PB6 (SCL) and PB7 (SDA)
	MX_USART2_UART_Init(); 	// Configure UART2 PA3(RX) and PA2(TX)
	MX_CRC_Init();			// Enable CRC for TinyML classifier
	MPU6500_Init();			// Initialize the IMU sensor

	HAL_StatusTypeDef status;

	// Initialize the haptic driver
	status = DRV2605_Init(&hi2c1);

	char msg[50];
	if (status != HAL_OK)
	{
		ERROR_LED(250);
		if (debug_flag)
		{
			snprintf(msg, sizeof(msg), "Init failed\r\n");
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
		}
		while (1); // Halt execution
	}

	// Create hapticQueue for haptic feedback task
	hapticQueue = xQueueCreate(4, sizeof(char));

	// Create semaphore for classifier synchronization
	classifierSemaphore = xSemaphoreCreateBinary();
	if (classifierSemaphore == NULL) {
		ERROR_LED(250);
		if (debug_flag)
		{
		snprintf(msg, sizeof(msg), "ERROR: Failed to create classifierSemaphore!\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
		}
		while (1); // Halt execution
	}

	// Create FreeRTOS tasks
	xTaskCreate (sensorTask, "Sensor Task",(1024),0,2,0);
	xTaskCreate (classifierTask, "Classifier Task",(3*1024),0,1,0);
	xTaskCreate (hapticTask, "Haptic Task",(400),0,1,0);

	if (debug_flag)
	{
		snprintf(msg, sizeof(msg), "Starting Task Scheduler\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
	}

	vTaskStartScheduler(); // This function never returns

	while (1) {}	// Infinite loop, should never be reached
}

/**
 * @brief Initializes GPIO for LED control.
 *
 * Configures GPIO pin PD15 for output to indicate errors.
 * @param None
 * @retval None
 */
void MX_GPIOD_Init(void)
{
	// Enable the GPIOD clk using HAL
	__HAL_RCC_GPIOD_CLK_ENABLE();

	// GPIOD config
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	// Init GPIO pin 14 with HAL
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
 * @brief Initializes I2C1 for MPU6500 and DRV2605 communication.
 *
 * Configures PB6 (SCL) and PB7 (SDA) for I2C1.
 * @param None
 * @retval None
 */
void MX_I2C1_Init(void)
{
	// Enable I2C1 Clock
	__HAL_RCC_I2C1_CLK_ENABLE();

	// Enable GPIOB Clock
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Configure PB6 (SCL) and PB7 (SDA) for I2C1 in Alternate Function mode
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;  			// Open-Drain for I2C
	GPIO_InitStruct.Pull = GPIO_PULLUP;      			// Enable pull-ups
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1; 			// AF4 is I2C1

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Configure the I2C peripheral
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000; 					// Standard mode 100kHz
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0x00; 						// Master mode, no own address
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		ERROR_LED(250);
		while (1); // Halt execution
	}
}

/**
 * @brief Initializes UART2 for debugging output.
 * @param None
 * @retval None
 */
void MX_USART2_UART_Init(void) {
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// PA2 (TX) and PA3 (RX) in AF mode
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Configure UART
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		ERROR_LED(250);
		while (1); // Halt execution
	}
}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void)
{
	hcrc.Instance = CRC;
	if (HAL_CRC_Init(&hcrc) != HAL_OK)
	{
		ERROR_LED(250);
		while (1); // Halt execution
	}
}
/**
 * @brief Initializes the MPU6500 IMU sensor.
 * @param None
 * @retval None
 */
void MPU6500_Init(void)
{
	uint8_t check = 0;
	uint8_t Data;
	char msg[50];

	// Read WHO_AM_I register
	HAL_I2C_Mem_Read(&hi2c1, MPU6500_ADDR, WHO_AM_I_REG, 1, &check, 1, 100);

	snprintf(msg, sizeof(msg), "WHO_AM_I: 0x%02X\r\n", check);
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

	// MPU6500 address is 0x68 but WHO_AM_I reg should be 0x70
	if (check != 0x70)
	  ERROR_LED(100);

	// Wake the sensor up
	Data = 0;
	HAL_I2C_Mem_Write(&hi2c1, MPU6500_ADDR, PWR_MGMT_REG, 1, &Data, 1, 1000);

	// Configure Sample Rate, Sample Rate = Gyro Output Rate / (1 + SMPRT_DIV value)
	// Gyro Output Rate = 8kHz, to get a 1kHz smae rate we need a SMPRT_DIV value of 7
	Data = 0x07;
	HAL_I2C_Mem_Write(&hi2c1, MPU6500_ADDR, SMPRT_DIV_REG, 1, &Data, 1, 1000);

	// Accelerometer configuration
	// XA_ST, YA_ST, ZA_ST, FS_SEL = 0 -> +-2g
	Data = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MPU6500_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1, 1000);

	// Gyroscope configuration
	// XG_ST, YG_ST, ZG_ST, FS_SEL = 0 -> +-250* /s
	Data = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MPU6500_ADDR, GYRO_CONFIG_REG, 1, &Data, 1, 1000);
}


/**
 * @brief Function to indicate an error state using an LED.
 *
 * This function blinks an LED indefinitely to indicate a system failure.
 * It is used in various initialization functions when a critical failure occurs.
 *
 * @param delay Blink delay in milliseconds.
 */
void ERROR_LED(int delay)
{
  // Initialization Error
  while (1)
  {
    // Toggle Pin 14 (LED)
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
    // Delay for 1 second
    HAL_Delay(delay);
  }
}

