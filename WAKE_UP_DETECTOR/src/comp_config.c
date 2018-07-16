/*
 * comp_config.c
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */
#include "comp_config.h"
#include "config_defines.h"
#include "stm32l0xx_hal_conf.h"

#ifdef USE_CMP
void COMP_Config(COMP_HandleTypeDef* hcomp1)
{
  /*##-1- Configure the COMP peripheral ######################################*/
  hcomp1->Instance = COMP2;

  hcomp1->Init.NonInvertingInput  = COMP_INPUT_PLUS_IO2;
  hcomp1->Init.InvertingInput  = COMP_INPUT_MINUS_IO2;
  hcomp1->Init.OutputPol       = COMP_OUTPUTPOL_NONINVERTED;
  hcomp1->Init.Mode            = COMP_POWERMODE_MEDIUMSPEED;
  hcomp1->Init.TriggerMode     = COMP_TRIGGERMODE_IT_RISING;
  hcomp1->Init.LPTIMConnection = COMP_LPTIMCONNECTION_DISABLED;
  hcomp1->Init.WindowMode	   = COMP_WINDOWMODE_DISABLE;
  hcomp1->State		           = HAL_COMP_STATE_RESET;
  hcomp1->Lock				   = HAL_UNLOCKED;

  if(HAL_COMP_Init(hcomp1) != HAL_OK)
  {
    /* Initiliazation Error */
	  System_Error_Handler();
  }

  /*##-3- Start teh COMP1 and enable the interrupt ###########################*/
  if(HAL_COMP_Start(hcomp1) != HAL_OK)
  {
    /* Initiliazation Error */
	  System_Error_Handler();
  }
}
#endif

/**
  * @brief GPIO EXTI callback
  * @param None
  * @retval None
  */
#ifndef USE_CMP
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Clear Wake Up Flag */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}

#else
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp){
	  /* Clear Wake Up Flag */
#ifndef BUSY_WAIT
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
#else
	intr_flag = 1;
#endif
}
#endif
