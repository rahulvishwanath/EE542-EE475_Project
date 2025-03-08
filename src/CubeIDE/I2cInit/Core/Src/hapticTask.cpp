/*
 * hapticTask.c
 *
 *  Created on: Mar 6, 2025
 *      Author: EE475
 */
#include "global.h"
#include "haptic_drv.h"

void hapticTask(void *arg)
{
	(void) arg;
	HAL_StatusTypeDef status;
//	status = DRV2605_Init(&hi2c1);
//	{
//	    snprintf(msg, sizeof(msg), "Haptic init failed\r\n");
//	    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
//	}
	TickType_t xLastWakeTime = xTaskGetTickCount();
	char hapticMessage;

	while (1)
	{
		xQueueReceive(hapticQueue, &hapticMessage, portMAX_DELAY);
		// Play a effect
		status = DRV2605_PlayEffect(&hi2c1, 14);
//		if (status != HAL_OK)
//		{
//		  snprintf(msg, sizeof(msg), "Failed to play effect\r\n");
//		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
//		}

		// Sample every 250ms
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(250));

		// Stop motor
		DRV2605_Stop(&hi2c1);
//		if (status != HAL_OK)
//		{
//		  snprintf(msg, sizeof(msg), "Failed to stop effect\r\n");
//		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
//		}
	}
}
