/**
 * @file sensorTask.cpp
 * @brief FreeRTOS task for IMU sensor data acquisition and TinyML classification.
 *
 * This file handles accelerometer and gyroscope data collection from the MPU6500.
 * It buffers data in a circular buffer and triggers classification using
 * an Edge Impulse model. If an action is detected, it signals the haptic task.
 */

#include "global.h"
#include "task.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

/* MPU6500 IMU Registers */
#define MPU6500_ADDR 0xD0					///< I2C address for MPU6500
#define ACCEL_START_REG 0x3B				///< Starting register for accelerometer readings


/* Sampling Parameters */
#define SAMPLING_RATE_HZ 100                 		///< Sampling rate (Hz)
#define SAMPLING_PERIOD_ms 1000/SAMPLING_RATE_HZ	///< Period between samples
#define TEST_DURATION_SECONDS 2.5          			///< Duration for collecting a sample batch
#define TOTAL_SAMPLES (static_cast<int>(SAMPLING_RATE_HZ * TEST_DURATION_SECONDS))	///< Number of samples per batch
#define BUFFER_MARGIN_SECONDS 0.3   				///< Extra time buffer for sampling
#define BUFFER_SIZE (static_cast<int>(6 * SAMPLING_RATE_HZ * (TEST_DURATION_SECONDS + BUFFER_MARGIN_SECONDS)))	 ///< Circular buffer size


using namespace ei;

signal_t ei_signal;							///< Edge Impulse signal struct for classification
float imu_circular_buffer[BUFFER_SIZE];   	///< Circular buffer for IMU data
volatile int write_index = 0;  				///< Points to the next write location in the buffer
TickType_t xLastWakeTime;					///< Timestamp for accurate periodic sampling

/**
 * @brief Custom UART printf function for Edge Impulse logs.
 *
 * @param fmt Format string.
 * @param argp Variable argument list.
 */
void vprint(const char *fmt, va_list argp)
{
    char string[200];
    if(0 < vsprintf(string, fmt, argp)) // build string
    {
    	//strcat(string, "\r");  // Ensure newlines are properly handled
        HAL_UART_Transmit(&huart2, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
    }
}

/**
 * @brief Prints formatted output to the UART console.
 *
 * @param format Format string.
 * @param ... Additional arguments.
 */
void ei_printf(const char *format, ...) {
    va_list myargs;
    va_start(myargs, format);
    vprint(format, myargs);
    va_end(myargs);
}

/**
 * @brief Reads accelerometer data from MPU6500.
 *
 * Reads raw 16-bit values and converts them to floating-point format.
 *
 * @param A Pointer to store acceleration data [Ax, Ay, Az].
 */
void MPU6500_Read_Accel(float *A)
{
  uint8_t Accel_Data[6];
  int16_t Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW;

  // Read 6 bytes of data starting at 0x3B to 0x40
  HAL_I2C_Mem_Read(&hi2c1, MPU6500_ADDR, ACCEL_START_REG, 1, Accel_Data, 6, 1000);

  // 0x3B = ACCEL_X_OUT_H, 0x3C = ACCEL_X_OUT_L
  Accel_X_RAW = (int16_t)(Accel_Data[0] << 8 | Accel_Data[1]);
  // 0x3D = ACCEL_Y_OUT_H, 0x3E = ACCEL_Y_OUT_L
  Accel_Y_RAW = (int16_t)(Accel_Data[2] << 8 | Accel_Data[3]);
  // 0x3F = ACCEL_Z_OUT_H, 0x40 = ACCEL_Z_OUT_L
  Accel_Z_RAW = (int16_t)(Accel_Data[4] << 8 | Accel_Data[5]);

  A[0] = (float)Accel_X_RAW;
  A[1] = (float)Accel_Y_RAW;
  A[2] = (float)Accel_Z_RAW;
}

/**
 * @brief Reads gyroscope data from MPU6500.
 *
 * Reads raw 16-bit values and converts them to floating-point format.
 *
 * @param G Pointer to store gyroscope data [Gx, Gy, Gz].
 */
void MPU6500_Read_Gyro(float *G)
{
  uint8_t Gyro_Data[6];
  int16_t Gyro_X_RAW, Gyro_Y_RAW, Gyro_Z_RAW;

  // Read 6 bytes of data starting at 0x3B to 0x40
  HAL_I2C_Mem_Read(&hi2c1, MPU6500_ADDR, ACCEL_START_REG, 1, Gyro_Data, 6, 1000);

  // 0x3B = ACCEL_X_OUT_H, 0x3C = ACCEL_X_OUT_L
  Gyro_X_RAW = (int16_t)(Gyro_Data[0] << 8 | Gyro_Data[1]);
  // 0x3D = ACCEL_Y_OUT_H, 0x3E = ACCEL_Y_OUT_L
  Gyro_Y_RAW = (int16_t)(Gyro_Data[2] << 8 | Gyro_Data[3]);
  // 0x3F = ACCEL_Z_OUT_H, 0x40 = ACCEL_Z_OUT_L
  Gyro_Z_RAW = (int16_t)(Gyro_Data[4] << 8 | Gyro_Data[5]);

  G[0] = (float)Gyro_X_RAW;
  G[1] = (float)Gyro_Y_RAW;
  G[2] = (float)Gyro_Z_RAW;
}

/**
 * @brief FreeRTOS task to collect IMU sensor data.
 *
 * Reads accelerometer and gyroscope data at a fixed sampling rate.
 * Stores the data in a circular buffer and triggers classification
 * when enough samples are collected.
 *
 * @param arg Unused parameter (for FreeRTOS compatibility).
 */
void sensorTask(void *arg)
{
	(void) arg;
	xLastWakeTime = xTaskGetTickCount();
	float G[3];			///< Gyroscope data
	float A[3];			///< Accelerometer data
	int samples_collected = 0;

	while (1)
	{
		MPU6500_Read_Accel(A);
		MPU6500_Read_Gyro(G);

		// Store IMU data in circular buffer
		imu_circular_buffer[write_index + 0] = A[0];
		imu_circular_buffer[write_index + 1] = A[1];
		imu_circular_buffer[write_index + 2] = A[2];
		imu_circular_buffer[write_index + 3] = G[0];
		imu_circular_buffer[write_index + 4] = G[1];
		imu_circular_buffer[write_index + 5] = G[2];
		write_index = (write_index + 6) % BUFFER_SIZE;

		samples_collected++;

		// Check if enough samples are ready for classification
		if (samples_collected >= TOTAL_SAMPLES)
		{
			samples_collected = 0; // Reset count after sending data
			xSemaphoreGive(classifierSemaphore); // Notify classifierTask
		}

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SAMPLING_PERIOD_ms));
	}
}

