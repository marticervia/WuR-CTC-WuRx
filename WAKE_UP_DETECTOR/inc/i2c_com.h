/*
 * i2c_com.h
 *
 *  Created on: May 3, 2019
 *      Author: marti
 */

#ifndef I2C_COM_H_
#define I2C_COM_H_

#include "main.h"

#define I2Cx                            I2C1
#define RCC_PERIPHCLK_I2Cx              RCC_PERIPHCLK_I2C1
#define RCC_I2CxCLKSOURCE_SYSCLK        RCC_I2C1CLKSOURCE_SYSCLK
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()
#define I2C_BUFFER_SIZE 32

void i2CConfig(wurx_context_t* context, I2C_HandleTypeDef *I2cHandle);

typedef enum i2c_result{
	I2C_SUCCESS_READ = 0,
	I2C_SUCCESS_WRITE = 1,
	I2C_ERROR = 2,
	I2C_ADDR_EVENT =3,
	I2C_LISTEN_EVENT = 4
}i2c_result_t;

typedef enum i2c_operation{
	I2C_NONE_OP = 0,
	I2C_READ_OP = 1,
	I2C_WRITE_OP = 2
}i2c_operation_t;

typedef enum i2c_registers{
	I2C_NONE_REGISTER = 0,
	I2C_STATUS_REGISTER = 1,
	I2C_ADDR_REGISTER = 2,
	I2C_FRAME_REGISTER = 3,
	I2C_FRAME_READY_REGISTER = 4
}i2c_registers_t;

typedef enum i2c_state{
	I2C_WAITING_OPERATION = 0,
	I2C_PERFORM_WRITE = 1,
	I2C_PERFORM_READ = 2
}i2c_state_t;

#endif /* I2C_COM_H_ */
