#ifndef __FTF_TEST_H_
#define __FTF_TEST_H_


#include "global.h"
#include "controller.h"
#include "component.h"
#include "ANC_noise.h"
#include "msmftf.h"
#include "test_components.h"
#include "alloc_components.h"
#include "serlcd.h"


// Parameters (adjust these)
#define FT__DELAY_SIZE 20
#define FT__DELAY_VAL 10
#define FT__FTF_SIZE FT__DELAY_SIZE


typedef struct ftf_test_setup {
	// DSP components
	dsp_comp* noise;
	dsp_comp* router;
	dsp_comp* delay;
	dsp_comp* ftf;
} ftf_test_setup;


// State variables
typedef struct ftf_test_state {
	ftf_test_setup* setup;
} ftf_test_state;


ftf_test_state* setupFTFTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing);
void destroyFTFTest(ftf_test_state *state);
void ft__sendAndReset(ftf_test_state *state);

#endif
