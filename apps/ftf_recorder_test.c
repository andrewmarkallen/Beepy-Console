#include "ftf_recorder_test.h"


ftf_rec_test_state* setupFTFRecorderTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing)
{
	// Dynamically allocate memory for the struct that holds references to components and their buffers

	ftf_rec_test_setup* setup = alloc_or_fail(1, sizeof(*setup), "ftf_rec_test_setup");


	// Allocate memory for each component and reserve (where applicable) their buffers

	setup->noise = alloc_generic();
	setup->router = alloc_generic();
	setup->recorder = alloc_recorder(2, FRT__REC_SIZE);
	float *playback_buffers[] = {setup->recorder->buffer[0].buff, setup->recorder->buffer[1].buff};
	setup->player = alloc_player(2, FRT__REC_SIZE, playback_buffers);
	setup->ftf = alloc_ftf(FRT__FTF_SIZE);


	// Convenience

	dsp_comp* noise = setup->noise;
	dsp_comp* router = setup->router;
	dsp_comp* recorder = setup->recorder;
	dsp_comp* player = setup->player;
	dsp_comp* ftf = setup->ftf;


	// Register all components with the controller

	setup_and_reg_comp(control, noise, 0, 1, ANC_Noise_Exec, "noise");
	setup_and_reg_comp(control, router, 1, 2, router_exec, "router");
	setup_and_reg_comp(control, recorder, 2, 0, recorder_exec, "recorder");
	setup_and_reg_comp(control, player, 0, 2, player_exec, "player");
	setup_and_reg_comp(control, ftf, 2, 1, msmftf_exec, "ftf");


	// Make connections

	attach_ADC(control, 0, &control->null_component, 0);
	attach_ADC(control, 1, recorder, 1);
	set_target(noise, 0, router, 0);
	set_target(router, 0, recorder, 0);
	set_target(router, 1, &control->dac_out, 1);
	set_target(player, 0, ftf, 0);
	set_target(player, 1, ftf, 1);
	set_target(&control->null_component, 0, &control->dac_out, 0);


	// Set component parameters

	noise->float_params[0] = -1.0f; // Min value
	noise->float_params[1] = 1.0f; // Max value

	router->int_params[0] = 0x1; // Route noise to recorder(r)

	recorder->int_params[0] = 0x0; // Don't record

	player->int_params[0] = 0x0; // Don't play back
	player->int_params[1] = 0; // Don't loop

	ftf->int_params[0] = 0; // Update flag
	ftf->int_params[1] = 1; // Reset flag
	ftf->int_params[2] = 0; // Apply flag
	ftf->int_params[3] = 0; // err_in flag
	ftf->int_params[4] = 0; // err_out flag

	ftf->float_params[0] = 0.997; // lambda (forgetting factor)
	ftf->float_params[1] = 0.5; // mu (init value)
	ftf->float_params[2] = 0.995; // v1 (leakage factor)
	ftf->float_params[3] = 1e-07; // c1 (regularisation constant)


	// Evaluate DSP graph

	build_evaluation_order(control);


	// Set event handlers

	*onButtonDown = (eventHandler)frt__sendAndReset;
	*afterProcessing = (eventHandler)frt__advanceIfDone;


	// Initialise state structure

	ftf_rec_test_state *state = alloc_or_fail(1, sizeof(*state), "ftf_rec_test_state");
	state->setup = setup;
	state->step = 0;
	state->defer = 0;


	return state;
}

void destroyFTFRecorderTest(ftf_rec_test_state *state)
{
	free_generic(state->setup->noise);
	free_generic(state->setup->router);
	free_recorder(state->setup->recorder);
	free_player(state->setup->player);
	free_ftf(state->setup->ftf);
	free(state->setup);
	free(state);
	reset_float_reservations();
}

void frt__advanceIfDone(ftf_rec_test_state *state)
{
	dsp_comp* recorder = state->setup->recorder;
	dsp_comp* player = state->setup->player;
	dsp_comp* router = state->setup->router;
	dsp_comp* ftf = state->setup->ftf;

	switch(state->step)
	{
	case 0:
		// Start playing noise and recording the mic input

		if(state->defer)
		{
			state->defer--;
			break;
		}

		router->int_params[0] = 0x1 | 0x2; // Route noise to recorder(r) and dac(r)

		recorder->int_params[0] = 0x1 | 0x2; // Record adc(r) and noise

		ftf->int_params[0] = 0; // Stop ftf

		state->step = 1;
		state->defer = 3;

//		Log("0 -> 1\n");

		break;
	case 1:
		if(recorder->int_params[0] == 0x0)
		{
			// Recording done

			router->int_params[0] = 0x1; // Route noise to recorder(r) and silence dac(r)

			if(state->defer)
			{
				state->defer--;
				break;
			}

			player->int_params[0] = 0x1 | 0x2; // Start player

			ftf->int_params[0] = 1; // Start ftf

			state->step = 2;

//			Log("1 -> 2\n");
		}
		break;
	case 2:
		if(player->int_params[0] == 0x0)
		{
			// Playback done

			ftf->int_params[0] = 0; // Stop ftf

			state->step = 0;

//			Log("2 -> 0\n");
		}
		break;
	case 100:
		router->int_params[0] = 0x1; // Silence dac(r)
		recorder->int_params[0] = 0x0; // Stop recorder
		player->int_params[0] = 0x0; // Stop player
		ftf->int_params[0] = 0; // Stop ftf

		if(state->defer)
		{
			state->defer--;
			break;
		}

		sendWaveform(ftf->buffer[1].buff, ftf->buffer[1].size); // Transmit filter taps

		ftf->int_params[1] = 1; // Reset ftf

		state->step = 0;
		state->defer = 100;

//		Log("100 -> 0\n");

		break;
	}
}

void frt__sendAndReset(ftf_rec_test_state *state)
{
	state->step = 100;
	state->defer = 10;

//	Log("? -> 100\n");
}
