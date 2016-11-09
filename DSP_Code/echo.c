#include "echo.h"


//Param 0 is the feedback amount, Param 1 is the delay in samples
uint16_t stereo_echo_exec(dsp_comp* comp)
{
  // uint32_t i = (comp->buffer->curr + comp->buffer->size - comp->int_params[1]) % comp->buffer->size;
  // component_buffer* buffer = comp->buffer;
  //
  // component_buffer* lbuffer = &buffer[0];
  // component_buffer* rbuffer = &buffer[1];
  //
  // int16_t oldL = lbuffer->buff[i];
  // int16_t oldR = rbuffer->buff[i];
  //
  // int16_t newLunscaled = comp->in[0] + oldR;
  // //int16_t newRunscaled = comp->in[1] + oldL;
  // int16_t newRunscaled = oldL; //This is a cheat for demo!
  //
  // int32_t newLscaled = ((newLunscaled * comp->int_params[0]) / 1024);
  // int32_t newRscaled = ((newRunscaled * comp->int_params[0]) / 1024);
  //
  //
  // comp->pre_out[0] = newLunscaled;
  // comp->pre_out[1] = newRunscaled;
  //
  // lbuffer->buff[lbuffer->curr] = newLscaled;
  // rbuffer->buff[rbuffer->curr] = newRscaled;
  //
  // lbuffer->curr++;
  // rbuffer->curr++;
  // if(lbuffer->curr > lbuffer->size)
  // {
  //   lbuffer->curr = 0;
  //   rbuffer->curr = 0;//They're both updated at the same time always
  // }
	finalise(comp);
	return 0;
}