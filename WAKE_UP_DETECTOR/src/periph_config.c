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
#include "i2c_com.h"

void HAL_COMP_MspInit(COMP_HandleTypeDef* hcomp)
{

  GPIO_InitTypeDef GPIO_InitStructure = {0};
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* copnfigure PA2 as COMP2 output for debugging purposes*/
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));

  GPIO_InitStructure.Pin = COMP_OUTPUT;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStructure.Alternate = GPIO_AF7_COMP1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);


  /* PA0 and PA1 are OK on ANALOG input mode */
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));

  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = COMP_INVERTING | COMP_NON_INVERTING;
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

void TIMER_Config(){
	__TIM2_CLK_ENABLE();
	TIMER_SET_COUNTING_MODE(TIM2, TIM_COUNTERMODE_UP);
	TIMER_SET_CLOCK_DIVISOR(TIM2, TIM_CLOCKDIVISION_DIV1);
	TIMER_SET_PERIOD(TIM2, 1599);
	TIMER_SET_PRESCALER(TIM2, 0);
	TIMER_COMMIT_UPDATE(TIM2);
	__TIM2_CLK_DISABLE();

	__TIM21_CLK_ENABLE();
	TIMER_SET_COUNTING_MODE(TIM21, TIM_COUNTERMODE_UP);
	TIMER_SET_CLOCK_DIVISOR(TIM21, TIM_CLOCKDIVISION_DIV1);
	TIMER_SET_PERIOD(TIM21, 64);
	TIMER_SET_PRESCALER(TIM21, 0);
	TIMER_COMMIT_UPDATE(TIM21);
	__TIM21_CLK_DISABLE();
}

void TIMER21_Config(TIM_TypeDef* TIMx){

}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;

  /*##-1- Configure the I2C clock source. The clock is derived from the SYSCLK #*/
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

  /*##-2- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  I2Cx_CLK_ENABLE();

  /*##-3- Configure peripheral GPIO ##########################################*/
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN | I2Cx_SDA_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

  /*##-4- Configure the NVIC for I2C ########################################*/
  /* NVIC for I2Cx */
  HAL_NVIC_SetPriority(I2Cx_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(I2Cx_IRQn);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{

  /*##-1- Reset peripherals ##################################################*/
  I2Cx_FORCE_RESET();
  I2Cx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure I2C Tx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
  /* Configure I2C Rx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);

  /*##-3- Disable the NVIC for I2C ##########################################*/
  HAL_NVIC_DisableIRQ(I2Cx_IRQn);
}

void pinModeinit(void){
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	  /* Enable GPIOs clock, PORTC is enabled when activating the button interrupt. */
	  __HAL_RCC_GPIOA_CLK_ENABLE();
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
	  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	  GPIO_InitStructure.Pin = ADDR_OK;
	  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	  /* configure fast input*/
	  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	  GPIO_InitStructure.Pin = INPUT_FAST;
	  GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
	  GPIO_InitStructure.Pull = GPIO_NOPULL;
	  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	  /* Enable and set Button EXTI Interrupt to the lowest priority */
	  //HAL_NVIC_SetPriority((IRQn_Type)(EXTI4_15_IRQn), 0x0F, 0);

	  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
	  /* set all the rest of pins to ANALOG NOPULL to save power.*/
	  GPIO_InitStructure.Pin = GPIO_PIN_All;
	  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	  GPIO_InitStructure.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

	  /* Disable unneeded GPIOs clock */
	  __HAL_RCC_GPIOA_CLK_DISABLE();
	  __HAL_RCC_GPIOC_CLK_DISABLE();
	  __HAL_RCC_GPIOD_CLK_DISABLE();
	  __HAL_RCC_GPIOH_CLK_DISABLE();
}

void pinModeSleep(void){
	HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI4_15_IRQn));
    HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);

	__HAL_RCC_GPIOA_CLK_DISABLE();

}

void pinModeAwake(void){
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_NVIC_DisableIRQ(ADC1_COMP_IRQn);
	HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI4_15_IRQn));
}
