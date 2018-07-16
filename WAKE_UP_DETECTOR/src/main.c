/**
  ******************************************************************************
  * @file    PWR/PWR_STOP/Src/main.c
  * @author  MCD Application Team
  * @brief   This sample code shows how to use STM32L0xx PWR HAL API to enter
  * and exit the stop mode without RTC.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include "stm32l0xx_hal_conf.h"

volatile uint32_t intr_flag = 0;
//#define BUTTON_DEBUG
#define USE_CMP
//#define BUSY_WAIT

#ifndef USE_CMP
#define INTERRUPT_PIN GPIO_PIN_8
#endif

#ifdef USE_CMP
	static void COMP_Config(COMP_HandleTypeDef* hcomp1);
	#define COMP_NON_INVERTING GPIO_PIN_3
	#define COMP_INVERTING GPIO_PIN_4
	#define COMP_OUTPUT GPIO_PIN_12
#endif

#define OUTPUT_PIN GPIO_PIN_9

/** @addtogroup STM32L0xx_HAL_Examples
  * @{
  */

/** @addtogroup PWR_STOP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void SystemPower_Config(void);
static void SystemClockConfig_STOP(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/
#ifdef USE_CMP
	COMP_HandleTypeDef hcomp1;
#endif

/**
* @brief  Main program
* @param  None
* @retval None
*/
int main(void)
{
  /* STM32L0xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source), keeping in mind that Time base
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();


  /* Configure the system clock @ 32 Mhz */
  SystemClock_Config();

  /* Configure the system Power */
  SystemPower_Config();

#ifdef USE_CMP
  COMP_Config(&hcomp1);
#endif

  /* activate either button pin for wakeup or GPIO PORT_C PIN 8 */
#ifdef BUTTON_DEBUG
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
#endif

  while (1)
  {
    /* Insert 5 second delay */
    HAL_Delay(5000);

    /* Key button (EXTI_Line13) will be used to wakeup the system from STOP mode */

    /* Enter Stop Mode and disable tick for the duration */
    HAL_SuspendTick();
    /* shut down indicator */
#ifdef BUTTON_DEBUG
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
#else
    HAL_GPIO_WritePin(GPIOC, OUTPUT_PIN, GPIO_PIN_RESET);
#endif

#ifndef BUSY_WAIT
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
#else
#ifdef USE_CMP
    // clear COMP2 intr line to not get bouncing
    WRITE_REG(EXTI->PR, COMP_EXTI_LINE_COMP2);
    intr_flag = 0;
#endif
    while(intr_flag == 0){}
    intr_flag = 0;
#endif
    /* restart indicator */
#ifdef BUTTON_DEBUG
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
#else
    HAL_GPIO_WritePin(GPIOC, OUTPUT_PIN, GPIO_PIN_SET);
#endif

    HAL_ResumeTick();
    /* Configures system clock after wake-up from STOP*/
    SystemClockConfig_STOP();
  }
}

/* Using HSI clock at 16000000 hz, not using PLL */

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSI Oscillator and activate PLL with HSI as source */
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
    Error_Handler();
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
    Error_Handler();
  }
}

#ifdef USE_CMP
static void COMP_Config(COMP_HandleTypeDef* hcomp1)
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
    Error_Handler();
  }

  /*##-3- Start teh COMP1 and enable the interrupt ###########################*/
  if(HAL_COMP_Start(hcomp1) != HAL_OK)
  {
    /* Initiliazation Error */
    Error_Handler();
  }
}
#endif

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
static void SystemPower_Config(void)
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
static void SystemClockConfig_STOP(void)
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
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while(1)
  {
    /* Turn on the LED2 */
  }
}

/**
  * @brief SYSTICK callback
  * @param None
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
  HAL_IncTick();
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

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
