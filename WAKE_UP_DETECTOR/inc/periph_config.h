/*
 * comp_config.h
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */

#ifndef PERIPH_CONFIG_H_
#define PERIPH_CONFIG_H_
#include "main.h"

void COMP_Config(COMP_HandleTypeDef* hcomp1);
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp);
void pinModeSleep(void);
void pinModeAwake(void);
void pinModeinit(void);

#endif /* PERIPH_CONFIG_H_ */
