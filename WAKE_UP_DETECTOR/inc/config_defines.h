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


#ifndef USE_CMP
#define INTERRUPT_PIN GPIO_PIN_8
#endif

#ifdef USE_CMP
	#define COMP_NON_INVERTING GPIO_PIN_3
	#define COMP_INVERTING GPIO_PIN_4
	#define COMP_OUTPUT GPIO_PIN_12
#endif

#define OUTPUT_PIN GPIO_PIN_9


#endif /* CONFIG_DEFINES_H_ */
