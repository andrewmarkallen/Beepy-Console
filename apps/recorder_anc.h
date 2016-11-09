#ifndef __RECORDER_ANC_H_
#define __RECORDER_ANC_H_

#include "global.h"
#include "controller.h"
#include "component.h"
#include "ANC_noise.h"
#include "msmftf.h"
#include "test_components.h"
#include "alloc_components.h"
#include "serlcd.h"


// Parameters (adjust these)
#define RA__REC_SIZE 6000 // Should be a multiple of BLOCK_SIZE
#define RA__FILTER_SIZE 300 // The filter length to be used for primary and secondary paths
#define RA__SECONDARY_ITERS 100 // How many times to repeat the secondary path identification record->adapt cycle
#define RA__PRIMARY_ITERS 100 // How many times to repeat the primary path identification record->adapt cycle
#define RA__SECONDARY_DEFER_CYCLES 10 // By how many cycles to defer the secondary path adaptive filter start
#define RA__PRIMARY_DEFER_CYCLES 10 // By how many cycles to defer the primary path adaptive filter start
#define RA__NUISANCE_FREQ 0 // Frequency in Hz of the nuisance signal (0 gives white noise)
#define RA__USE_LEFT_MIC 0 // Whether to measure the nuisance signal (1) or use the signal being output directly (0)
#define RA__DEBUG (0x2) // Bit flags: 1 -> log state changes, 2 -> send transfer functions


typedef struct recorder_anc_setup {
	// Inputs
	dsp_comp *noise;
	dsp_comp *osc;
	dsp_comp *player;

	dsp_comp *in_router;

	// Processing components
	dsp_comp *ftf;
	dsp_comp *fir;
	dsp_comp *inv;

	dsp_comp *out_router;

	// Outputs
	dsp_comp *recorder;
} recorder_anc_setup;


// State variables
typedef struct recorder_anc_state {
	recorder_anc_setup *setup;
	uint8_t calibrationStep;
	uint8_t defer;
	uint32_t iterations;
} recorder_anc_state;


recorder_anc_state *setupRecorderANC(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing);
void destroyRecorderANC(recorder_anc_state *state);
void ra__startCalibration(recorder_anc_state *state);
void ra__advanceIfDone(recorder_anc_state *state);


#endif
