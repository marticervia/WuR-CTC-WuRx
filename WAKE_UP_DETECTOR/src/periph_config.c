/*
 * comp_config.c
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */
#include "periph_config.h"
#include "config_defines.h"
#include "stm32l0xx_hal_conf.h"
#include "string.h"
#include "user_handlers.h"

void HAL_COMP_MspInit(COMP_HandleTypeDef* hcomp)
{

  GPIO_InitTypeDef GPIO_InitStructure = {0};
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* copnfigure PA2 as COMP2 output for debugging purposes*/
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));

  GPIO_InitStructure.Pin = COMP_OUTPUT;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Alternate = GPIO_AF7_COMP1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);


  /* PA0 and PA1 are OK on ANALOG input mode */
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));

  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*##-3- Configure the NVIC for COMP1 #######################################*/
   /* Enable the COMP1 IRQ Channel */
  HAL_NVIC_SetPriority(ADC1_COMP_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);
}

/**
  * @brief  DeInitializes the COMP MSP.
  * @param  hcomp: pointer to a COMP_HandleTypeDef structure that contains
  *         the configuration information for the specified COMP.
  * @retval None
  */
void HAL_COMP_MspDeInit(COMP_HandleTypeDef* hcomp)
{
  /*##-1- Reset peripherals ##################################################*/
  /* Disable COMP1 clock */
  __HAL_RCC_SYSCFG_CLK_DISABLE();
  /* Disable GPIO clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();

  /*##-2- Disable peripherals and GPIO  ######################################*/
  /* De-Initialize COMP1 */
  HAL_COMP_DeInit(hcomp);
  /* De-initialize the GPIO pin */
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

  /*##-3- Disable the NVIC for COMP ##########################################*/
  HAL_NVIC_DisableIRQ(ADC1_COMP_IRQn);
}

void COMP_Config(COMP_HandleTypeDef* hcomp1)
{
  /*##-1- Configure the COMP peripheral ######################################*/
  hcomp1->Instance = COMP1;

  hcomp1->Init.NonInvertingInput  = COMP_INPUT_PLUS_IO1;
  hcomp1->Init.InvertingInput  = COMP_INPUT_MINUS_IO1;
  hcomp1->Init.OutputPol       = COMP_OUTPUTPOL_NONINVERTED;
  hcomp1->Init.Mode            = COMP_POWERMODE_ULTRALOWPOWER;
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

/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  __TIM2_CLK_ENABLE();
  /*##-2- Configure the NVIC for TIMx ########################################*/
  /* Set the TIMx priority */
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);

  /* Enable the TIMx global Interrupt */
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void TIMER_Config(TIM_TypeDef* TIMx){
    HAL_TIM_Base_MspInit(NULL);
	TIMER_SET_COUNTING_MODE(TIMx, TIM_COUNTERMODE_UP);
	TIMER_SET_CLOCK_DIVISOR(TIMx, TIM_CLOCKDIVISION_DIV1);
	TIMER_SET_PERIOD(TIMx, 799);
	TIMER_SET_PRESCALER(TIMx, 0);
	TIMER_COMMIT_UPDATE(TIMx);
}



void pinModeinit(void){
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	  /* Enable GPIOs clock, PORTC is enabled when activating the button interrupt. */
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  __HAL_RCC_GPIOH_CLK_ENABLE();

	  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	  /* Configure rest of GPIO port pins in Analog Input mode (floating input trigger OFF) */
	  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	  GPIO_InitStructure.Pull = GPIO_NOPULL;
	  GPIO_InitStructure.Pin = GPIO_PIN_All;
	  GPIO_InitStructure.Pin = GPIO_PIN_All & ~((uint32_t)(1<<13) | (uint32_t)(1<<14) | (uint32_t)(1<<12));
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* configure OUTPUTs */
	  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	  GPIO_InitStructure.Pin = WAKE_UP_FAST;
	  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	  GPIO_InitStructure.Pin = ADDR_OK;
	  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	  /* configure fast input*/
	  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	  GPIO_InitStructure.Pin = INPUT_FAST;
	  GPIO_InitStructure.Mode   = GPIO_MODE_IT_RISING;
	  GPIO_InitStructure.Pull = GPIO_PULLUP;
	  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	  /* Enable and set Button EXTI Interrupt to the lowest priority */
	  HAL_NVIC_SetPriority((IRQn_Type)(EXTI4_15_IRQn), 0x0F, 0);


	  /* set all the rest of pins to ANALOG NOPULL to save power.*/
	  GPIO_InitStructure.Pin = GPIO_PIN_All;
	  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	  GPIO_InitStructure.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

	  /* Disable unneeded GPIOs clock */
	  __HAL_RCC_GPIOA_CLK_DISABLE();
	  __HAL_RCC_GPIOB_CLK_DISABLE();
	  __HAL_RCC_GPIOC_CLK_DISABLE();
	  __HAL_RCC_GPIOD_CLK_DISABLE();
	  __HAL_RCC_GPIOH_CLK_DISABLE();
}

void pinModeSleep(void){
#ifdef ULP
	HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI4_15_IRQn));
	__HAL_RCC_GPIOA_CLK_DISABLE();
#endif
}

void pinModeAwake(void){
#ifdef ULP
    __HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI4_15_IRQn));
#endif
}
