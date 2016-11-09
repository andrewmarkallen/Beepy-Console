#include "oscillator.h"

void init_wavetable(float* wav, uint16_t size, char* name, InterpolationType interp)
{
  
}

uint64_t set_delta(oscillator* osc, q15_t frequency, uint16_t sample_rate)
{
  uint64_t steps = 0xFFFFFFFFFFFFFFFF;
  DEBUG_PRINTF("max: %d\n", steps); //TODO: This probably needs to be fixed to ACTUALLY print the 64 bit number.
  double steps_float = (double)steps;
  double frequency_float = (double)frequency;
  double sample_rate_float = (double)sample_rate;
  double delta_phase_float = (steps_float * frequency_float) / sample_rate_float;
  osc->delta_phase = (uint64_t)delta_phase_float;
  DEBUG_PRINTF("delta_phase: %d\n", osc->delta_phase);
  return osc->delta_phase;
}

q15_t step_and_compute_sample(oscillator* osc)
{
  if(osc->interpolation == NO_INTERPOLATION)
  {
    uint64_t index = (osc->phase_accumulator + osc->phase_offset)  >> osc->wave->shift_amount;
    return osc->wave->waveform[index];
  }
  else
  {
    return 0;
  }
}
