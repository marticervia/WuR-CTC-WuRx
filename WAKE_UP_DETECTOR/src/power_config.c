/*
 * power_config.c
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */
#include "power_config.h"
#include "config_defines.h"
#include "stm32l0xx_hal_conf.h"
#include "string.h"
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
void SystemPower_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure = {0};

#ifndef USE_CMP
  /* Enable Ultra low power mode */
  HAL_PWREx_EnableUltraLowPower();

  /* Enable the fast wake up from Ultra low power mode */
  HAL_PWREx_EnableFastWakeUp();

#endif
  /* Select HSI as system clock source after Wake Up from Stop mode */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

  /* Enable GPIOs clock, PORTC is enabled when activating the button interrupt. */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

#ifdef BUTTON_DEBUG
  /* start LED2 pin to show MCU state */
  GPIO_InitStructure.Pin = (GPIO_PIN_5);
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH  ;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
  __HAL_RCC_GPIOC_CLK_ENABLE();
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));

  GPIO_InitStructure.Pin = OUTPUT_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* copnfigure PA2 as COMP2 output */
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));

  GPIO_InitStructure.Pin = COMP_OUTPUT;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Alternate = GPIO_AF7_COMP2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

#ifndef USE_CMP
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
  GPIO_InitStructure.Pin = INTERRUPT_PIN;
  GPIO_InitStructure.Mode   = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  /* Enable and set Button EXTI Interrupt to the lowest priority */
  HAL_NVIC_SetPriority((IRQn_Type)(EXTI4_15_IRQn), 0x0F, 0);
  HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI4_15_IRQn));

#endif
  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
  /* Configure rest of GPIO port pins in Analog Input mode (floating input trigger OFF) */
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_All;

#ifdef BUTTON_DEBUG
  /* DO NOT overwrite SWD pins when debugging, also, LED2. */
  GPIO_InitStructure.Pin = GPIO_PIN_All & ~((uint32_t)(1<<13) | (uint32_t)(1<<14) | (uint32_t)(1<<5));
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.Pin = GPIO_PIN_All;
#else
  GPIO_InitStructure.Pin = GPIO_PIN_All & ~((uint32_t)(1<<13) | (uint32_t)(1<<14) | (uint32_t)(1<<12));
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.Pin = GPIO_PIN_All;
#endif
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
  /* All port B in analog input is also OK for comparator mode */
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
  /* Disable unneeded GPIOs clock */
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
#ifndef BUTTON_DEBUG
  //__HAL_RCC_GPIOA_CLK_DISABLE();
#endif

}

/**
  * @brief  Configures system clock after wake-up from STOP: enable HSI, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SystemClockConfig_STOP(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Get the Oscillators configuration according to the internal RCC registers */
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

  /* After wake-up from STOP reconfigure the system clock: Enable HSI and PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV2;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
	  System_Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
	  System_Error_Handler();
  }
}

