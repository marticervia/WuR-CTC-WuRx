/*
MIT License

Copyright (c) 2020 marticervia

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <periph_config.h>
#include "stm32l0xx_hal_conf.h"
#include "user_handlers.h"
#include "config_defines.h"
#include "power_config.h"
#include "i2c_com.h"
#include "wurx.h"

COMP_HandleTypeDef hcomp1, hcomp2;
TIM_HandleTypeDef  timeout_timer;

I2C_HandleTypeDef I2cHandle;

static volatile uint8_t I2C_operation = 0, WuR_operation = 0;

typedef enum wurx_status{
	WUR_SLEEPING,
	WUR_WAIT_DATA,
	WUR_WAIT_I2C
}wurx_status_t;

typedef struct wurx_ctxt{
	wurx_status_t wurx_status;
	uint32_t	  wurx_timestamp;
}wurx_ctxt_t;

/**
* @brief  Main program
* @param  None
* @retval None
*/

static wurx_ctxt_t app_wurx_ctxt = {
		.wurx_status = WUR_SLEEPING,
		.wurx_timestamp = 0
};

static void loopMain(wurx_context_t* context){
	int32_t wake_status;
    HAL_SuspendTick();

	while (1)
	{
		switch(app_wurx_ctxt.wurx_status){
			case WUR_SLEEPING:
				I2C_operation = 0;
				WuR_operation = 0;
				WuR_go_sleep(context);
				/* configure on wakeup for HSI use*/
				SystemPower_sleep();
				if(WuR_operation && !I2C_operation){
					/* WAKE MS for a WAKE frame, will be != 0, it will be 0xFFFF for a SLEEP frame*/
					wake_status = WuR_process_frame(context, 1);
					WuR_operation = 0;
					if(wake_status < 0){
						break;
					}
					if(wake_status > 0){
						app_wurx_ctxt.wurx_timestamp = wake_status;
						app_wurx_ctxt.wurx_status = WUR_WAIT_DATA;
					}
				}else if(I2C_operation){
					app_wurx_ctxt.wurx_timestamp = 10;
					PIN_RESET(GPIOA, WAKE_UP_FAST);
					app_wurx_ctxt.wurx_status = WUR_WAIT_I2C;
				}
				break;
			case WUR_WAIT_DATA:
				/* activate HSE for use. During this interval the MCU will be irresponsive.*/
			    PIN_SET(GPIOA, WAKE_UP_FAST);
				SystemPower_data();
			    HAL_SuspendTick();
				/* notify host that we have a frame ready via interrupt and change state accordingly*/
				PIN_SET(GPIOA, ADDR_OK);
				ADJUST_WITH_NOPS;
				ADJUST_WITH_NOPS;
				PIN_RESET(GPIOA, ADDR_OK);
				/* prepare TIM6 to end at the timeout */
				__TIM6_CLK_ENABLE();
				__TIM21_CLK_ENABLE();
				TIMER_SET_PERIOD(TIM6, app_wurx_ctxt.wurx_timestamp);
				TIMER_UIT_ENABLE(TIM6);
				TIMER_COMMIT_UPDATE(TIM6);
				TIMER_ENABLE(TIM6);
				WuR_operation = 0;
			    I2C_operation = 0;
				pinModeWaitFrame();
				/* activate again the reception interrupt*/
				/* loop used inside the state to minimize jitter*/
			    PIN_RESET(GPIOA, WAKE_UP_FAST);
				while(!IS_TIMER_EXPIRED(TIM6))
				{
					if((context->wurx_state != WURX_HAS_FRAME) && WuR_operation && !I2C_operation){
						pinModeFrameReceived();
					    PIN_SET(GPIOA, WAKE_UP_FAST);
						WuR_operation = 0;
						PIN_RESET(GPIOA, WAKE_UP_FAST);
						/* WAKE MS for a WAKE frame, will be != 0, it will be 0xFFFF for a SLEEP frame*/
						wake_status = WuR_process_frame(context, 0);
					    PIN_SET(GPIOA, WAKE_UP_FAST);
					    if(wake_status < 0){
							WuR_operation = 0;
							pinModeWaitFrame();
					    	continue;
					    }
					    else if(wake_status == 0x0001){
							/* if that, set the TIM6 to timeout in 10 ms*/
							TIMER_SET_PERIOD(TIM6, 10);
							TIMER_UIT_ENABLE(TIM6);
							TIMER_COMMIT_UPDATE(TIM6);
						}
						PIN_SET(GPIOA, ADDR_OK);
						ADJUST_WITH_NOPS;
						ADJUST_WITH_NOPS;
						PIN_RESET(GPIOA, ADDR_OK);
						pinModeWaitFrame();
					}else if(I2C_operation){
						pinModeFrameReceived();
			    		i2c_notify_req_operation();
					    I2C_operation = 0;
						WuR_operation = 0;
						/* protect I2C transactions from frame interruptions*/
					    PIN_RESET(GPIOA, WAKE_UP_FAST);
					    PIN_SET(GPIOA, WAKE_UP_FAST);
						/* process first frame */
				    	i2c_state_machine();
				    	while(i2Cbusy()){
					    	i2c_state_machine();
					    }
					    PIN_RESET(GPIOA, WAKE_UP_FAST);
						I2C_operation = 0;
						pinModeWaitFrame();
					}
				}
				CLEAR_TIMER_EXPIRED(TIM6);
				TIMER_DISABLE(TIM6);
				__TIM6_CLK_DISABLE();
				__TIM21_CLK_DISABLE();

				/* deactivate HSE and return to the default clock config with HSI*/
				SystemPower_wake();
				app_wurx_ctxt.wurx_status = WUR_SLEEPING;
				break;
			case WUR_WAIT_I2C:
				pinModeFrameReceived();
				/* prepare TIM6 to end at the timeout */
				PIN_SET(GPIOA, WAKE_UP_FAST);
				__TIM21_CLK_ENABLE();
				i2c_notify_req_operation();
				i2c_state_machine();
				while(i2Cbusy()){
					i2c_state_machine();
				}
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				I2C_operation = 0;
				reset_i2c_state(&I2cHandle);
			    app_wurx_ctxt.wurx_status = WUR_SLEEPING;
				pinModeWaitFrame();
				__TIM21_CLK_DISABLE();
				break;
			default:
				app_wurx_ctxt.wurx_status = WUR_SLEEPING;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  if(GPIO_Pin == WAKE_UP_I2C)
  {
	  /* flag the start of an I2C operation */
	  I2C_operation = 1;
  }
#ifndef USE_CMP
  else if(GPIO_Pin == INPUT_FAST){
	  WuR_operation = 1;
  }
#endif
}

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp){
	  /* Clear Wake Up Flag */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	  WuR_operation = 1;
}

int main(void)
 {

	wurx_context_t context;
	WuR_init_context(&context);

	HAL_Init();

	/* Configure the system Power for HSI use */
	Initial_SystemPower_Config();
	i2CConfig(&context, &I2cHandle);
	reset_i2c_state(&I2cHandle);
	pinModeinit();
	TIMER_Config();
#ifdef USE_CMP
	COMP_Config(&hcomp1, 1);
	COMP_Config(&hcomp2, 2);
#endif
	loopMain(&context);

}
