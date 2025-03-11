/*
 * hapticTask.c
 *
 *  Created on: Mar 6, 2025
 *      Author: EE475
 */
#include "global.h"
#include "task.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "haptic_drv.h"

#ifdef __cplusplus
	}
#endif

void hapticTask(void *arg)
{
	(void) arg;
	HAL_StatusTypeDef status;
	char msg[50];
	char hapticMessage;

	while (1)
	{
		xQueueReceive(hapticQueue, &hapticMessage, portMAX_DELAY);
		// Play a effect
		status = DRV2605_PlayEffect(&hi2c1, 14);
		if (status != HAL_OK)
		{
		  snprintf(msg, sizeof(msg), "Failed to play effect\r\n");
		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
		}

		// Delay for the desired effect duration (e.g. 250ms)
		vTaskDelay(pdMS_TO_TICKS(250));

		// Stop motor
		DRV2605_Stop(&hi2c1);
		if (status != HAL_OK)
		{
		  snprintf(msg, sizeof(msg), "Failed to stop effect\r\n");
		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
		}
	}
}
