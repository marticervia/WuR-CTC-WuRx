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

//128 bits plus 16 bits of headers and a crc 8
#define MAX_FRAME_LEN 19
#define COMP_NON_INVERTING ((uint16_t)GPIO_PIN_3)
#define COMP_INVERTING ((uint16_t)GPIO_PIN_4)
#define COMP_OUTPUT ((uint16_t)GPIO_PIN_11)
#define WAKE_UP_FAST ((uint16_t)GPIO_PIN_4)
#define ADDR_OK ((uint16_t)GPIO_PIN_9)
#define INPUT_FAST ((uint16_t)GPIO_PIN_10)

#define DEFAULT_ADDRESS 0x5555

#endif /* CONFIG_DEFINES_H_ */
