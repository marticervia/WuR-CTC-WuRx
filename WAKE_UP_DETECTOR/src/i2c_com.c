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

/*
 * i2c_com.c
 *
 *  Created on: May 3, 2019
 *      Author: marti
 */
#include "i2c_com.h"

#include "user_handlers.h"
#include "config_defines.h"
#include "periph_config.h"
#include <string.h>

typedef struct i2c_context{
	i2c_state_t i2c_state;
	i2c_registers_t i2c_last_reg;
	i2c_operation_t i2c_last_operation;
	uint8_t i2c_frame_buffer[I2C_BUFFER_SIZE];
	I2C_HandleTypeDef* i2c_handle;
}i2c_context_t;

static wurx_context_t* wur_context = NULL;
volatile static i2c_context_t i2c_context = {0};
volatile static uint8_t pending_i2c_operation;

void reset_i2c_state(I2C_HandleTypeDef *I2cHandle){
	i2c_context.i2c_state = I2C_IDLE;
	i2c_context.i2c_last_reg = I2C_NONE_REGISTER;
	i2c_context.i2c_last_operation = I2C_NONE_OP;
	pending_i2c_operation = I2C_OP_NONE;
	CLEAR_TIMER_EXPIRED(TIM21);
	TIMER_DISABLE(TIM21);
}

static void reset_i2c_coms(I2C_HandleTypeDef *I2cHandle){

	HAL_I2C_DeInit(I2cHandle);
	i2CConfig(wur_context, I2cHandle);
	reset_i2c_state(I2cHandle);
}

void i2c_state_machine(void){
	uint8_t register_id;
	uint8_t operation_id;
	uint16_t address = 0;

	I2C_HandleTypeDef *I2cHandle = i2c_context.i2c_handle;

	if(IS_TIMER_EXPIRED(TIM21)){
		reset_i2c_state(I2cHandle);
	}

	if(pending_i2c_operation == I2C_OP_NONE){
		return;
	}
	if(pending_i2c_operation == I2C_ERROR){
		/* restore to the initial state!*/
		reset_i2c_coms(I2cHandle);
		return;
	}
	if((pending_i2c_operation == I2C_LISTEN_EVENT) || (pending_i2c_operation == I2C_ADDR_EVENT)){
		/* we have received a new I2C transaction without interrupt*/
		//i2c_notify_req_operation();
		return;
	}


	switch(i2c_context.i2c_state){
		case I2C_IDLE:
			reset_i2c_coms(I2cHandle);
			break;
		case I2C_WAITING_OPERATION:
			register_id = i2c_context.i2c_frame_buffer[0] >> 1;
			operation_id = i2c_context.i2c_frame_buffer[0] & 0x01;

			if(operation_id){
				operation_id = I2C_WRITE_OP;
			}else{
				operation_id = I2C_READ_OP;
			}

			switch(register_id){
				case I2C_STATUS_REGISTER:
					if(operation_id == I2C_WRITE_OP){
						//write is not supported on this register
						reset_i2c_coms(I2cHandle);
					}
					else if(operation_id == I2C_READ_OP){
						i2c_context.i2c_frame_buffer[0] = wur_context->wurx_state;
						i2c_context.i2c_frame_buffer[1] = wur_context->frame_len;

						if(HAL_I2C_Slave_Transmit_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, 2) != HAL_OK)
						{
						/* Transfer error in transmission process */
							reset_i2c_coms(I2cHandle);
						}
					}
					break;
				case I2C_ADDR_REGISTER:
					if(operation_id == I2C_WRITE_OP){
						if(HAL_I2C_Slave_Receive_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, 2) != HAL_OK)
						{
						/* Transfer error in transmission process */
							reset_i2c_coms(I2cHandle);
						}
					}
					else if(operation_id == I2C_READ_OP){
						address = WuR_get_hex_addr(wur_context);
						i2c_context.i2c_frame_buffer[0] = (address & 0x0F00) >> 8;
						i2c_context.i2c_frame_buffer[1] = address & 0xFF;

						if(HAL_I2C_Slave_Transmit_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, 2) != HAL_OK)
						{
						/* Transfer error in reception process */
							reset_i2c_coms(I2cHandle);
						}
					}
					break;
				case I2C_FRAME_REGISTER:
					if(operation_id == I2C_WRITE_OP){
						//write is not supported on this register
						reset_i2c_coms(I2cHandle);
					}
					uint8_t frame_len;

					if(wur_context->frame_len == 0){
						frame_len = 1;
					}else{
						frame_len = wur_context->frame_len;
					}

					memcpy((uint8_t*)i2c_context.i2c_frame_buffer, (uint8_t*)wur_context->frame_buffer, frame_len);
					if(HAL_I2C_Slave_Transmit_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, frame_len) != HAL_OK)
					{
					/* Transfer error in transmission process */
						reset_i2c_coms(I2cHandle);
					}
					WuR_clear_context((wurx_context_t*)wur_context);
					break;

				default:
					reset_i2c_coms(I2cHandle);
					return;
			}
			/* successful start of read/write operation, save status for completition.*/
			if(operation_id == I2C_WRITE_OP){
				i2c_context.i2c_state = I2C_PERFORM_WRITE;
			}else{
				i2c_context.i2c_state = I2C_PERFORM_READ;
			}

			i2c_context.i2c_last_reg = register_id;
			i2c_context.i2c_last_operation = operation_id;
			break;
		case I2C_PERFORM_WRITE:
			if(pending_i2c_operation != I2C_SUCCESS_READ){
				//wrong operation for the current state!
				reset_i2c_coms(I2cHandle);
				break;
			}
			if(i2c_context.i2c_last_reg == I2C_NONE_REGISTER){
				/* Should not happen,operation not started?!*/
				reset_i2c_coms(I2cHandle);
				break;
			}
			switch(i2c_context.i2c_last_reg){
				case I2C_ADDR_REGISTER:
					address = 0;
					address |= (i2c_context.i2c_frame_buffer[0] & 0x03) << 8;
					address |= i2c_context.i2c_frame_buffer[1];
					WuR_set_hex_addr(address, wur_context);
					break;
				default:
					//Operation not suported on write access
					break;
			}
			reset_i2c_state(I2cHandle);
			break;
		case I2C_PERFORM_READ:
			if(pending_i2c_operation != I2C_SUCCESS_WRITE){
				//wrong operation for the current state!
				reset_i2c_coms(I2cHandle);
				break;
			}
			if(i2c_context.i2c_last_reg == I2C_NONE_REGISTER){
				/* Should not happen,operation not started?!*/
				reset_i2c_coms(I2cHandle);
				break;
			}
			switch(i2c_context.i2c_last_reg){
			    case I2C_STATUS_REGISTER:
				case I2C_FRAME_REGISTER:
					/* as frame has been read, we can flush it and reset the start of the machine*/
					reset_i2c_state(I2cHandle);
					break;
				default:
					break;
			}
			break;
		default:
			reset_i2c_state(I2cHandle);
			break;
	}
	pending_i2c_operation = I2C_OP_NONE;
}
/**
  * @brief  Tx Transfer completed callback.
  * @param  I2cHandle: I2C handle.
  * @note   This example shows a simple way to report end of IT Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	pending_i2c_operation = I2C_SUCCESS_WRITE;
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	pending_i2c_operation = I2C_SUCCESS_READ;
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *I2cHandle, uint8_t TransferDirection, uint16_t AddrMatchCode){
	pending_i2c_operation = I2C_ADDR_EVENT;
}
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *I2cHandle){
	pending_i2c_operation = I2C_LISTEN_EVENT;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
  /** Error_Handler() function is called when error occurs.
    * 1- When Slave don't acknowledge it's address, Master restarts communication.
    * 2- When Master don't acknowledge the last data transferred, Slave don't care in this example.
    */
	pending_i2c_operation = I2C_ERROR;
}

