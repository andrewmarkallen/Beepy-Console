#ifndef __FTF_REC_TEST_H_
#define __FTF_REC_TEST_H_


#include "global.h"
#include "controller.h"
#include "component.h"
#include "ANC_noise.h"
#include "msmftf.h"
#include "test_components.h"
#include "alloc_components.h"
#include "serlcd.h"


// Parameters (adjust these)
#define FRT__REC_SIZE 6000
#define FRT__FTF_SIZE 200


typedef struct ftf_rec_test_setup {
	// DSP components
	dsp_comp* noise;
	dsp_comp* router;
	dsp_comp* recorder;
	dsp_comp* player;
	dsp_comp* ftf;
} ftf_rec_test_setup;


// State variables
typedef struct ftf_rec_test_state {
	ftf_rec_test_setup* setup;
	uint8_t step;
	uint32_t defer;
} ftf_rec_test_state;


ftf_rec_test_state* setupFTFRecorderTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing);
void destroyFTFRecorderTest(ftf_rec_test_state *state);
void frt__advanceIfDone(ftf_rec_test_state *state);
void frt__sendAndReset(ftf_rec_test_state *state);

#endif
