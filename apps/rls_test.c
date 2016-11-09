#include "rls_test.h"


rls_test_state* setupRLSTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing)
{
	// Dynamically allocate memory for the struct that holds references to components and their buffers
	rls_test_setup* setup = (rls_test_setup *)alloc_or_fail(1, sizeof(rls_test_setup), "rls_test_setup");


	// Dynamically allocate memory for each component and (where applicable) their buffers
	setup->noise = alloc_generic();
	setup->router = alloc_generic();
	setup->delay = alloc_delay(RT__DELAY_SIZE);
	setup->rls = alloc_rls(RT__RLS_SIZE);


	// Convenience
	dsp_comp* noise = setup->noise;
	dsp_comp* router = setup->router;
	dsp_comp* delay = setup->delay;
	dsp_comp* rls = setup->rls;


	// Register all components with the controller
	setup_and_reg_comp(control, noise, 0, 1, ANC_Noise_Exec, "noise");
	setup_and_reg_comp(control, router, 1, 2, router_exec, "router");
	setup_and_reg_comp(control, delay, 1, 1, delay_exec, "delay");
	setup_and_reg_comp(control, rls, 2, 1, rls_exec, "rls");


	// Make connections
	attach_ADC(control, 0, &control->null_component, 0);
	attach_ADC(control, 1, &control->null_component, 1);
	set_target(noise, 0, router, 0);
	set_target(router, 0, rls, 0);
	set_target(router, 1, delay, 0);
	set_target(delay, 0, rls, 1);
	set_target(&control->null_component, 0, &control->dac_out, 0);
	set_target(&control->null_component, 1, &control->dac_out, 1);


	// Set component parameters
	noise->float_params[0] = -1.0f; // Min value
	noise->float_params[1] = 1.0f; // Max value

	router->int_params[0] = 0x1 | 0x2; // Route noise to rls and delay

	delay->int_params[0] = RT__DELAY_VAL; // Delay value

	rls->int_params[0] = 1; // Update flag
	rls->int_params[1] = 1; // Reset flag
	rls->int_params[2] = 0; // Apply flag
	rls->int_params[3] = 0; // err_in flag
	rls->int_params[4] = 0; // err_out flag

	rls->float_params[0] = 0.997; // lambda (forgetting factor)
	rls->float_params[1] = 0.5;    // mu (init value)


	// Evaluate DSP graph
	build_evaluation_order(control);


	// Set event handlers
	*onButtonDown = (eventHandler)rt__sendAndReset;


	// Initialise state structure
	rls_test_state *state = (rls_test_state *)alloc_or_fail(1, sizeof(rls_test_state), "rls_test_state");
	state->setup = setup;


	return state;
}

void destroyRLSTest(rls_test_state *state)
{
	free_generic(state->setup->noise);
	free_generic(state->setup->router);
	free_delay(state->setup->delay);
	free_rls(state->setup->rls);
	free(state->setup);
	free(state);
}

void rt__sendAndReset(rls_test_state *state)
{
	dsp_comp* rls = state->setup->rls;
	sendWaveform(rls->buffer[1].buff, rls->buffer[1].size);
	rls->int_params[0] = 1;
	rls->int_params[1] = 1;
}