void i2CConfig(wurx_context_t* context, I2C_HandleTypeDef *I2cHandle){
	  I2cHandle->Instance             = I2Cx;
	  I2cHandle->Init.Timing          = I2C_TIMING;
	  I2cHandle->Init.OwnAddress1     = I2C_ADDRESS;
	  I2cHandle->Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	  I2cHandle->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	  I2cHandle->Init.OwnAddress2     = 0xFF;
	  I2cHandle->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	  I2cHandle->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	  I2cHandle->Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

	  i2c_context.i2c_handle = I2cHandle;

	  if(HAL_I2C_Init(I2cHandle) != HAL_OK)
	  {
	    /* Initialization Error */
		  System_Error_Handler();
	  }

	  if(context == NULL){
		  /* error, context invalid*/
		  System_Error_Handler();
	  }

	  wur_context = context;
	  /* Enable the Analog I2C Filter */
	  HAL_I2CEx_ConfigAnalogFilter(I2cHandle,I2C_ANALOGFILTER_ENABLE);
	  //HAL_I2CEx_EnableWakeUp(I2cHandle);

}

uint8_t i2Cbusy(void){
	return i2c_context.i2c_state != I2C_IDLE;
}

void i2c_notify_req_operation(void){
	i2c_context.i2c_state = I2C_WAITING_OPERATION;
	TIMER_SET_PERIOD(TIM21, 2);
	TIMER_UIT_ENABLE(TIM21);
	TIMER_COMMIT_UPDATE(TIM21);
	TIMER_ENABLE(TIM21);
	HAL_I2C_Slave_Receive_IT(i2c_context.i2c_handle,(uint8_t*) i2c_context.i2c_frame_buffer, 1);
}




