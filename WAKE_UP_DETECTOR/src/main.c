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

/**
* @brief  Main program
* @param  None
* @retval None
*/
static void loopMain(wurx_context_t* context){

	while (1)
	{
		/* this one blocks until MCU wakes*/
		SystemPower_sleep();
		/* one awaken,. check is we woke cause of a WuR frame or an I2C transmission req*/

		if(WuR_operation){
			/* process WuR frame*/
			WuR_process_frame(context);
			/* cleanup HW used by WuR*/
			WuR_go_sleep(context);
			WuR_operation = 0;
		}
		if(I2C_operation){
			/*keep awake 10ms to let the host perform I2C operations*/
			HAL_Delay(2);
			I2C_operation = 0;
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

	/* Configure the system Power */
	SystemPower_Config();
	i2CConfig(&context, &I2cHandle);
	pinModeinit();
	TIMER_Config();
	COMP_Config(&hcomp1);
	//first test if I2C is working, just by default.

	loopMain(&context);

}
