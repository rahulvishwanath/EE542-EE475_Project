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
#define TEST_DURATION_SECONDS 3          // Duration of the test in seconds
#define TOTAL_SAMPLES (static_cast<int>(SAMPLING_RATE_HZ * TEST_DURATION_SECONDS))
using namespace ei;

signal_t ei_signal;
// Acceleration and Gyro Data
float G[3];
float A[3];

float imu_data[6*TOTAL_SAMPLES];
float imu_data_next[6*TOTAL_SAMPLES];

float *pdata_collector = nullptr;
float *pdata_collector_next = nullptr;
bool collector_flag = true;

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

void collectData(){
	int sampleCount = 0;

	while (sampleCount < TOTAL_SAMPLES) {
		MPU6500_Read_Accel(A);
		MPU6500_Read_Gyro(G);
		pdata_collector[sampleCount * 6] = A[0];
		pdata_collector[sampleCount * 6 + 1] = A[1];
		pdata_collector[sampleCount * 6 + 2] = A[2];
		pdata_collector[sampleCount * 6 + 3] = G[0];
		pdata_collector[sampleCount * 6 + 4] = G[1];
		pdata_collector[sampleCount * 6 + 5] = G[2];
		sampleCount++;
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SAMPLING_PERIOD_ms));
	}
}

void sensorTask(void *arg)
{
	(void) arg;
	xLastWakeTime = xTaskGetTickCount();
	TickType_t startTick,diffTick;

	while (1)
	{
		startTick = xTaskGetTickCount();
		if (collector_flag){
			pdata_collector = imu_data;
			collector_flag = false;
		}else{
			pdata_collector = imu_data_next;
			collector_flag = true;
		}

		collectData();
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		diffTick =  xTaskGetTickCount()- startTick;
		ei_printf("Data Collection Timing: ");
		ei_printf_float(diffTick);
		xSemaphoreTake(sensorSemaphore, portMAX_DELAY);
		pdata_collector_next = pdata_collector;
		// Run inference// Run inference
		ei_signal.total_length = (unsigned int)6*TOTAL_SAMPLES;
		ei_signal.get_data = [&pdata_collector_next](size_t offset, size_t length, float *out_ptr) {
			memcpy(out_ptr, pdata_collector_next + offset, length * sizeof(float));
			return 0;
		};

		xSemaphoreGive(classifierSemaphore);
	}
}

void classifierTask(void *arg)
{
	(void) arg;
	float label_thres;
	int label_ix;
	char hapticMessage;
	TickType_t startTick,diffTick;

	while (1)
	{

		xSemaphoreGive(sensorSemaphore);
		xSemaphoreTake(classifierSemaphore, portMAX_DELAY);

		startTick = xTaskGetTickCount();
		ei_impulse_result_t result = { 0 };
		EI_IMPULSE_ERROR res = run_classifier(&ei_signal, &result, false);
		diffTick =  xTaskGetTickCount()- startTick;
		ei_printf("Classifier Timing: ");
		ei_printf_float(diffTick);      // Print value inline
		ei_printf("Predictions:\r\n");
		for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
			ei_printf("  %s: ", result.classification[ix].label);  // Print label first
			ei_printf_float(result.classification[ix].value);      // Print value inline
			ei_printf("\r\n");                                     // Newline at the end
		}                             // Newline at the end
		}

//		label_thres = 0.0f;
//		label_ix = 0;
//		for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
//			if (result.classification[ix].value > label_thres){
//				label_ix = ix;
//			}
//		}
//
//		if (label_ix != 2){
//			 // Call Haptic task
//			hapticMessage = '1';
//			xQueueSend(hapticQueue,&hapticMessage,0);
//		}
//		ei_printf(" %s : %f ",result.classification[label_ix].label , result.classification[label_ix].value);
//		ei_printf("\r\n");
}

