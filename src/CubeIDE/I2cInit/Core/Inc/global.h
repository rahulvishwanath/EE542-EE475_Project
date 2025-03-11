/*
 * global.h
 *
 *  Created on: Mar 6, 2025
 *      Author: EE475
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

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern QueueHandle_t hapticQueue;
extern SemaphoreHandle_t classifierSemaphore;

extern void ERROR_LED(int);

#endif /* INC_GLOBAL_H_ */

