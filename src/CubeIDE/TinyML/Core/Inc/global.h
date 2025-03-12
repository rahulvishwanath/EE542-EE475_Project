/**
 * @file global.h
 * @brief Global definitions and shared resources for the STM32 TinyML system.
 *
 * This file declares global variables, including hardware peripherals,
 * FreeRTOS queues, and semaphores used across the system.
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#pragma once
#include <main.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "semphr.h"

/* External Hardware Interfaces */
extern I2C_HandleTypeDef hi2c1;		///< I2C handle for MPU6500 and DRV2605 communication
extern UART_HandleTypeDef huart2;	///< UART handle for debugging output

/* FreeRTOS Synchronization Primitives */
extern QueueHandle_t hapticQueue;				///< Queue to trigger haptic feedback
extern SemaphoreHandle_t classifierSemaphore;	///< Semaphore for classification task synchronization
extern bool debug_flag;							///< DebugFlag to handle print statement on UART

/* Error LED for Debugging */
extern void ERROR_LED(int);

#endif /* INC_GLOBAL_H_ */

