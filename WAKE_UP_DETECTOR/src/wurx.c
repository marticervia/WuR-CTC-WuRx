/*
 * wurx.c
 *
 *  Created on: May 15, 2019
 *      Author: marti
 */
#include "wurx.h"
#include "periph_config.h"
#include "config_defines.h"
#include "power_config.h"
#include <string.h>

/* CRC table for polynomial CRC-8 0x07*/
static const uint8_t CRC_8_TABLE[256] =
{
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

static uint32_t expected_preamble[PREAMBLE_LEN] = {COMP_VALUE, COMP_VALUE};

/* the length of the output array must be at least 12!*/
void WuR_set_hex_addr(uint16_t input_addr, wurx_context_t* context){
	/* apply mask!*/
	input_addr = APPLY_ADDR_MASK(input_addr);

	/* from MSB to LSB*/
	for(uint8_t bit_index = 0; bit_index < ADDR_LEN; bit_index++){
		context->wurx_address[bit_index] = (input_addr &  (1 << (9 - bit_index))) ? COMP_VALUE : 0;
	}
}

uint16_t WuR_get_hex_addr(wurx_context_t* context){
	uint16_t wur_addr = 0;
	for(uint8_t bit_index = 0; bit_index < ADDR_LEN; bit_index++){
		if(context->wurx_address[bit_index]){
			wur_addr |= 1 << (9 - bit_index);
		}
	}
	return wur_addr;
}

void WuR_clear_buffer(wurx_context_t* context){
	memset(context->frame_buffer, 0, 3);
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

	if(wur_context->wurx_state != WURX_HAS_FRAME){
		wur_context->wurx_state = WURX_SLEEP;
		wur_context->frame_len = 0;
	}
	SystemPower_prepare_sleep();
}

/* initialization not really required */
static uint8_t frame_buffer[32] = {0};

int32_t WuR_process_frame(wurx_context_t* context, uint8_t from_sleep){
	uint32_t result = 0;
	uint16_t loop = 0, byte = 0;
	uint16_t offset = 0, wake_ms = 0;
	uint16_t length = 0;
	uint8_t last_results[3] = {0};
	last_results[1] = 1;
	last_results[2] = 1;

	if(context->wurx_state == WURX_HAS_FRAME){
		/* notify host via interrupt that a frame is still pending*/
		return -1;
	}
	/*wait 64.25 us for operation completition */
	context->wurx_state = WURX_DECODING_FRAME;

	PIN_RESET(GPIOA, WAKE_UP_FAST);
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);

	/* wait for preamble init.*/
	__TIM2_CLK_ENABLE();
	/* block for 60 us @ 16 ticks x us*/
	if(from_sleep){
#ifndef USE_GPIO
		TIMER_SET_PERIOD(TIM2, 297);
		TIMER_COMMIT_UPDATE(TIM2);
		CLEAR_TIMER_EXPIRED(TIM2);
		TIMER_ENABLE(TIM2);

		/* finish waiting for preamble start */
		while(!IS_TIMER_EXPIRED(TIM2));
		CLEAR_TIMER_EXPIRED(TIM2);
#else
		ALIGN_WITH_NOPS;
#endif

		TIMER_SET_PERIOD(TIM2, 63);
		TIMER_COMMIT_UPDATE(TIM2);

		while(!IS_TIMER_EXPIRED(TIM2));
		CLEAR_TIMER_EXPIRED(TIM2);
		for(loop = 0; loop < PREAMBLE_MATCHING_LEN; loop++){
			while(!IS_TIMER_EXPIRED(TIM2));
			CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
			result = COMP_READ(COMP2);
#else
			result = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);
#endif
			PIN_SET(GPIOA, WAKE_UP_FAST);
			PIN_RESET(GPIOA, WAKE_UP_FAST);

			if(result && last_results[0]){
				/* We have a preamble match! */
				break;
			}
			else if(!result && !last_results[0] && !last_results[1] && !last_results[2]){
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				return -2;
			}

			last_results[2] = last_results[1];
			last_results[1] = last_results[0];
			last_results[0] = result;

			if(loop == PREAMBLE_MATCHING_LEN -1){
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				return -2;
			}
		}
	}
	else{
		uint8_t last_result = 0, last_last_result = 1;
		TIMER_SET_PERIOD(TIM2, 890);
		TIMER_COMMIT_UPDATE(TIM2);
		CLEAR_TIMER_EXPIRED(TIM2);
		TIMER_ENABLE(TIM2);

		/* finish waiting for preamble start */
		while(!IS_TIMER_EXPIRED(TIM2));
		/* arm sample timer */
		TIMER_SET_PERIOD(TIM2, 63);
		TIMER_COMMIT_UPDATE(TIM2);
		CLEAR_TIMER_EXPIRED(TIM2);

		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);

		for(loop = 0; loop < PREAMBLE_MATCHING_LEN; loop++){
			while(!IS_TIMER_EXPIRED(TIM2));
			CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
			result = COMP_READ(COMP2);
#else
			result = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);
