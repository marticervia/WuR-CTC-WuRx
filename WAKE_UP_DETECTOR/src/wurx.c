/*
 * wurx.c
 *
 *  Created on: May 15, 2019
 *      Author: marti
 */
#include "wurx.h"
#include "periph_config.h"
#include "power_config.h"
#include <string.h>

static const uint8_t CRC_8_TABLE[256] =
{
	  0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
	157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
	 35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
	190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
	 70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
	219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
	101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
	248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
	140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
	 17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
	175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
	 50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
	202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
	 87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
	233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
	116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};

static uint16_t expected_preamble[PREAMBLE_LEN] = {INPUT_FAST, INPUT_FAST};

/* the length of the output array must be at least 12!*/
void WuR_set_hex_addr(uint16_t input_addr, wurx_context_t* context){
	/* apply mask!*/
	input_addr = APPLY_ADDR_MASK(input_addr);

	/* from MSB to LSB*/
	for(uint8_t bit_index = 0; bit_index < ADDR_LEN; bit_index++){
		context->wurx_address[bit_index] = (input_addr &  (1 << (11 - bit_index))) ? INPUT_FAST : 0;
	}
}

uint16_t WuR_get_hex_addr(wurx_context_t* context){
	uint16_t wur_addr = 0;
	for(uint8_t bit_index = 0; bit_index < ADDR_LEN; bit_index++){
		if(context->wurx_address[bit_index]){
			wur_addr |= 1 << (11 - bit_index);
		}
	}
	return wur_addr;
}

void WuR_clear_buffer(wurx_context_t* context){
	memset(context->frame_buffer, 0, MAX_FRAME_LEN);
	context->frame_len = 0;
}

void WuR_clear_context(wurx_context_t* context){
	context->wurx_state = WURX_SLEEP;
	memset(context->frame_buffer, 0, MAX_FRAME_LEN);
	context->frame_len = 0;
}


void WuR_init_context(wurx_context_t* context){
	WuR_clear_context(context);
	WuR_set_hex_addr(DEFAULT_ADDRESS, context);
}


void WuR_set_frame_buffer(wurx_context_t* context, uint8_t* buffer, uint8_t length){
	uint8_t num_byte_loops = length/8;

	WuR_clear_buffer(context);

	for(uint8_t byte_loop = 0; byte_loop < num_byte_loops; byte_loop++){
		for(uint8_t bit_loop = 0; bit_loop < 8; bit_loop++){
			context->frame_buffer[byte_loop] |= (buffer[bit_loop + (byte_loop*8)] << (7 - bit_loop));
		}
	}
	context->frame_len = num_byte_loops;
}

uint8_t WuR_is_CRC_good(wurx_context_t* context){
	uint16_t i;
	uint8_t crc_num = 0;

	/* do not CRC the CRC!*/
	for (i=0; i < context->frame_len -1; i++){
		crc_num = CRC_8_TABLE[crc_num ^ context->frame_buffer[i]];
	}

	return crc_num == context->frame_buffer[context->frame_len - 1];
}

void WuR_go_sleep(wurx_context_t* wur_context){

    HAL_ResumeTick();

	if(wur_context->wurx_state != WURX_HAS_FRAME){
		wur_context->wurx_state = WURX_SLEEP;
		wur_context->frame_len = 0;
	}
	SystemPower_prepare_sleep();
}

/* initialization not really required */
static uint8_t frame_buffer[200] = {0};

