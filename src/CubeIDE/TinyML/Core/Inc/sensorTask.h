/**
 * @file sensorTask.h
 * @brief Header file for sensor data acquisition and classification tasks.
 *
 * This file contains function prototypes for the sensorTask and classifierTask,
 * responsible for collecting IMU data and running Edge Impulse classification.
 */

#ifndef INC_SENSORTASK_H_
#define INC_SENSORTASK_H_
#pragma once

 /* Function Prototypes */
void sensorTask(void *arg);
void classifierTask(void *arg);

#endif /* INC_SENSORTASK_H_ */
