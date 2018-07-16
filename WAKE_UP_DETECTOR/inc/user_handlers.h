
#ifndef __USER_HANDLERS
#define __USER_HANDLERS
#include "stm32l0xx_hal.h"
#include "stm32l0xx_nucleo.h"

void System_Error_Handler(void);
void HAL_SYSTICK_Callback(void);
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line);
#endif

#endif

