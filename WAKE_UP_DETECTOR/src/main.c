#include <periph_config.h>
#include "stm32l0xx_hal_conf.h"
#include "user_handlers.h"
#include "config_defines.h"
#include "power_config.h"
#include "i2c_com.h"
#include "wurx.h"

COMP_HandleTypeDef hcomp1;
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

static wurx_ctxt_t wurx_ctxt = {
		.wurx_status = WUR_SLEEPING,
		.wurx_timestamp = 0
};

static void loopMain(wurx_context_t* context){
	uint16_t wake_ms;
	uint32_t current_tick;

	while (1)
	{
		switch(wurx_ctxt.wurx_status){
			case WUR_SLEEPING:
				WuR_go_sleep(context);
				/* configure on wakeup for HSI use*/
				SystemPower_sleep();
				if(WuR_operation){
					wake_ms = WuR_process_frame(context, 1);
					WuR_operation = 0;
					if(!wake_ms){
						break;
					}
					/* activate HSE for use*/
					SystemPower_data();
					wurx_ctxt.wurx_timestamp = HAL_GetTick() + wake_ms;
					wurx_ctxt.wurx_status = WUR_WAIT_DATA;
				}else if(I2C_operation){
					I2C_operation = 0;
					wurx_ctxt.wurx_timestamp = HAL_GetTick() + 10;
					wurx_ctxt.wurx_status = WUR_WAIT_I2C;
				}
				break;
			case WUR_WAIT_DATA:
				current_tick = HAL_GetTick();
				/* activate again the reception interrupt*/
				pinModeWaitFrame();
				/* loop used inside the state to minimize jitter*/
				while(current_tick < wurx_ctxt.wurx_timestamp){
					if(WuR_operation){
					    PIN_SET(GPIOA, WAKE_UP_FAST);
						pinModeFrameReceived();
						PIN_RESET(GPIOA, WAKE_UP_FAST);
					    WuR_process_frame(context, 0);
						pinModeWaitFrame();
						WuR_operation = 0;
					}
					current_tick = HAL_GetTick();
				}
				/* deactivate HSE and return to the default clock config with HSI*/
				SystemPower_wake();
				wurx_ctxt.wurx_status = WUR_SLEEPING;
				break;
			case WUR_WAIT_I2C:
				current_tick = HAL_GetTick();
				while(current_tick < wurx_ctxt.wurx_timestamp){
					HAL_Delay(10);
					break;
				}
				wurx_ctxt.wurx_status = WUR_SLEEPING;
				break;
			default:
				wurx_ctxt.wurx_status = WUR_SLEEPING;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == WAKE_UP_I2C)
  {
	  /* flag the start of an I2C operation */
	  I2C_operation = 1;
  }
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
	pinModeinit();
	TIMER_Config();
	COMP_Config(&hcomp1);

	loopMain(&context);

}
