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
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32l0xx_it.h"

#define USE_CMP

#ifndef USE_CMP
#define INTERRUPT_PIN GPIO_PIN_8
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables declared in main.c"---------------------------------------------------------*/
#ifdef USE_CMP
extern COMP_HandleTypeDef     hcomp1;
#endif
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
#ifdef USE_RTOS_SYSTICK
	osSystickHandler();
#endif
}

#ifdef USE_CMP

void ADC1_COMP_IRQHandler(void)
{
  HAL_COMP_IRQHandler(&hcomp1);
}
#else
void EXTI4_15_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(INTERRUPT_PIN);
}
#endif


