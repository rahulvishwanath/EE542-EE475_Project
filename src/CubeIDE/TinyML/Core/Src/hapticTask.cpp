/**
 * @file hapticTask.cpp
 * @brief FreeRTOS task for handling haptic feedback.
 *
 * This task listens to the haptic queue and triggers vibration effects
 * when an action other than "idle" is detected.
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

/**
 * @brief FreeRTOS task to handle haptic feedback.
 *
 * The task continuously listens to the `hapticQueue` and triggers a vibration effect
 * whenever it receives a signal. The effect duration is controlled with a delay,
 * and the motor is stopped afterward.
 *
 * @param arg Unused parameter (for FreeRTOS compatibility).
 */
void hapticTask(void *arg)
{
	(void) arg;	// Suppress unused parameter warning
	HAL_StatusTypeDef status;
	char hapticMessage;
	char msg[50];

	while (1)
	{
		// Wait indefinitely for a message in the queue
		xQueueReceive(hapticQueue, &hapticMessage, portMAX_DELAY);

		// Play the vibration effect
		status = DRV2605_PlayEffect(&hi2c1, 14);
		if (debug_flag)
		{
			if (status != HAL_OK)
			{
			  snprintf(msg, sizeof(msg), "Failed to play effect\r\n");
			  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
			}
		}

		// Delay to allow effect to play (e.g., 250ms)
		vTaskDelay(pdMS_TO_TICKS(250));

		 // Stop the motor after the delay
		DRV2605_Stop(&hi2c1);
		if (debug_flag)
		{
			if (status != HAL_OK)
			{
			  snprintf(msg, sizeof(msg), "Failed to stop effect\r\n");
			  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
			}
		}
	}
}