/**
 * @brief FreeRTOS task to classify IMU data using Edge Impulse.
 *
 * When enough samples are collected, this function runs classification
 * using the Edge Impulse model and triggers haptic feedback if an action
 * other than "idle" is detected.
 *
 * @param arg Unused parameter (for FreeRTOS compatibility).
 */
void classifierTask(void *arg)
{
	(void) arg;
	float temp_data[6 * TOTAL_SAMPLES];  ///< Temporary buffer for classification
	int gesture_trigger;

	while (1)
	{

		xSemaphoreTake(classifierSemaphore, portMAX_DELAY);  // Wait for sensor data

		// Find the start index for the last N samples
		int read_index = (write_index - (6 * TOTAL_SAMPLES) + BUFFER_SIZE) % BUFFER_SIZE;

		// Copy the latest data into temp_data
		for (int i = 0; i < (6 * TOTAL_SAMPLES); i++) {
			temp_data[i] = imu_circular_buffer[(read_index + i) % BUFFER_SIZE];
		}

		// Prepare the signal for Edge Impulse classifier
		ei_signal.total_length = (unsigned int)6 * TOTAL_SAMPLES;
		ei_signal.get_data = [&temp_data](size_t offset, size_t length, float *out_ptr) {
			memcpy(out_ptr, temp_data + offset, length * sizeof(float));
			return 0;
		};

		// Run Edge Impulse classifier
		ei_impulse_result_t result = { 0 };
		EI_IMPULSE_ERROR res = run_classifier(&ei_signal, &result, false);

		// Find best classification result
		float highest_value = -1.0f;
		int best_index = 0;
		for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
			if (result.classification[ix].value > highest_value) {
				highest_value = result.classification[ix].value;
				best_index = ix;
			}
		}

		// Print only the best prediction
		ei_printf("%s", result.classification[best_index].label);
		ei_printf("\r\n");

		if (strcmp(result.classification[best_index].label, "idle") != 0) {
			// Here you could map the prediction to a particular haptic effect.
			// For simplicity, we simply send a nonzero trigger.
			gesture_trigger = 1;
			xQueueSend(hapticQueue, &gesture_trigger, portMAX_DELAY);
		}

	}
}

