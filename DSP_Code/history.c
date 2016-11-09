#include "history.h"

void init_sample_history(adc_sample_history* history)
{
	//Clear sample history, otherwise for large values of N_HISTORY, we'd get a few seconds of hiss for echo, reverb, etc.
	history->curr = 0;
	int i = 0;
	int j = 0;
	for(i = 0; i < INPUT_CHANNELS; i++)
	{
		for(j = 0; j < N_HISTORY; j++)
		{
			history->buff[i][j] = 0;
		}
	}
}

void init_component_buffer(component_buffer* buffer, float* buff_param, uint32_t size_param)
{
	// Sets the members of the given component_buffer struct.

	buffer->buff = buff_param;
	buffer->size = size_param;
	buffer->curr = 0;
	memset(buffer->buff, 0, size_param * sizeof(float));
}

void init_reverse_component_buffer(component_buffer* buffer, float* buff_param, uint32_t size_param)
{
	// Like init_component_buffer, but sets the start index to the last element (for components that make use of time-reversed histories).

	buffer->buff = buff_param;
	buffer->size = size_param;
	buffer->curr = size_param-1;
	memset(buffer->buff, 0, size_param * sizeof(float));
}

void advance_double_buffer(component_buffer* buffer, float* in, uint8_t blockSize)
{
	// Stores an incoming block of samples in the buffer and advances the read/write index sensibly (see below).

	if(buffer->curr >= buffer->size/2) buffer->curr -= buffer->size/2;

	memcpy(buffer->buff + buffer->curr, in, blockSize * sizeof(float));
	memcpy(buffer->buff + buffer->curr + buffer->size/2, in, blockSize * sizeof(float));

	buffer->curr += blockSize; // buffer->curr now gives the beginning of the contiguous history belonging to the oldest block element
}

void advance_reverse_double_buffer(component_buffer* buffer, float* in, uint8_t blockSize)
{
    // Stores an incoming block of samples in the buffer and advances the read/write index sensibly (see below).

	if(buffer->curr < buffer->size/2) buffer->curr += buffer->size/2;

	uint8_t i;
	for(i = 0; i < blockSize; i++)
	{
		buffer->buff[buffer->curr - buffer->size/2 - i] = buffer->buff[buffer->curr - i] = in[i];
	}

	buffer->curr -= blockSize; // buffer->curr now gives the end of the reverse contiguous history belonging to the oldest block element
}
