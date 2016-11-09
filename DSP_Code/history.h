#ifndef _HISTORY_H_
#define _HISTORY_H_

#include "global.h"

#define hist_idx(comp_buff, i) ((comp_buff->curr + i + comp_buff->size) % comp_buff->size)
#define hist(comp_buff, i) comp_buff->buff[hist_idx(comp_buff, i)]

// For a required history of length M, find M' >= M such that (M'-1) is a multiple of BLOCK_SIZE
#define COMPATIBLE_M(M) _COMPATIBLE_M(M, BLOCK_SIZE)
// Same, but use custom block size in place of the global one
#define _COMPATIBLE_M(M, BSIZE) ((M-1) + BSIZE - ((M-2) % BSIZE))

// Compute safe double buffer length for required history of length M by finding M' (see above)
#define DOUBLE_BUFFER_SIZE(M) _DOUBLE_BUFFER_SIZE(M, BLOCK_SIZE)
// Same, but use custom block size in place of the global one
#define _DOUBLE_BUFFER_SIZE(M, BSIZE) 2*(_COMPATIBLE_M(M, BSIZE) - 1 + BSIZE)

typedef struct
{
int16_t buff[INPUT_CHANNELS][N_HISTORY];
uint32_t curr;

}adc_sample_history;

typedef struct
{
float* buff;
uint32_t curr;
uint32_t size;

}component_buffer;

void init_sample_history(adc_sample_history* history);
void init_component_buffer(component_buffer* buffer, float* buff_param, uint32_t size_param);
void init_reverse_component_buffer(component_buffer* buffer, float* buff_param, uint32_t size_param);
void advance_double_buffer(component_buffer* buffer, float* in, uint8_t blockSize);
void advance_reverse_double_buffer(component_buffer* buffer, float* in, uint8_t blockSize);

#endif
