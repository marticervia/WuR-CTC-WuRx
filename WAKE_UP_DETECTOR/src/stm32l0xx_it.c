/**
  ******************************************************************************
  * @file    stm32l0xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include "stm32l0xx_it.h"
#include "config_defines.h"

#define USE_CMP

extern I2C_HandleTypeDef I2cHandle;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables declared in main.c"---------------------------------------------------------*/
extern COMP_HandleTypeDef     hcomp1;
extern TIM_HandleTypeDef    timeout_timer;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/
/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void ADC1_COMP_IRQHandler(void)
{
  HAL_COMP_IRQHandler(&hcomp1);
}

void EXTI4_15_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(INPUT_FAST);
}

void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&timeout_timer);
}
void I2Cx_IRQHandler(void)
{
HAL_I2C_EV_IRQHandler(&I2cHandle);
HAL_I2C_ER_IRQHandler(&I2cHandle);
}


