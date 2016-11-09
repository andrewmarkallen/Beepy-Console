#include "test_components.h"

uint16_t mixer_exec(dsp_comp* comp)
{
	arm_add_f32(comp->in[0], comp->in[1], comp->pre_out[0], BLOCK_SIZE);

	finalise(comp);
	return 0;
}

uint16_t router_exec(dsp_comp* comp)
{
	uint8_t in, out;
	
	for(out = 0; out < comp->n_out; out++)
	{
		arm_fill_f32(0.0f, comp->pre_out[out], BLOCK_SIZE);

		for(in = 0; in < comp->n_in; in++)
		{
			if((comp->int_params[in] >> out) & 0x1)
			{
				arm_add_f32(comp->in[in], comp->pre_out[out], comp->pre_out[out], BLOCK_SIZE);
			}
		}
	}

	finalise(comp);
	return 0;
}

uint16_t amplifier_exec(dsp_comp* comp)
{
	arm_scale_f32(comp->in[0], comp->float_params[0], comp->pre_out[0], BLOCK_SIZE);

	finalise(comp);
	return 0;
}

uint16_t pass_through_exec(dsp_comp* comp)
{
	arm_copy_f32(comp->in[0], comp->pre_out[0], BLOCK_SIZE);

	finalise(comp);
	return 0;
}

uint16_t swap_exec(dsp_comp* comp)
{
	arm_copy_f32(comp->in[0], comp->pre_out[1], BLOCK_SIZE);
	arm_copy_f32(comp->in[1], comp->pre_out[0], BLOCK_SIZE);

	finalise(comp);
	return 0;
}

uint16_t splitter_exec(dsp_comp* comp)
{
	uint8_t j;
	for(j = 0; j < comp->n_out; j++)
	{
		arm_copy_f32(comp->in[0], comp->pre_out[j], BLOCK_SIZE);
	}

	finalise(comp);
	return 0;
}

uint16_t mult_exec(dsp_comp* comp)
{
	arm_mult_f32(comp->in[0], comp->in[1], comp->pre_out[0], BLOCK_SIZE);

	finalise(comp);
	return 0;
}

uint16_t noise_exec(dsp_comp* comp)
{
  //printf("exec called for %s\n", comp->name);
  
  //This implements a 16-bit linear feedback shift register in Galois configuration 
  //Each step returns the next element in a (2^16)-1 length unsigned integer sequence (all values except 0).
  
  //It works by shifting the current state (16 bit unsigned int) right by one, saving the LSB we shift off the end
  //Then certain bits (taps) of the shifted state are XOR'd with the stored LSB.
  //The taps are determined by algebraic concerns so as to ensure the entire field of numbers is computed (i.e. google it)
   
  int16_t curr_state = comp->int_params[0];  
   
  int i = 0;
  for(i = 0; i < BLOCK_SIZE; i++)
  {
    uint16_t lsb = curr_state & 1;  //Extract the least significant bit
    curr_state >>= 1;               //Shift curr state by 1
    curr_state ^= (-lsb) & 0xB400u; //bitwise XOR the curr_state with the bitmask corresponding to taps 
    
    int32_t int_output = 0;
    int_output = curr_state;
    
    float two_to_sixteen = 0x10000;
    float half_of_two_to_sixteen = 0x8000;
    
    comp->pre_out[0][i] = (int_output - half_of_two_to_sixteen) / two_to_sixteen;
  }  
  
  comp->int_params[0] = curr_state;
  finalise(comp);
  return 0; 
}

uint16_t diff_exec(dsp_comp* comp)
{
	arm_sub_f32(comp->in[0], comp->in[1], comp->pre_out[0], BLOCK_SIZE);

	finalise(comp);
	return 0;
}

uint16_t inv_exec(dsp_comp* comp)
{
	uint8_t j;
	for(j = 0; j < comp->n_out; j++)
	{
		arm_negate_f32(comp->in[j], comp->pre_out[j], BLOCK_SIZE);
	}

	finalise(comp);
	return 0;
}

uint16_t delay_exec(dsp_comp* comp)
{
	// NOTE: Buffer needs to be of size 2*(M-1+BLOCK_SIZE), with (M-1) a multiple of BLOCK_SIZE
	// Can use the macro DOUBLE_BUFFER_SIZE(M) (where M-1 is the maximum allowed delay)

	component_buffer* buffer = &comp->buffer[0];

	// Store incoming signal block in double buffer history
	advance_double_buffer(buffer, comp->in[0], BLOCK_SIZE);

	// The maximum allowed delay
	uint32_t M = buffer->size/2 + 1 - BLOCK_SIZE;

	// Get the start index of the delayed block
	uint32_t idx = buffer->curr + M - 1 - comp->int_params[0];

	// Output delayed samples
	arm_copy_f32(buffer->buff + idx, comp->pre_out[0], BLOCK_SIZE);

	finalise(comp);
	return 0;
}

