#ifndef __CORR_TEST_H_
#define __CORR_TEST_H_


#include "global.h"
#include "controller.h"
#include "component.h"
#include "ANC_noise.h"
#include "test_components.h"
#include "alloc_components.h"
#include "serlcd.h"


// Parameters (adjust these)
#define CT__DELAY_SIZE 128
#define CT__DELAY_VAL 80
#define CT__CORR_SIZE CT__DELAY_SIZE
#define CT__CORR_ITERS 48000*5


typedef struct corr_test_setup {
	// DSP components
	dsp_comp* noise;
	dsp_comp* router;
	dsp_comp* delay;
	dsp_comp* corr;
} corr_test_setup;


// State variables
typedef struct corr_test_state {
	corr_test_setup* setup;
	char corrBusy;
} corr_test_state;


corr_test_state* setupCorrTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing);
void destroyCorrTest(corr_test_state *state);
void ct__saveCorrBusyState(corr_test_state *state);
void ct__maybeSendCorrelation(corr_test_state *state);

#endif
