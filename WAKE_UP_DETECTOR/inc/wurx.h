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

#ifndef WURX_H_
#define WURX_H_
#include "main.h"

#define DEFAULT_ADDRESS 0x0555

#define PREAMBLE_MATCHING_LEN 14
#define PREAMBLE_LEN 2
#define ADDR_LEN 10
#define FLAGS_LEN 4
#define LENGTH_LEN 8
#define WUR_DATA_OFFSET_BYTES 4
#define WUR_FLAGS_OFFSET_BYTES 2

#define SAMPLE_THRESHOLD 6

#define LEN_ACK_FRAME 8
#define LEN_WUR_FRAME 8
#define MAX_LEN_DATA_FRAME 128

#define DEFAULT_WAKE_TIMEOUT 10000

#define MAX_FRAME_LEN (MAX_LEN_DATA_FRAME + WUR_DATA_OFFSET_BYTES + 1)
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
typedef enum wurx_frame_type{
	WURX_FRAME_WAKEUP = 0,
	WURX_FRAME_ACK = 1,
	WURX_FRAME_DATA = 2
}wurx_frame_type_t;

typedef enum wurx_states{
	WURX_SLEEP = 0,
	WURX_DECODING_FRAME = 1,
	WURX_HAS_FRAME = 2,
}wurx_states_t;

typedef struct wurx_context{
	wurx_states_t wurx_state;
	wurx_states_t wurx_osc_active;
	uint32_t wurx_address[ADDR_LEN];
	uint8_t frame_len;
	uint8_t frame_buffer[MAX_FRAME_LEN];
}wurx_context_t;

#define htons(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
(((uint16_t)(A) & 0x00ff) << 8))
#define ntohs htons


#define APPLY_ADDR_MASK(addr) (addr & 0x03FF)

void WuR_init_context(wurx_context_t* context);
void WuR_clear_context(wurx_context_t* context);
void WuR_clear_buffer(wurx_context_t* context);
void WuR_set_frame_buffer(wurx_context_t* context, uint8_t* buffer, uint8_t length);
uint8_t WuR_is_CRC_good(wurx_context_t* context);

int32_t WuR_process_frame(wurx_context_t* context, uint8_t from_sleep);

void WuR_go_sleep(wurx_context_t* wur_context);
void WuR_set_hex_addr(uint16_t input_addr, wurx_context_t* context);
uint16_t WuR_get_hex_addr(wurx_context_t* context);

#endif
