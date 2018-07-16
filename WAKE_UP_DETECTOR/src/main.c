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
#include "clock_config.h"
#include "user_handlers.h"
#include "config_defines.h"

volatile uint32_t intr_flag = 0;

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


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
