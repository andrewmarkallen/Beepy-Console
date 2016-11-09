#ifndef _OSCILLATOR_H_
#define _OSCILLATOR_H_

#include "global.h"
#include "debug_settings.h"

#define MAX_WAVE_NAME_SIZE 16

typedef enum{NO_INTERPOLATION} InterpolationType;
typedef enum{ENVELOPE, OSCILLATOR} OscillatorType;

typedef struct
{
float* waveform;
uint16_t size;
uint16_t shift_amount;
char name[MAX_WAVE_NAME_SIZE];  
}wavetable;

typedef struct
{
uint64_t phase_accumulator;
uint64_t phase_offset;
uint64_t delta_phase;  //the amount by which the phase changes per step.
wavetable* wave;
q15_t multiplier; //For virtual 'CV inputs', later on expand this so it can have multiple multipliers.
InterpolationType interpolation; 
OscillatorType type;

}oscillator;

void init_wavetable(float* wav, uint16_t size, char* name, InterpolationType interp);
uint64_t set_delta(oscillator* osc, q15_t frequency, uint16_t sample_rate);
q15_t step_and_compute_sample(oscillator* osc);


#endif // _OSCILLATOR_H_