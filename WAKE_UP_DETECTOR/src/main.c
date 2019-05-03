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
	WURX_DECODING_FRAME = 2,
	WURX_GOING_TO_SLEEP = 3,
	WURX_HAS_FRAME = 0,
}wurx_states_t;

typedef struct wurx_context{
	wurx_states_t wurx_state;
	uint16_t wurx_address;
	uint8_t frame_len;
	uint8_t frame_buffer[MAX_FRAME_LEN];
}wurx_context_t;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile uint32_t timer_timeout = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static uint16_t expected_addr[20] = {0, 0, INPUT_FAST, 0, 0,INPUT_FAST,0,INPUT_FAST,0,0,INPUT_FAST,INPUT_FAST,0,0,0,0,INPUT_FAST,INPUT_FAST,INPUT_FAST,INPUT_FAST};

static void sleepMCU(wurx_context_t* wur_context){
    /* shut down indicator */
	if(wur_context->wurx_state != WURX_HAS_FRAME){
		wur_context->wurx_state = WURX_SLEEP;
	}

	PIN_RESET(GPIOA, WAKE_UP_FAST);
	pinModeSleep();
    HAL_SuspendTick();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    HAL_NVIC_DisableIRQ(ADC1_COMP_IRQn);
    //HAL_ResumeTick();
    /* restart indicator */
	pinModeAwake();
    PIN_SET(GPIOA, WAKE_UP_FAST);
    /* Configures system clock after wake-up from STOP*/
    SystemPower_ConfigSTOP();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Set timeout flag */
	timer_timeout++;
}


static void initWuRContext(wurx_context_t* context){
	context->wurx_state = WURX_GOING_TO_SLEEP;
	context->wurx_address = APPLY_ADDR_MASK(DEFAULT_ADDRESS);
	context->frame_len = 0;
	memset(context->frame_buffer, 0, MAX_FRAME_LEN)	;
}

static void goToSleep(wurx_context_t* wur_context){

	if(wur_context->wurx_state != WURX_HAS_FRAME){
		wur_context->wurx_state = WURX_GOING_TO_SLEEP;
	}

	PIN_SET(GPIOA, ADDR_OK);
	PIN_RESET(GPIOA, ADDR_OK);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	TIMER_DISABLE(TIM2);
	CLEAR_TIMER_EXPIRED(TIM2);
	TIMER_DISABLE(TIM21);
	CLEAR_TIMER_EXPIRED(TIM21);
	__TIM2_CLK_DISABLE();
	__TIM21_CLK_DISABLE();
}

/* sets all pins to analog input, but SWD */

/**
* @brief  Main program
* @param  None
* @retval None
*/

static void loopWuR(wurx_context_t* context){

	while (1)
	{
		uint32_t result = 0;
		/* this one blocks until MCU wakes*/
		sleepMCU(context);
		__TIM2_CLK_ENABLE();
		CLEAR_TIMER_EXPIRED(TIM2);
		/* wait 100 us for preamble init.*/
		PIN_SET(GPIOA, WAKE_UP_FAST);

		TIMER_SET_PERIOD(TIM2, 1060);
		TIMER_COMMIT_UPDATE(TIM2);
		CLEAR_TIMER_EXPIRED(TIM2);
		TIMER_ENABLE(TIM2);

		/* finish waiting for preamble start */
		while(!IS_TIMER_EXPIRED(TIM2));

		TIMER_DISABLE(TIM2);

		//12 instructions loop
		for(uint8_t loop = 0; loop < MAX_LOOPS; loop++){
			result = READ_PIN(GPIOA, INPUT_FAST);
			if(result != expected_addr[loop]){
				goToSleep(context);
				break;
			}
			//52 cycles left still to do crazy shit
			ADJUST_WITH_NOPS;
		}

		PIN_SET(GPIOA, ADDR_OK);
		ADJUST_WITH_NOPS;
		PIN_RESET(GPIOA, ADDR_OK);

		goToSleep(context);
	}
}

int main(void)
{

	wurx_context_t context;
	initWuRContext(&context);

	HAL_Init();

	/* Configure the system Power */
	SystemPower_Config();

	pinModeinit();
	TIMER_Config();
	COMP_Config(&hcomp1);

	loopWuR(&context);

}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
