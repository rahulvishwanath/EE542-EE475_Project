/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdarg.h>
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

using namespace ei;

// paste the raw features here
static const float features[] = { -40.9800, -40.8900, 107.1500, -44.7600, -43.9100, 110.8100, -37.5900,
		-41.4700, 107.1800, -39.4200, -42.7200, 109.5900, -38.0800, -41.7700, 110.6600, -35.2400, -44.3400,
		107.1500, -43.8500, -45.8300, 111.2700, -36.5200, -44.5800, 110.8700, -45.1600, -48.0900, 114.9600,
		-37.1600, -46.6600, 114.6300, -39.9100, -46.6000, 116.1800, -31.6000, -45.3100, 112.2400, -38.4400,
		-46.8100, 115.7900, -44.1800, -50.4700, 119.0200, -52.8200, -51.5100, 121.5000, -53.7400, -53.0100,
		123.8500, -49.6800, -49.1000, 121.9800, -46.6900, -46.6000, 121.9200, -53.9200, -46.7800, 122.3800,
		-51.1500, -44.4000, 127.1800, -35.8500, -40.0900, 121.5300, -48.0300, -35.3600, 120.2400, -55.4200,
		-46.2000, 128.8500, -60.3700, -50.3200, 126.9000, -41.5300, -43.7600, 134.3500, -13.5300, -28.0000,
		120.0600, 4.1200, -20.7900, 115.3300, 18.7800, -21.3700, 112.5500, 36.5200, -22.4700, 109.4700, 40.9500,
		-26.5600, 113.3700, 50.8400, -26.0200, 111.5700, 74.3500, -17.4400, 106.3500, 130.5300, 3.4800, 92.6100,
		141.2200, 19.0800, 75.0500, 117.7100, 14.0500, 69.6800, 121.0700, 13.3700, 73.7700, 60.1800, -29.7400,
		64.7000, 10.1100, -52.2700, 71.7600, 5.7100, -61.7700, 62.7500, -45.1900, -69.5300, 65.5600, -98.9900,
		-76.7900, 67.9400, -117.8600, -65.5600, 76.4600, -152.3400, -70.0800, 88.7600, -167.1100, -62.7200, 94.9300,
		-177.8600, -57.1900, 101.1600, -194.1700, -58.0200, 114.0800, -199.4500, -64.7000, 116.5800, -195.6000, -77.9200,
		117.0100, -185.5300, -93.7400, 124.5800, -159.8800, -99.6000, 115.4200, -157.3700, -99.1800, 106.1100, -162.2000,
		-99.2100, 111.7600, -181.3100, -101.5000, 120.3700, -153.6200, -91.5400, 120.6100, -134.5600, -86.2300, 102.9600,
		-116.9800, -76.9500, 102.5000, -87.6300, -58.1700, 88.7600, -59.8800, -43.8500, 81.1600, -55.6900, -26.5300,
		78.7200, -19.3900, -8.1200, 74.0800, 14.4400, 6.9900, 73.0400, 45.8300, 14.3200, 72.8900, 66.8700, 16.7600,
		72.2700, 89.4700, 10.7800, 78.1400, 102.3800, -2.8400, 77.6500, 119.8200, 6.3500, 78.4400, 160.3100, 20.3400,
		64.7900, 189.8900, 25.8300, 57.1300, 154.6000, 31.2100, 48.4300, 125.9500, 5.1600, 54.8700, 85.1900, -24.1800,
		54.6300, 46.2300, -43.0800, 49.3100, 4.6400, -69.5900, 47.9700, -56.5200, -95.0800, 66.5300, -92.1500, -107.5100,
		71.8800, -124.8200, -104.6400, 81.2200, -145.7100, -84.2700, 83.0800, -170.3500, -66.7800, 84.4600, -199.1100,
		-52.5200, 96.0300, -220.8200, -46.1700, 106.0500, -231.9100, -46.5300, 116.0900, -224.0000, -46.0800, 112.4300,
		-198.5000, -51.3000, 106.9600, -164.3100, -53.7700, 101.7400, -144.1200, -55.1800, 98.7200, -126.3500, -58.9000,
		98.1700, -114.4700, -62.0500, 99.2700, -111.6300, -65.3700, 100.4900, -106.5300, -66.2600, 99.8200, -102.0200,
		-62.1700, 101.7700, -81.0400, -52.2400, 96.0000, -78.6600, -43.8500, 93.4700, -76.3400, -40.8500, 91.4200,
		-72.0300, -43.8500, 96.1800, -76.5200, -43.6900, 95.2700, -74.6900, -43.6600, 101.6800, -62.2900, -42.1400,
		100.6400, -54.5600, -36.5800, 96.7000, -61.7100, -40.3400, 101.4000, -58.7200, -41.9200, 103.5400, -43.1800,
		-42.2000, 103.5700, -37.9200, -36.4900, 100.3700, -36.1200, -38.7200, 101.8000, -36.5800, -37.5300, 106.2900,
		-26.1700, -37.8300, 103.6900, -25.8000, -38.3500, 104.1800, -36.5200, -44.7300, 107.1500, -25.0400, -42.4400,
		108.4900, -20.4300, -44.3700, 107.3300, -22.7200, -46.9000, 105.5900, -24.5500, -49.6500, 108.3700, -26.3500,
		-51.1500, 110.8400, -26.7200, -50.6900, 112.0900, -30.9300, -53.0400, 112.8200, -31.3600, -53.5000, 113.7100,
		-31.6300, -52.3100, 112.1500, -30.9300, -49.5900, 111.4500, -32.8900, -44.9200, 112.9200, -35.6300, -44.6700,
		113.0700, -38.4400, -44.5500, 113.0100, -39.1500, -46.4100, 112.5800, -41.3700, -44.9200, 112.4000, -45.0700,
		-45.9500, 113.3100, -46.5000, -46.9000, 113.8300, -53.4400, -47.6600, 115.5700, -53.6800, -46.6300, 116.1800,
		-48.7000, -43.2100, 114.2600, -49.0700, -45.5600, 113.7100, -45.5300, -45.3100, 114.0800, -45.8900, -43.8500,
		113.5000, -45.3100, -43.6900, 112.9200, -44.3400, -43.7900, 112.5200, -49.6800, -48.3100, 114.4400, -47.9400,
		-47.6600, 113.4400, -49.5900, -47.2700, 113.3400, -48.8900, -47.6300, 114.0200, -50.5300, -47.6600, 114.0800,
		-50.9300, -46.6600, 115.1100, -50.9300, -44.7900, 114.7200, -55.3000, -47.8500, 114.1400, -51.2400, -46.2600,
		114.2000, -47.9100, -45.8000, 112.5800, -47.6000, -46.1400, 113.1600, -48.7300, -46.9600, 112.9200, -47.2700,
		-47.8800, 114.0500, -44.4000, -46.3200, 112.4300, -46.9600, -48.0000, 112.4600, -52.5500, -50.0800, 113.4000,
		-47.4800, -47.6300, 111.7900, -48.3700, -50.1100, 112.7900, -47.4500, -51.5400, 112.1800, -47.7300, -51.5700,
		112.1200, -45.2200, -52.3700, 111.7600, -44.2100, -52.4000, 110.7800, -46.7800, -52.6700, 109.3100, -51.2700,
		-52.4000, 110.7200, -49.1600, -49.9500, 109.1900, -51.2700, -49.5900, 109.9800, -50.8400, -48.7000, 110.0800,
		-52.0900, -51.5100, 109.5600, -51.0500, -51.8800, 108.9800, -50.4400, -51.8800, 108.9200, -50.8100, -51.4800,
		107.7300, -48.8500, -49.1300, 107.8200, -48.9500, -49.3400, 105.5900, -49.1300, -49.4400, 105.1300, -47.0200,
		-49.6200, 104.0300, -47.3300, -50.6600, 103.5400, -48.1500, -53.1000, 102.6900, -49.5000, -52.8500, 102.2000,
		-47.5400, -51.3600, 98.6600, -54.3800, -54.0200, 100.1800, -49.4000, -51.1800, 98.1400, -47.7300, -49.6500, 97.5900,
		-44.0000, -50.8100, 96.8500, -43.1800, -53.7100, 96.1200, -40.5200, -52.2700, 96.0600, -40.4600, -52.0000, 95.0800,
		-40.2400, -52.3100, 96.8200, -43.3300, -53.2200, 96.3700, -42.8100, -53.4400, 97.1000, -43.9700, -53.0400, 97.5900,
		-37.1300, -48.2100, 97.9200, -42.0800, -49.3700, 101.2500, -40.9800, -48.2100, 101.2500, -46.3200, -50.3200, 106.6900,
		-45.5900, -50.8400, 107.4800, -47.0200, -50.4400, 108.2700, -48.9200, -52.0900, 109.8000, -47.8800, -50.2900,
		110.6000, -48.8200, -49.7700, 110.9000, -51.5100, -49.5600, 111.2400, -50.4700, -49.9200, 110.9000, -56.7000,
		-51.6900, 112.6700, -60.4000, -52.0600, 114.4700, -57.2200, -50.7500, 115.4800, -54.2000, -47.3900, 115.0500,
		-54.2600, -44.8200, 114.7800, -58.6600, -45.1000, 115.4500, -66.3500, -46.1700, 116.1200, -66.8100, -42.9900,
		116.6100, -67.9700, -39.7300, 115.2400, -65.7700, -36.8500, 115.6000, -61.2500, -35.5100, 115.1500, -56.7300,
		-32.6100, 114.0800, -54.7800, -30.4400, 113.5000, -52.1800, -29.7400, 112.7000, -50.7500, -31.4800, 113.2800,
		-51.4200, -29.3400, 113.1900, -50.8700, -29.6500, 113.7700, -59.2700, -33.4400, 115.6300, -64.9500, -35.2700,
		117.6200, -66.9300, -35.2100, 119.2400, -66.2300, -33.2200, 119.3600, -59.7600, -30.1700, 119.3300, -60.8900,
		-27.7600, 118.1700, -57.8900, -26.3200, 117.5300, -49.6500, -20.8900, 117.6500, -49.9200, -17.8900, 118.9000,
		-49.9200, -14.1700, 119.4800, -51.9400, -14.9000, 121.7400, -58.0500, -19.6600, 125.2500
};

