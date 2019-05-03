/*
 * comp_config.h
 *
 *  Created on: Jul 16, 2018
 *      Author: marti
 */

#ifndef PERIPH_CONFIG_H_
#define PERIPH_CONFIG_H_
#include "main.h"

/* HAL layer uses asserts and multiple function calls, it is SLOW.*/
/* Use macros at registry level to skimp cycles. */

/*pins*/
#define PIN_SET(GPIOx, GPIO_Pin)  (GPIOx->BSRR = GPIO_Pin)
#define PIN_RESET(GPIOx, GPIO_Pin)  (GPIOx->BRR = GPIO_Pin)
/* returns 0 when not set, > 0 when set*/
#define READ_PIN(GPIOx, GPIO_Pin)   (GPIOx->IDR & GPIO_Pin)
/*timers, works with all CC1 timers: TIM2, TIM21 and TIM22*/
#define TIMER_ENABLE(TIMx) 											\
							do{										\
								TIMx->CR1 |= TIM_CR1_CEN;			\
							}while(0)
#define TIMER_DISABLE(TIMx) 										\
							do{										\
								TIMx->CR1 &= ~(TIM_CR1_CEN);		\
							}while(0)
#define TIMER_SET_COUNTING_MODE(TIMx, mode)							\
					do{												\
						TIMx->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);	\
						TIMx->CR1 |= mode;							\
					}while(0)
#define TIMER_SET_CLOCK_DIVISOR(TIMx, div) 							\
					do{												\
						TIMx->CR1 &= ~TIM_CR1_CKD;					\
						TIMx->CR1 |= div;							\
					}while(0)

#define TIMER_SET_PRELOAD(TIMx, div) 							\
					do{												\
						TIMx->CR1 |= ~TIM_CR1_CKD;					\
						TIMx->CR1 |= div;							\
					}while(0)
#define TIMER_CLEAR_PRELOAD(TIMx, div) 							\
					do{												\
						TIMx->CR1 &= ~TIM_CR1_CKD;					\
						TIMx->CR1 |= div;							\
					}while(0)

#define ADJUST_WITH_NOPS \
    __asm__ __volatile__ ( \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
		"nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
		"nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
		"nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
		"nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
        "nop\r\n" \
		"nop\r\n" \
        "nop\r\n" \
    );

#define APPLY_ADDR_MASK(addr) (addr & 0x3F)
/* use ALWAYS values < 0xffff for both of this macros*/
#define TIMER_SET_PERIOD(TIMx, period) (TIMx->ARR = (uint16_t)period)
#define TIMER_SET_PRESCALER(TIMx, prescaler) (TIMx->PSC = (uint16_t)prescaler)
#define TIMER_COMMIT_UPDATE(TIMx) (TIMx->EGR = TIM_EGR_UG)
#define IS_TIMER_EXPIRED(TIMx) ((TIMx->SR &(TIM_FLAG_UPDATE)) == (TIM_FLAG_UPDATE))
#define CLEAR_TIMER_EXPIRED(TIMx) (TIMx->SR &= ~(TIM_FLAG_UPDATE))

void COMP_Config(COMP_HandleTypeDef* hcomp1);
void TIMER_Config();
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp);
void pinModeSleep(void);
void pinModeAwake(void);
void pinModeinit(void);

#endif /* PERIPH_CONFIG_H_ */
