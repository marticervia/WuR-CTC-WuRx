/*
 * comp_config.h
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */

#ifndef COMP_CONFIG_H_
#define COMP_CONFIG_H_
#include "main.h"

void COMP_Config(COMP_HandleTypeDef* hcomp1);
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp);


#endif /* COMP_CONFIG_H_ */
