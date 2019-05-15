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
}i2c_context_t;

volatile static wurx_context_t* wur_context = NULL;
volatile static i2c_context_t i2c_context = {0};

static void reset_i2c_state(I2C_HandleTypeDef *I2cHandle){

	i2c_context.i2c_state = I2C_WAITING_OPERATION;
	i2c_context.i2c_last_reg = I2C_NONE_REGISTER;
	i2c_context.i2c_last_operation = I2C_NONE_OP;
	memset((uint8_t*)i2c_context.i2c_frame_buffer, 0, I2C_BUFFER_SIZE);

	if(HAL_I2C_Slave_Receive_IT(I2cHandle,(uint8_t*) i2c_context.i2c_frame_buffer, 1) != HAL_OK)
	{
	/* Transfer error in reception process */
		System_Error_Handler();
	}
}

static void i2c_state_machine(uint8_t i2c_operation, I2C_HandleTypeDef *I2cHandle){
	uint8_t register_id;
	uint8_t operation_id;

	if(i2c_operation == I2C_ERROR){
		/* restore to the initial state!*/
		reset_i2c_state(I2cHandle);
		return;
	}

	switch(i2c_context.i2c_state){
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
						reset_i2c_state(I2cHandle);
					}
					else{
						i2c_context.i2c_frame_buffer[0] = wur_context->wurx_state;
						i2c_context.i2c_frame_buffer[1] = wur_context->frame_len;

						if(HAL_I2C_Slave_Transmit_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, 2) != HAL_OK)
						{
						/* Transfer error in transmission process */
							System_Error_Handler();
							return;
						}
					}
					break;
				case I2C_ADDR_REGISTER:
					if(operation_id == I2C_WRITE_OP){
						if(HAL_I2C_Slave_Receive_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, 2) != HAL_OK)
						{
						/* Transfer error in transmission process */
							System_Error_Handler();
							return;
						}

					}
					else{
						i2c_context.i2c_frame_buffer[0] = (wur_context->wurx_address & 0x0F00) >> 8;
						i2c_context.i2c_frame_buffer[1] = wur_context->wurx_address & 0xFF;

						if(HAL_I2C_Slave_Transmit_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, 2) != HAL_OK)
						{
						/* Transfer error in reception process */
							System_Error_Handler();
							return;
						}
					}
					break;
				case I2C_FRAME_READY_REGISTER:
					if(operation_id == I2C_WRITE_OP){
						//write is not supported on this register
						reset_i2c_state(I2cHandle);
					}

					memcpy((uint8_t*)i2c_context.i2c_frame_buffer, (uint8_t*)wur_context->frame_buffer, wur_context->frame_len);
					if(HAL_I2C_Slave_Transmit_IT(I2cHandle, (uint8_t*) i2c_context.i2c_frame_buffer, wur_context->frame_len) != HAL_OK)
					{
					/* Transfer error in transmission process */
						System_Error_Handler();
						return;
					}
					break;

				default:
					reset_i2c_state(I2cHandle);
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
			if(i2c_operation != I2C_SUCCESS_READ){
				//wrong operation for the current state!
				reset_i2c_state(I2cHandle);
			}
			if(i2c_context.i2c_last_reg == I2C_NONE_REGISTER){
				/* Should not happen,operation not started?!*/
				reset_i2c_state(I2cHandle);
				return;
			}
			switch(i2c_context.i2c_last_reg){
				case I2C_ADDR_REGISTER:
					wur_context->wurx_address = 0;
					wur_context->wurx_address |= (i2c_context.i2c_frame_buffer[0] & 0x0F) << 8;
					wur_context->wurx_address |= i2c_context.i2c_frame_buffer[1];
					reset_i2c_state(I2cHandle);
					break;
				default:
					//Operation not suported on write access
					reset_i2c_state(I2cHandle);
					return;
			}
			break;
		case I2C_PERFORM_READ:
			if(i2c_operation != I2C_SUCCESS_WRITE){
				//wrong operation for the current state!
				reset_i2c_state(I2cHandle);
			}
			if(i2c_context.i2c_last_reg == I2C_NONE_REGISTER){
				/* Should not happen,operation not started?!*/
				reset_i2c_state(I2cHandle);
				return;
			}
			switch(i2c_context.i2c_last_reg){
				case I2C_FRAME_READY_REGISTER:
					/* as frame has been read, we can flush it and reset the start of the machine*/
					WuR_init_context((wurx_context_t*)wur_context);
					reset_i2c_state(I2cHandle);
					break;
				default:
					break;
			}
			reset_i2c_state(I2cHandle);
			break;
		default:
			break;
	}
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
  i2c_state_machine(I2C_SUCCESS_WRITE, I2cHandle);
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
  i2c_state_machine(I2C_SUCCESS_READ, I2cHandle);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *I2cHandle, uint8_t TransferDirection, uint16_t AddrMatchCode){
  i2c_state_machine(I2C_ADDR_EVENT, I2cHandle);
}
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *I2cHandle){
  i2c_state_machine(I2C_LISTEN_EVENT, I2cHandle);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
  /** Error_Handler() function is called when error occurs.
    * 1- When Slave don't acknowledge it's address, Master restarts communication.
    * 2- When Master don't acknowledge the last data transferred, Slave don't care in this example.
    */
  if (HAL_I2C_GetError(I2cHandle) != HAL_I2C_ERROR_AF)
  {
	  System_Error_Handler();
  }
  i2c_state_machine(I2C_ERROR, I2cHandle);
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

	  reset_i2c_state(I2cHandle);
}




