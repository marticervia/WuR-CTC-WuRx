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


#define COMP_NON_INVERTING GPIO_PIN_3
#define COMP_INVERTING GPIO_PIN_4
#define COMP_OUTPUT GPIO_PIN_11
#define WAKE_UP_FAST GPIO_PIN_4
#define ADDR_OK GPIO_PIN_3
#define INPUT_FAST GPIO_PIN_2

#define DEFAULT_ADDRESS 0x5555

#endif /* CONFIG_DEFINES_H_ */
