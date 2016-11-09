#include "reverb.h"

uint16_t allpass_reverb(dsp_comp* comp)
{
	// uint32_t i = (comp->buffer->curr + comp->buffer->size - comp->int_params[1]) % comp->buffer->size;
//   component_buffer* buffer = comp->buffer;
//
//   component_buffer* lbuffer = &buffer[0];
//   component_buffer* rbuffer = &buffer[1];
//
//   float oldL = lbuffer->buff[i];
//   float oldR = rbuffer->buff[i];
//
//   float newLunscaled = comp->in[0] + oldR;
//   float newRunscaled = comp->in[1] + oldL;
//
//   float newLscaled = ((newLunscaled * comp->int_params[0]) / 1024);
//   float newRscaled = ((newRunscaled * comp->int_params[0]) / 1024);
//
//
//   comp->pre_out[0] = newLunscaled;
//   comp->pre_out[1] = newRunscaled;
//
//   lbuffer->buff[lbuffer->curr] = newLscaled;
//   rbuffer->buff[rbuffer->curr] = newRscaled;
//
//   lbuffer->curr++;
//   rbuffer->curr++;
//   if(lbuffer->curr > lbuffer->size)
//   {
//     lbuffer->curr = 0;
//     rbuffer->curr = 0;//They're both updated at the same time always
//   }
	finalise(comp);
	return 0;
}

