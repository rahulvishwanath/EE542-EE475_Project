/*
 * sensorTask.c
 *
 *  Created on: Mar 6, 2025
 *      Author: EE475
 */
#include "global.h"
#include "task.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

#define MPU6500_ADDR 0xD0
#define ACCEL_START_REG 0x3B

// Sampling Defines
#define SAMPLING_RATE_HZ 100                // 111 samples per second
#define SAMPLING_PERIOD_ms 1000/SAMPLING_RATE_HZ
#define TEST_DURATION_SECONDS 2.5          // Duration of the test in seconds
#define TOTAL_SAMPLES (static_cast<int>(SAMPLING_RATE_HZ * TEST_DURATION_SECONDS))
#define BUFFER_MARGIN_SECONDS 0.3  // Add extra time buffer
#define BUFFER_SIZE (static_cast<int>(6 * SAMPLING_RATE_HZ * (TEST_DURATION_SECONDS + BUFFER_MARGIN_SECONDS)))


using namespace ei;

signal_t ei_signal;
float imu_circular_buffer[BUFFER_SIZE];  // Circular buffer for IMU data
volatile int write_index = 0;  // Points to the next write location
TickType_t xLastWakeTime;


void vprint(const char *fmt, va_list argp)
{
    char string[200];
    if(0 < vsprintf(string, fmt, argp)) // build string
    {
    	//strcat(string, "\r");  // Ensure newlines are properly handled
        HAL_UART_Transmit(&huart2, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
    }
}

void ei_printf(const char *format, ...) {
    va_list myargs;
    va_start(myargs, format);
    vprint(format, myargs);
    va_end(myargs);
}

// Reads Acceleration Data from MPU6500, converts to 'g' format and stores in A
// A = [Ax, Ay, Az]
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

// Reads Gyro Data from MPU6500, converts to dps (degrees per second) format and stores in G
// G = [Gx, Gy, Gz]
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


void sensorTask(void *arg)
{
	(void) arg;
	xLastWakeTime = xTaskGetTickCount();
	// Acceleration and Gyro Data
	float G[3];
	float A[3];
	int samples_collected = 0;

	while (1)
	{
		MPU6500_Read_Accel(A);
		MPU6500_Read_Gyro(G);

		// Write IMU data into the circular buffer
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

void classifierTask(void *arg)
{
	(void) arg;
	float temp_data[6 * TOTAL_SAMPLES];  // Temporary buffer
	int gesture_trigger;

	while (1)
	{

		xSemaphoreTake(classifierSemaphore, portMAX_DELAY);  // Wait until sensorTask signals enough data is ready
		// Find the starting index for the last N samples
		int read_index = (write_index - (6 * TOTAL_SAMPLES) + BUFFER_SIZE) % BUFFER_SIZE;

		// Copy the latest data into temp_data
		for (int i = 0; i < (6 * TOTAL_SAMPLES); i++) {
			temp_data[i] = imu_circular_buffer[(read_index + i) % BUFFER_SIZE];
		}

		// Prepare the signal for classification
		ei_signal.total_length = (unsigned int)6 * TOTAL_SAMPLES;
		ei_signal.get_data = [&temp_data](size_t offset, size_t length, float *out_ptr) {
			memcpy(out_ptr, temp_data + offset, length * sizeof(float));
			return 0;
		};
		// Run Edge Impulse classifier
		ei_impulse_result_t result = { 0 };
		EI_IMPULSE_ERROR res = run_classifier(&ei_signal, &result, false);

		// Print results
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