#endif
			PIN_SET(GPIOA, WAKE_UP_FAST);
			PIN_RESET(GPIOA, WAKE_UP_FAST);
			if(result && last_results[0]){
				/* We have a preamble match! */
				break;
			}
			else if(!result && !last_results[0] && !last_results[1] && !last_results[2]){
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				return -2;
			}

			last_results[2] = last_results[1];
			last_results[1] = last_results[0];
			last_results[0] = result;

			if(loop == PREAMBLE_MATCHING_LEN -1){
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				PIN_SET(GPIOA, WAKE_UP_FAST);
				PIN_RESET(GPIOA, WAKE_UP_FAST);
				return -2;
			}
		}
	}

	/* match address!*/
	//64 instructions loop at 16MHz
	for(loop = 0; loop < ADDR_LEN; loop++){
		while(!IS_TIMER_EXPIRED(TIM2));
		CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
		result = COMP_READ(COMP2);
#else
		result = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);

#endif
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		if(result != context->wurx_address[loop]){
			PIN_SET(GPIOA, WAKE_UP_FAST);
			PIN_RESET(GPIOA, WAKE_UP_FAST);
			PIN_SET(GPIOA, WAKE_UP_FAST);
			PIN_RESET(GPIOA, WAKE_UP_FAST);
			PIN_SET(GPIOA, WAKE_UP_FAST);
			PIN_RESET(GPIOA, WAKE_UP_FAST);
			WuR_clear_buffer(context);
			return -3;
		}

		frame_buffer[offset] = (result != 0);
		offset++;
	}

	/* store sender address*/
	for(loop = 0; loop < ADDR_LEN; loop++){
		while(!IS_TIMER_EXPIRED(TIM2));
		CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
		result = COMP_READ(COMP2);
#else
		result = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);

#endif
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		frame_buffer[offset] = (result != 0);
		offset++;
	}


	/* now decode frame type, 3 bits */

	while(!IS_TIMER_EXPIRED(TIM2));
	CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
	frame_buffer[offset] = COMP_READ(COMP2);
#else
	frame_buffer[offset] = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);
#endif
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	offset++;

	while(!IS_TIMER_EXPIRED(TIM2));
	CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
	frame_buffer[offset] = COMP_READ(COMP2);
#else
	frame_buffer[offset] = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);

#endif
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	offset++;

	while(!IS_TIMER_EXPIRED(TIM2));
	CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
	frame_buffer[offset] = COMP_READ(COMP2);
#else
	frame_buffer[offset] = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);

#endif
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	offset++;

	/* now decode seq number */
	while(!IS_TIMER_EXPIRED(TIM2));
	CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
	frame_buffer[offset] = COMP_READ(COMP2);
#else
	frame_buffer[offset] = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);

#endif
	PIN_SET(GPIOA, WAKE_UP_FAST);
	PIN_RESET(GPIOA, WAKE_UP_FAST);
	offset++;

	/* now decode length! */

	for(loop = 0; loop < LENGTH_LEN; loop++){
		while(!IS_TIMER_EXPIRED(TIM2));
		CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
		result = COMP_READ(COMP2);
#else
		result = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);

#endif
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		if(result){
			length |= 1 << (7 - loop);
		}

		frame_buffer[offset] = result;
		offset++;
	}

	if(length > MAX_LEN_DATA_FRAME){
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		WuR_clear_buffer(context);
		return -4;
	}

	/* now we have length, read the rest of bits!*/

	for(byte = 0; byte < length + 1; byte++){
		uint8_t byte_res = 0;

		for(loop = 0; loop < 8; loop++){
			while(!IS_TIMER_EXPIRED(TIM2));
			CLEAR_TIMER_EXPIRED(TIM2);
#ifdef USE_CMP
			result = COMP_READ(COMP2);
#else
			result = READ_PIN(GPIOA, INPUT_FAST, INPUT_FAST_NUM);
#endif
			PIN_SET(GPIOA, WAKE_UP_FAST);
			PIN_RESET(GPIOA, WAKE_UP_FAST);
			if(result){
				byte_res |= 1 << (7 - loop);
			}

			context->frame_buffer[WUR_DATA_OFFSET_BYTES + byte] = byte_res;
		}
	}

	/* well, now the frame is over, let's just process it */

	/* first, move from header bits to bytes! */
	WuR_set_frame_buffer(context, frame_buffer, 32);
	context->frame_len += length + 1;

	/* is CRC ok? */
	if(!WuR_is_CRC_good(context)){
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		PIN_SET(GPIOA, WAKE_UP_FAST);
		PIN_RESET(GPIOA, WAKE_UP_FAST);
		WuR_clear_buffer(context);
		return -4;
	}

	/* a WuR WAKE/SLEEP frame is present*/
	if(length == 2 && ((context->frame_buffer[WUR_FLAGS_OFFSET_BYTES] & 0x0E) == 0b0010)){
		memcpy(&wake_ms, &context->frame_buffer[WUR_DATA_OFFSET_BYTES], 2);
		wake_ms = ntohs(wake_ms);
	}

	context->wurx_state = WURX_HAS_FRAME;

    /* return wake_time to notify the WuRx the time in ms it must stay awake.*/
	return wake_ms;

}

