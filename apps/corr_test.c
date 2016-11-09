#include "corr_test.h"


corr_test_state* setupCorrTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing)
{
	// Dynamically allocate memory for the struct that holds references to components and their buffers
	corr_test_setup* setup = (corr_test_setup *)alloc_or_fail(1, sizeof(corr_test_setup), "corr_test_setup");


	// Dynamically allocate memory for each component and (where applicable) their buffers
	setup->noise = alloc_generic();
	setup->router = alloc_generic();
	setup->delay = alloc_delay(CT__DELAY_SIZE);
	setup->corr = alloc_corr(CT__CORR_SIZE);


	// Convenience
	dsp_comp* noise = setup->noise;
	dsp_comp* router = setup->router;
	dsp_comp* delay = setup->delay;
	dsp_comp* corr = setup->corr;


	// Register all components with the controller
	setup_and_reg_comp(control, noise, 0, 1, ANC_Noise_Exec, "noise");
	setup_and_reg_comp(control, router, 1, 2, router_exec, "router");
	setup_and_reg_comp(control, delay, 1, 1, delay_exec, "delay");
	setup_and_reg_comp(control, corr, 2, 0, corr_exec, "corr");


	// Make connections
	attach_ADC(control, 0, &control->null_component, 0);
	attach_ADC(control, 1, &control->null_component, 1);
	set_target(noise, 0, router, 0);
	set_target(router, 0, corr, 0);
	set_target(router, 1, delay, 0);
	set_target(delay, 0, corr, 1);
	set_target(&control->null_component, 0, &control->dac_out, 0);
	set_target(&control->null_component, 1, &control->dac_out, 1);


	// Set component parameters
	noise->float_params[0] = -1.0f; // Min value
	noise->float_params[1] = 1.0f; // Max value

	router->int_params[0] = 0x1 | 0x2; // Route noise to corr and delay

	delay->int_params[0] = CT__DELAY_VAL; // Delay value

	corr->int_params[0] = 1; // Update flag
	corr->int_params[1] = 1; // Reset flag
	corr->int_params[2] = CT__CORR_ITERS; // Finish after this many samples


	// Evaluate DSP graph
	build_evaluation_order(control);


	// Set event handlers
	*beforeProcessing = (eventHandler)ct__saveCorrBusyState;
	*afterProcessing = (eventHandler)ct__maybeSendCorrelation;


	// Initialise state structure
	corr_test_state *state = (corr_test_state *)alloc_or_fail(1, sizeof(corr_test_state), "corr_test_state");
	state->setup = setup;
	state->corrBusy = 1;


	return state;
}

void destroyCorrTest(corr_test_state *state)
{
	free_generic(state->setup->noise);
	free_generic(state->setup->router);
	free_delay(state->setup->delay);
	free_corr(state->setup->corr);
	free(state->setup);
	free(state);
}

void ct__saveCorrBusyState(corr_test_state *state)
{
	state->corrBusy = state->setup->corr->int_params[0];
}

void ct__maybeSendCorrelation(corr_test_state *state)
{
	dsp_comp* corr = state->setup->corr;
	if(state->corrBusy && !corr->int_params[0])
	{
		sendWaveform(corr->buffer[1].buff, corr->buffer[1].size);
	}
}
