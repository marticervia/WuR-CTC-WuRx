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
#include <periph_config.h>
#include "main.h"
#include <string.h>
#include "stm32l0xx_hal_conf.h"
#include "user_handlers.h"
#include "config_defines.h"
#include "power_config.h"


COMP_HandleTypeDef hcomp1;
TIM_HandleTypeDef  timeout_timer;

/** @addtogroup STM32L0xx_HAL_Examples
  * @{
  */

/** @addtogroup PWR_STOP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
typedef enum wurx_states{
	WURX_SLEEP = 0,
	WURX_WAITING_PREAMBLE = 1,
	WURX_DECODING_PREAMBLE = 2,
	WURX_DECODING_PAYLOAD = 3,
	WURX_GOING_TO_SLEEP = 4,
}wurx_states_t;

typedef struct wurx_context{
	wurx_states_t wurx_state;
	uint16_t wurx_address;
}wurx_context_t;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile uint32_t timer_timeout = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


static void sleepMCU(void){
    /* shut down indicator */
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	pinModeSleep();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    /* restart indicator */
	pinModeAwake();
    PIN_SET(GPIOA, WAKE_UP_FAST);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Set timeout flag */
	timer_timeout++;
}


static void initWuRxContext(wurx_context_t* context){
	context->wurx_address = DEFAULT_ADDRESS;
	context->wurx_state = WURX_SLEEP;

	/* use Timer 2 with a 100 us timeout*/
	HAL_TIM_Base_DeInit(&timeout_timer);
	timeout_timer.Instance = TIM2;
}

/* For now let's use the SLOW solution of struct + switch
 *  if 16 MHz is not enough start using uglier but faster solutions
 */


/* sets all pins to analog input, but SWD */

/**
* @brief  Main program
* @param  None
* @retval None
*/
int main(void)
{
	wurx_context_t wur_ctxt = {0};
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

	/* Configure the system Power */
	SystemPower_Config();
    HAL_SuspendTick();
	pinModeinit();
	COMP_Config(&hcomp1);
	TIMER_Config(TIM2);
	initWuRxContext(&wur_ctxt);
	PIN_RESET(GPIOA, ADDR_OK);
	while (1)
	{
		switch(wur_ctxt.wurx_state){
			case WURX_SLEEP:
				/* this one blocks until MCU wakes*/
				sleepMCU();
#ifdef ULP
				__TIM2_CLK_ENABLE();
#endif
				/* wait 100 us for preamble init.*/
				TIMER_ENABLE(TIM2);
				PIN_SET(GPIOA, ADDR_OK);
				wur_ctxt.wurx_state = WURX_WAITING_PREAMBLE;
				break;
			case WURX_WAITING_PREAMBLE:
				if(timer_timeout == 2){
					timer_timeout = 0;
					PIN_RESET(GPIOA, ADDR_OK);
					TIMER_DISABLE(TIM2);
#ifdef ULP
					__TIM2_CLK_DISABLE();
#endif
					wur_ctxt.wurx_state = WURX_SLEEP;
				}
				/* add logic to read the preamble */
				break;
			case WURX_DECODING_PREAMBLE:
				break;
			case WURX_DECODING_PAYLOAD:
				break;
			default:
			initWuRxContext(&wur_ctxt);
			break;
		}
	}
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
