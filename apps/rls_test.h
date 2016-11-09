#ifndef __RLS_TEST_H_
#define __RLS_TEST_H_


#include "global.h"
#include "controller.h"
#include "component.h"
#include "ANC_noise.h"
#include "rls.h"
#include "test_components.h"
#include "alloc_components.h"
#include "serlcd.h"


// Parameters (adjust these)
#define RT__DELAY_SIZE 20
#define RT__DELAY_VAL 10
#define RT__RLS_SIZE RT__DELAY_SIZE


typedef struct rls_test_setup {
	// DSP components
	dsp_comp* noise;
	dsp_comp* router;
	dsp_comp* delay;
	dsp_comp* rls;
	// Component buffers
	component_buffer* delay_buffer;
} rls_test_setup;


// State variables
typedef struct rls_test_state {
	rls_test_setup* setup;
} rls_test_state;


rls_test_state* setupRLSTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing);
void destroyRLSTest(rls_test_state *state);
void rt__sendAndReset(rls_test_state *state);

#endif
