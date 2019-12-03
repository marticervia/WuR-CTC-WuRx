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
	uint16_t wake_ms;
	uint32_t current_tick;

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
					wake_ms = WuR_process_frame(context, 1);
					WuR_operation = 0;
					if(!wake_ms){
						break;
					}
					app_wurx_ctxt.wurx_timestamp = HAL_GetTick() + wake_ms;
					app_wurx_ctxt.wurx_status = WUR_WAIT_DATA;
				}else if(I2C_operation && !WuR_operation){
					I2C_operation = 0;
					app_wurx_ctxt.wurx_timestamp = HAL_GetTick() + 10;
					app_wurx_ctxt.wurx_status = WUR_WAIT_I2C;
				}else{
					WuR_operation = 0;
					I2C_operation = 0;
				}
				break;
			case WUR_WAIT_DATA:
				/* activate HSE for use*/
			    HAL_ResumeTick();
				SystemPower_data();
				pinModeWaitFrame();
				current_tick = HAL_GetTick();
				/* activate again the reception interrupt*/
				/* loop used inside the state to minimize jitter*/
				while(current_tick < app_wurx_ctxt.wurx_timestamp){
					if(WuR_operation){
					    HAL_SuspendTick();
					    PIN_SET(GPIOA, WAKE_UP_FAST);
						pinModeFrameReceived();
						PIN_RESET(GPIOA, WAKE_UP_FAST);
						/* WAKE MS for a WAKE frame, will be != 0, it will be 0xFFFF for a SLEEP frame*/
						wake_ms = WuR_process_frame(context, 0);
						pinModeWaitFrame();
						WuR_operation = 0;
					    HAL_ResumeTick();
					    PIN_SET(GPIOA, WAKE_UP_FAST);
						if(wake_ms == 0x0001){
							app_wurx_ctxt.wurx_timestamp = current_tick + 10;
						}
					}
					current_tick = HAL_GetTick();
				}
				/* deactivate HSE and return to the default clock config with HSI*/
				SystemPower_wake();
				app_wurx_ctxt.wurx_status = WUR_SLEEPING;
				break;
			case WUR_WAIT_I2C:
			    HAL_ResumeTick();
			    do
			    {
			    	I2C_operation = 0;
					HAL_Delay(2);
					while(i2Cbusy());
			    }
			    while(I2C_operation);

			    app_wurx_ctxt.wurx_status = WUR_SLEEPING;
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
