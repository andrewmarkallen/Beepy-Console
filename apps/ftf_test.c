#include "ftf_test.h"


ftf_test_state* setupFTFTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing)
{
	// Dynamically allocate memory for the struct that holds references to components and their buffers
	ftf_test_setup* setup = alloc_or_fail(1, sizeof(*setup), "ftf_test_setup");


	// Dynamically allocate memory for each component and (where applicable) their buffers
	setup->noise = alloc_generic();
	setup->router = alloc_generic();
	setup->delay = alloc_delay(FT__DELAY_SIZE);
	setup->ftf = alloc_ftf(FT__FTF_SIZE);


	// Convenience
	dsp_comp* noise = setup->noise;
	dsp_comp* router = setup->router;
	dsp_comp* delay = setup->delay;
	dsp_comp* ftf = setup->ftf;


	// Register all components with the controller
	setup_and_reg_comp(control, noise, 0, 1, ANC_Noise_Exec, "noise");
	setup_and_reg_comp(control, router, 1, 2, router_exec, "router");
	setup_and_reg_comp(control, delay, 1, 1, delay_exec, "delay");
	setup_and_reg_comp(control, ftf, 2, 1, msmftf_exec, "ftf");


	// Make connections
	attach_ADC(control, 0, &control->null_component, 0);
	attach_ADC(control, 1, &control->null_component, 1);
	set_target(noise, 0, router, 0);
	set_target(router, 0, ftf, 0);
	set_target(router, 1, delay, 0);
	set_target(delay, 0, ftf, 1);
	set_target(&control->null_component, 0, &control->dac_out, 0);
	set_target(&control->null_component, 1, &control->dac_out, 1);


	// Set component parameters
	noise->float_params[0] = -1.0f; // Min value
	noise->float_params[1] = 1.0f; // Max value

	router->int_params[0] = 0x1 | 0x2; // Route noise to ftf and delay

	delay->int_params[0] = FT__DELAY_VAL; // Delay value

	ftf->int_params[0] = 1; // Update flag
	ftf->int_params[1] = 1; // Reset flag
	ftf->int_params[2] = 0; // Apply flag
	ftf->int_params[3] = 0; // err_in flag
	ftf->int_params[4] = 0; // err_out flag

	ftf->float_params[0] = 0.997; // lambda (forgetting factor)
	ftf->float_params[1] = 0.5;    // mu (init value)
	ftf->float_params[2] = 0.995; // v1 (leakage factor)
	ftf->float_params[3] = 1e-07; // c1 (regularisation constant)


	// Evaluate DSP graph
	build_evaluation_order(control);


	// Set event handlers
	*onButtonDown = (eventHandler)ft__sendAndReset;


	// Initialise state structure
	ftf_test_state *state = alloc_or_fail(1, sizeof(*state), "ftf_test_state");
	state->setup = setup;


	return state;
}

void destroyFTFTest(ftf_test_state *state)
{
	free_generic(state->setup->noise);
	free_generic(state->setup->router);
	free_delay(state->setup->delay);
	free_ftf(state->setup->ftf);
	free(state->setup);
	free(state);
}

void ft__sendAndReset(ftf_test_state *state)
{
	dsp_comp* ftf = state->setup->ftf;
	sendWaveform(ftf->buffer[1].buff, ftf->buffer[1].size);
	ftf->int_params[0] = 1;
	ftf->int_params[1] = 1;
}
