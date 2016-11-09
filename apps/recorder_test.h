#ifndef __RECORDER_TEST_H_
#define __RECORDER_TEST_H_

#include "global.h"
#include "controller.h"
#include "component.h"
#include "ANC_noise.h"
#include "test_components.h"
#include "alloc_components.h"
#include "serlcd.h"


// Parameters (adjust these)
#define RT__REC_SIZE 3000
#define RT__LOOP 1


typedef struct recorder_test_setup {
	// DSP components
	dsp_comp *recorder;
	dsp_comp *player;
} recorder_test_setup;


// State variables
typedef struct recorder_test_state {
	recorder_test_setup *setup;
} recorder_test_state;


recorder_test_state *setupRecorderTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing);
void destroyRecorderTest(recorder_test_state *state);
void rt__record(recorder_test_state *state);
void rt__play(recorder_test_state *state);


#endif