void WuR_process_frame(wurx_context_t* context){
	uint32_t result = 0;
	uint8_t loop = 0;
	uint16_t offset = 0;
	uint8_t length = 0;

    HAL_SuspendTick();

	if(context->wurx_state == WURX_HAS_FRAME){
		return;
	}
	/*wait 64.25 us for operation completition */
	context->wurx_state = WURX_DECODING_FRAME;


	/* wait for preamble init.*/
	__TIM2_CLK_ENABLE();
	/* block for 60 us @ 16 ticks x us*/
	TIMER_SET_PERIOD(TIM2, 270);
	TIMER_COMMIT_UPDATE(TIM2);
	CLEAR_TIMER_EXPIRED(TIM2);
	TIMER_ENABLE(TIM2);
	PIN_RESET(GPIOA, WAKE_UP_FAST);

	/* finish waiting for preamble start */
	while(!IS_TIMER_EXPIRED(TIM2));
	/* arm sample timer */
	TIMER_SET_PERIOD(TIM2, 63);
	TIMER_COMMIT_UPDATE(TIM2);
	CLEAR_TIMER_EXPIRED(TIM2);

	/* match preamble!*/
	for(loop = 0; loop < PREAMBLE_LEN; loop++){
		while(!IS_TIMER_EXPIRED(TIM2));
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		result = READ_PIN(GPIOA, INPUT_FAST);
		CLEAR_TIMER_EXPIRED(TIM2);

		if(result != expected_preamble[loop]){
			PIN_SET(GPIOA, ADDR_OK);
			PIN_RESET(GPIOA, ADDR_OK);
			PIN_SET(GPIOA, ADDR_OK);
			PIN_RESET(GPIOA, ADDR_OK);
			return;
		}
	}

	/* match address!*/
	//64 instructions loop at 16MHz
	for(loop = 0; loop < ADDR_LEN; loop++){
		while(!IS_TIMER_EXPIRED(TIM2));
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		result = READ_PIN(GPIOA, INPUT_FAST);
		CLEAR_TIMER_EXPIRED(TIM2);
		if(result != context->wurx_address[loop]){
			PIN_SET(GPIOA, ADDR_OK);
			PIN_RESET(GPIOA, ADDR_OK);
			PIN_SET(GPIOA, ADDR_OK);
			PIN_RESET(GPIOA, ADDR_OK);
			PIN_SET(GPIOA, ADDR_OK);
			PIN_RESET(GPIOA, ADDR_OK);
			WuR_clear_buffer(context);
			return;
		}

		frame_buffer[offset] = (result != 0);
		offset++;
	}

	/* now decode frame type, 3 bits */

	while(!IS_TIMER_EXPIRED(TIM2));
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	frame_buffer[offset] = (READ_PIN(GPIOA, INPUT_FAST) != 0);
	CLEAR_TIMER_EXPIRED(TIM2);

	offset++;

	while(!IS_TIMER_EXPIRED(TIM2));
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	frame_buffer[offset] = (READ_PIN(GPIOA, INPUT_FAST) != 0);
	CLEAR_TIMER_EXPIRED(TIM2);

	offset++;

	while(!IS_TIMER_EXPIRED(TIM2));
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	frame_buffer[offset] = (READ_PIN(GPIOA, INPUT_FAST) != 0);
	CLEAR_TIMER_EXPIRED(TIM2);

	offset++;

	/* now decode seq number */
	while(!IS_TIMER_EXPIRED(TIM2));
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	frame_buffer[offset] = (READ_PIN(GPIOA, INPUT_FAST) != 0);
	CLEAR_TIMER_EXPIRED(TIM2);

	offset++;

	/* now decode length! */

	for(loop = 0; loop < LENGTH_LEN; loop++){
		while(!IS_TIMER_EXPIRED(TIM2));
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		result = (READ_PIN(GPIOA, INPUT_FAST) != 0);
		CLEAR_TIMER_EXPIRED(TIM2);

		if(result){
			length |= 1 << (7 - loop);
		}

		frame_buffer[offset] = result;
		offset++;
	}

	/* now we have length, read the rest of bits!*/
	for(loop = 0; loop < (length*8) + 8; loop++){
		while(!IS_TIMER_EXPIRED(TIM2));
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		frame_buffer[offset] = (READ_PIN(GPIOA, INPUT_FAST) != 0);
		CLEAR_TIMER_EXPIRED(TIM2);

		offset++;
	}

	/* well, now the frame is over, let's just process it */

	/* first, move from bits to bytes! */
	WuR_set_frame_buffer(context, frame_buffer, offset);

	/* is CRC ok? */
	if(!WuR_is_CRC_good(context)){
		PIN_SET(GPIOA, ADDR_OK);
		PIN_RESET(GPIOA, ADDR_OK);
		PIN_SET(GPIOA, ADDR_OK);
		PIN_RESET(GPIOA, ADDR_OK);
		PIN_SET(GPIOA, ADDR_OK);
		PIN_RESET(GPIOA, ADDR_OK);
		PIN_SET(GPIOA, ADDR_OK);
		PIN_RESET(GPIOA, ADDR_OK);
		WuR_clear_buffer(context);
		return;
	}

	/* notify host that we have a frame ready via interrupt and change state accordingly*/
	PIN_SET(GPIOA, ADDR_OK);
	ADJUST_WITH_NOPS;
	ADJUST_WITH_NOPS;
	PIN_RESET(GPIOA, ADDR_OK);

	context->wurx_state = WURX_HAS_FRAME;

}

