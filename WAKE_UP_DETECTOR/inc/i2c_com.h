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
 * i2c_com.h
 *
 *  Created on: May 3, 2019
 *      Author: marti
 */

#ifndef I2C_COM_H_
#define I2C_COM_H_

#include "main.h"
#include "wurx.h"

#define I2Cx                            I2C1
#define RCC_PERIPHCLK_I2Cx              RCC_PERIPHCLK_I2C1
#define RCC_I2CxCLKSOURCE_SYSCLK        RCC_I2C1CLKSOURCE_SYSCLK
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()
#define I2C_BUFFER_SIZE 128

void i2CConfig(wurx_context_t* context, I2C_HandleTypeDef *I2cHandle);
uint8_t i2Cbusy(void);
void reset_i2c_state(I2C_HandleTypeDef *I2cHandle);
void i2c_notify_req_operation(void);
void i2c_state_machine(void);

typedef enum i2c_result{
	I2C_SUCCESS_READ = 0,
	I2C_SUCCESS_WRITE = 1,
	I2C_ERROR = 2,
	I2C_ADDR_EVENT =3,
	I2C_LISTEN_EVENT = 4,
	I2C_OP_NONE = 5
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
	I2C_FRAME_REGISTER = 3
}i2c_registers_t;

typedef enum i2c_state{
	I2C_IDLE = 0,
	I2C_WAITING_OPERATION = 1,
	I2C_PERFORM_WRITE = 2,
	I2C_PERFORM_READ = 3
}i2c_state_t;

#endif /* I2C_COM_H_ */