uint16_t fir_exec(dsp_comp* comp)
{
	// NOTE: filter taps referenced in arm_fir_instance_f32 are interpreted as time-ordered (ascending)!

	if(comp->int_params[0])
	{
		arm_fir_instance_f32 *S = (arm_fir_instance_f32 *)comp->pInstance;
		arm_fir_f32(S, comp->in[0], comp->pre_out[0], BLOCK_SIZE);
	}
	else
	{
		arm_fill_f32(0, comp->pre_out[0], BLOCK_SIZE);
	}

	finalise(comp);
	return 0;
}

uint16_t recorder_exec(dsp_comp* comp)
{
	// NOTE: Buffer sizes should be a multiple of BLOCK_SIZE

	int32_t* activeFlags = &comp->int_params[0]; // Bit field showing which channels are currently recording

	uint8_t j;
	for(j = 0; j < comp->n_in; j++)
	{
		if(*activeFlags >> j & 0x1)
		{
			component_buffer* buffer = &comp->buffer[j];

			if(buffer->curr + BLOCK_SIZE <= buffer->size)
			{
				arm_copy_f32(comp->in[j], buffer->buff + buffer->curr, BLOCK_SIZE);
				buffer->curr += BLOCK_SIZE;
			}
			else
			{
				// If buffer is full, stop recording this channel
				*activeFlags &= ~(0x1 << j);
				buffer->curr = 0;
			}
		}
	}

	finalise(comp);
	return 0;
}

uint16_t player_exec(dsp_comp* comp)
{
	int32_t* activeFlags = &comp->int_params[0]; // Bit field showing which channels are currently playing
	int32_t loop = comp->int_params[1]; // Whether to loop playback or not

	uint8_t j;
	for(j = 0; j < comp->n_out; j++)
	{
		if(*activeFlags >> j & 0x1)
		{
			component_buffer* buffer = &comp->buffer[j];

			if(buffer->curr + BLOCK_SIZE <= buffer->size || loop)
			{
				buffer->curr %= buffer->size;
				arm_copy_f32(buffer->buff + buffer->curr, comp->pre_out[j], BLOCK_SIZE);
				buffer->curr += BLOCK_SIZE;
			}
			else
			{
				*activeFlags &= ~(0x1 << j);
				buffer->curr = 0;
			}
		}
		else
		{
			arm_fill_f32(0.0f, comp->pre_out[j], BLOCK_SIZE);
		}
	}

	finalise(comp);
	return 0;
}

uint16_t osc_exec(dsp_comp* comp)
{
	int32_t period = comp->int_params[0];
	int32_t *phase = &comp->int_params[1];
  
	uint8_t i;
	for(i = 0; i < BLOCK_SIZE; i++)
	{
		comp->pre_out[0][i] = 0.5f * sinf(2 * PI * (float)(*phase) / (float)period);
		*phase = (*phase + 1) % period;
	}

	finalise(comp);
	return 0;
}

uint16_t corr_exec(dsp_comp* comp)
{
	// NOTE: (M-1) needs to be a multiple of BLOCK_SIZE
	// Can use macro DOUBLE_BUFFER_SIZE(M) when allocating buffer[0]

	// comp->buffer[0] : left double history buffer, size: 2*(M-1+BLOCK_SIZE)
	// comp->buffer[1] : correlation output vector, size: M
	// comp->buffer[2] : reserved buffer, size: M

	// comp->int_params[0] : update flag
	// comp->int_params[1] : reset flag
	// comp->int_params[2] : stop after this many samples (if nonzero)
	// comp->int_params[3] : current number of iterations (reserved)

	if(comp->int_params[0])
	{
		component_buffer* leftHist = &comp->buffer[0];
		component_buffer* corrVec = &comp->buffer[1];
		component_buffer* temp = &comp->buffer[2];

		uint32_t M = corrVec->size;

		if(comp->int_params[1])
		{
			// Reset left input buffer and correlation vector
			arm_fill_f32(0.0f, leftHist->buff, leftHist->size);
			arm_fill_f32(0.0f, corrVec->buff, corrVec->size);
			leftHist->curr = leftHist->size - 1;
			comp->int_params[1] = 0;
			comp->int_params[3] = 0;
		}

		// Store left signal block in the reversed double history buffer
		advance_reverse_double_buffer(leftHist, comp->in[0], BLOCK_SIZE);

		// Iterate through right signal block
		uint8_t i;
		for(i = 0; i < BLOCK_SIZE; i++)
		{
			// Correlate new right sample with left history
			float in = comp->in[1][i];
			if(comp->int_params[2]) in /= comp->int_params[2];
			arm_scale_f32(leftHist->buff + leftHist->curr - M + 1 - i, in, temp->buff, M);
			arm_add_f32(corrVec->buff, temp->buff, corrVec->buff, M);

			comp->int_params[3]++;

			// If target number of samples is reached, stop updating
			if(comp->int_params[2] && comp->int_params[3] >= comp->int_params[2])
			{
				comp->int_params[0] = 0;
				break;
			}
		}
	}

	finalise(comp);
	return 0;
}
