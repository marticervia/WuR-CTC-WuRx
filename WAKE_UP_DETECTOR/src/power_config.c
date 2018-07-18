/*
 * power_config.c
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */
#include "power_config.h"
#include "config_defines.h"
#include "user_handlers.h"
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
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	 clocked below the maximum system frequency, to update the voltage scaling value
	 regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/* Enable HSI Oscillator and deactivate PLL with HSI as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
	RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV2;
	/* Used in all examples, maybe its the most common trim.*/
	RCC_OscInitStruct.HSICalibrationValue = 0x10;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
	  System_Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
	  System_Error_Handler();
	}

#ifdef ULP
  /* Enable Ultra low power mode */
  HAL_PWREx_EnableUltraLowPower();

  /* Enable the fast wake up from Ultra low power mode */
  HAL_PWREx_EnableFastWakeUp();

#endif
  /* Select HSI as system clock source after Wake Up from Stop mode */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
}


/**
  * @brief  Configures system clock after wake-up from STOP: enable HSI, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SystemPower_ConfigSTOP(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

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


#ifdef USE_ULP
  HAL_PWREx_EnableUltraLowPower();
  HAL_PWREx_EnableFastWakeUp();
#endif

  /* Select HSI as system clock source after Wake Up from Stop mode */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

}

