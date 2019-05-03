/*
 * config_defines.h
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */

#ifndef CONFIG_DEFINES_H_
#define CONFIG_DEFINES_H_

//#define BUTTON_DEBUG
#define USE_CMP
//#define BUSY_WAIT
#define USE_ULP
#define MAX_LOOPS 20

#define COMP_NON_INVERTING ((uint16_t)GPIO_PIN_3)
#define COMP_INVERTING ((uint16_t)GPIO_PIN_4)
#define COMP_OUTPUT ((uint16_t)GPIO_PIN_11)
#define WAKE_UP_FAST ((uint16_t)GPIO_PIN_4)
#define ADDR_OK ((uint16_t)GPIO_PIN_9)
#define INPUT_FAST ((uint16_t)GPIO_PIN_10)

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_8
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1
#define I2C_ADDRESS        				0x30F
//TODO:Optimize this!
#define I2C_TIMING      				0x00B1112E /* 400 kHz with analog Filter ON, Rise Time 250ns, Fall Time 100ns */

/* Definition for I2Cx's NVIC */
#define I2Cx_IRQn                       I2C1_IRQn
#define I2Cx_IRQHandler                 I2C1_IRQHandler

#define DEFAULT_ADDRESS 0x5555

#endif /* CONFIG_DEFINES_H_ */
