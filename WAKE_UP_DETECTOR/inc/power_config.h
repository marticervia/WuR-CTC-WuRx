/*
 * power_config.h
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */

#ifndef POWER_CONFIG_H_
#define POWER_CONFIG_H_
#include "main.h"

/**
  * @brief  System Power Configuration
  *         The system Power is configured as follow :
  *            + Regulator in LP mode
  *            + VREFINT OFF, with fast wakeup enabled
  *            + HSI as SysClk after Wake Up
  *            + No IWDG
  *            + Wakeup using EXTI Line (Key Button PC.13)
  * @param  None
  * @retval None
  */
void Initial_SystemPower_Config(void);

/**
  * @brief  Configures system clock after wake-up from STOP: enable HSI, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SystemPower_ConfigSTOP(void);
void SystemPower_sleep(void);
void SystemPower_prepare_sleep(void);
void SystemPower_data(void);
void SystemPower_wake(void);

#endif /* POWER_CONFIG_H_ */
