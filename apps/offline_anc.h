#ifndef __OFFLINE_ANC_H_
#define __OFFLINE_ANC_H_


#include "global.h"
#include "controller.h"
#include "component.h"
#include "ANC_noise.h"
#include "test_components.h"
#include "alloc_components.h"
#include "msmftf.h"
#include "serlcd.h"
#include <arm_math.h>


// Parameters (adjust these)
#define OA__CORR_SIZE 128
#define OA__CORR_ITERS 40000
#define OA__DELAY_SIZE OA__CORR_SIZE
#define OA__FTF1_SIZE 20
#define OA__FTF2_SIZE 20
#define OA__PREDELAY_OFFSET 3


// DSP components
typedef struct offline_anc_setup {
	dsp_comp* noise;
	dsp_comp* osc;
	dsp_comp* in_router;
	dsp_comp* out_router;
	dsp_comp* fir;
	dsp_comp* inv;
	dsp_comp* corr;
	dsp_comp* delay;
	dsp_comp* ftf1;
	dsp_comp* ftf2;
} offline_anc_setup;


// State variables
typedef struct offline_anc_state {
	offline_anc_setup* setup;
	char calibrationStep;
	char corrBusy;
} offline_anc_state;


offline_anc_state* setupOfflineANC(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing);
void destroyOfflineANC(offline_anc_state *state);
void oa__advanceCalibration(offline_anc_state *state);
void oa__saveCorrBusyState(offline_anc_state *state);
void oa__maybeFinishCorrelation(offline_anc_state *state);


#endif