int get_feature_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s3;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);
static void MX_SPI1_Init(void);
static void MX_CRC_Init(void);
void MX_USB_HOST_Process(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void vprint(const char *fmt, va_list argp)
{
    char string[200];
    if(0 < vsprintf(string, fmt, argp)) // build string
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
    }
}

void ei_printf(const char *format, ...) {
    va_list myargs;
    va_start(myargs, format);
    vprint(format, myargs);
    va_end(myargs);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  signal_t signal;
  signal.total_length = sizeof(features) / sizeof(features[0]);
  signal.get_data = &get_feature_data;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USB_HOST_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */
  // Configure PA3(RX) and PA2(TX) for USART2
    MX_USART2_UART_Init();
  /* USER CODE END 2 */

    // Buffer to send through UART
    char msg[128];
    snprintf(msg, sizeof(msg), "Setup successfull\r\n");

    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    //MX_USB_HOST_Process();
	  ei_impulse_result_t result = { 0 };
	       EI_IMPULSE_ERROR res = run_classifier(&signal, &result, true);
	       ei_printf("run_classifier returned: %d\n", res);

	       ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
	           result.timing.dsp, result.timing.classification, result.timing.anomaly);

	       // print the predictions
	       ei_printf("[");
	       for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
	     	  ei_printf_float(result.classification[ix].value);
	   #if EI_CLASSIFIER_HAS_ANOMALY == 1
	           ei_printf(", ");
	   #else
	           if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
	               ei_printf(", ");
	           }
	   #endif
	       }
	   #if EI_CLASSIFIER_HAS_ANOMALY == 1
	       ei_printf_float(result.anomaly);
	   #endif
	       ei_printf("]\n\n\n");

	     HAL_Delay(5000);

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_96K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// UART Initialization (USART2 for USB output)
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
    	Error_Handler();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
