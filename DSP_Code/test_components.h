//Basic suite of crap DSP components no-one would ever want to use

#ifndef _TEST_COMPONENTS_H_
#define _TEST_COMPONENTS_H_

#ifndef PI
#define PI 3.1415926
#endif

#include "component.h"
#include "global.h"
#include <stdio.h>

uint16_t mixer_exec(dsp_comp* comp);
uint16_t router_exec(dsp_comp* comp);
uint16_t amplifier_exec(dsp_comp* comp);
uint16_t pass_through_exec(dsp_comp* comp);
uint16_t swap_exec(dsp_comp* comp);
uint16_t splitter_exec(dsp_comp* comp);
uint16_t mult_exec(dsp_comp* comp);
uint16_t noise_exec(dsp_comp* comp);
uint16_t diff_exec(dsp_comp* comp);
uint16_t inv_exec(dsp_comp* comp);
uint16_t delay_exec(dsp_comp* comp);
uint16_t fir_exec(dsp_comp* comp);
uint16_t recorder_exec(dsp_comp* comp);
uint16_t player_exec(dsp_comp* comp);
uint16_t osc_exec(dsp_comp* comp);
uint16_t corr_exec(dsp_comp* comp);

#endif
