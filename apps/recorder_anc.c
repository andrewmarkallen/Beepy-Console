#include "recorder_anc.h"


recorder_anc_state *setupRecorderANC(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing)
{
	// Allocate setup struct and all components

	recorder_anc_setup *setup = alloc_or_fail(1, sizeof(*setup), "recorder_anc_setup");

	setup->recorder = alloc_recorder(2, RA__REC_SIZE);
	float *playback_buffers[] = {setup->recorder->buffer[0].buff, setup->recorder->buffer[1].buff};
	setup->player = alloc_player(2, RA__REC_SIZE, playback_buffers);
	setup->noise = alloc_generic();
	setup->osc = alloc_generic();
	setup->in_router = alloc_generic();
	setup->ftf = alloc_ftf(RA__FILTER_SIZE);
	setup->fir = alloc_fir(setup->ftf->buffer[1].size, NULL); // fir needs a separate buffer from the ftf one, partially because they are ordered differently
	setup->inv = alloc_generic();
	setup->out_router = alloc_generic();


	// Setup all components

	setup_and_reg_comp(control, setup->noise, 0, 1, ANC_Noise_Exec, "noise");
	setup_and_reg_comp(control, setup->osc, 0, 1, RA__NUISANCE_FREQ ? osc_exec : ANC_Noise_Exec, "osc");
	setup_and_reg_comp(control, setup->player, 0, 2, player_exec, "player");
	setup_and_reg_comp(control, setup->in_router, 6, 6, router_exec, "in_router");
	setup_and_reg_comp(control, setup->ftf, 2, 0, msmftf_exec, "ftf");
	setup_and_reg_comp(control, setup->fir, 1, 1, fir_exec, "fir");
	setup_and_reg_comp(control, setup->inv, 1, 1, inv_exec, "inv");
	setup_and_reg_comp(control, setup->out_router, 4, 4, router_exec, "out_router");
	setup_and_reg_comp(control, setup->recorder, 2, 0, recorder_exec, "recorder");


	// Make connections

	set_target(setup->noise, 0, setup->in_router, 0);
	set_target(setup->osc, 0, setup->in_router, 1);
	attach_ADC(control, 0, setup->in_router, 2);
	attach_ADC(control, 1, setup->in_router, 3);
	set_target(setup->player, 0, setup->in_router, 4);
	set_target(setup->player, 1, setup->in_router, 5);

	set_target(setup->in_router, 0, setup->out_router, 0);
	set_target(setup->in_router, 1, setup->out_router, 1);
	set_target(setup->in_router, 2, setup->ftf, 0);
	set_target(setup->in_router, 3, setup->ftf, 1);
	set_target(setup->in_router, 4, setup->fir, 0);
	set_target(setup->in_router, 5, setup->inv, 0);

	set_target(setup->fir, 0, setup->out_router, 2);
	set_target(setup->inv, 0, setup->out_router, 3);

	set_target(setup->out_router, 0, &control->dac_out, 0);
	set_target(setup->out_router, 1, &control->dac_out, 1);
	set_target(setup->out_router, 2, setup->recorder, 0);
	set_target(setup->out_router, 3, setup->recorder, 1);


	// Set initial parameters

	setup->noise->float_params[0] = -1.0f; // Min value
	setup->noise->float_params[1] = 1.0f; // Max value

#if RA__NUISANCE_FREQ // Using macro rather than C if block to avoid compiler warning about division by zero
	setup->osc->int_params[0] = 48000/RA__NUISANCE_FREQ; // Period
	setup->osc->int_params[1] = 0; // Phase
#else
	setup->osc->float_params[0] = -0.1f; // Min value (don't want to overdrive)
	setup->osc->float_params[1] = 0.1f; // Max value (don't want to overdrive)
#endif

	setup->player->int_params[0] = 0x0; // Don't play anything
	setup->player->int_params[1] = 0; // Don't loop

	setup->in_router->int_params[0] = 0x0; // Don't route anything
	setup->in_router->int_params[1] = 0x0;
	setup->in_router->int_params[2] = 0x0;
	setup->in_router->int_params[3] = 0x0;
	setup->in_router->int_params[4] = 0x0;
	setup->in_router->int_params[5] = 0x0;

	setup->ftf->int_params[0] = 0; // Update flag
	setup->ftf->int_params[1] = 0; // Reset flag
	setup->ftf->int_params[2] = 0; // Apply flag
	setup->ftf->int_params[3] = 0; // Error in flag
	setup->ftf->int_params[4] = 0; // Error out flag

	setup->ftf->float_params[0] = 0.997; // Forgetting factor
	setup->ftf->float_params[1] = 0.5; // Initialization parameter
	setup->ftf->float_params[2] = 0.995; // Leakage factor
	setup->ftf->float_params[3] = 1e-07; // Regularisation constant

	setup->fir->int_params[0] = 0; // Enable flag

	setup->out_router->int_params[0] = 0x0; // Don't route anything
	setup->out_router->int_params[1] = 0x0;
	setup->out_router->int_params[2] = 0x0;
	setup->out_router->int_params[3] = 0x0;

	setup->recorder->int_params[0] = 0x0; // Don't record anything


	// Evaluate DSP graph

	build_evaluation_order(control);


	// Set event handlers

	*onButtonDown = (eventHandler)ra__startCalibration;
	*afterProcessing = (eventHandler)ra__advanceIfDone;


	// Allocate state struct

	recorder_anc_state *state = alloc_or_fail(1, sizeof(*state), "recorder_anc_state");
	state->setup = setup;
	state->calibrationStep = 0;
	state->defer = 0;
	state->iterations = 0;


	// Return state struct

	return state;
}


void destroyRecorderANC(recorder_anc_state *state)
{
	free_generic(state->setup->noise);
	free_generic(state->setup->osc);
	free_player(state->setup->player);
	free_generic(state->setup->in_router);
	free_ftf(state->setup->ftf);
	free_fir(state->setup->fir);
	free_generic(state->setup->inv);
	free_generic(state->setup->out_router);
	free_recorder(state->setup->recorder);
	free(state->setup);
	free(state);
	reset_float_reservations();
}


void ra__startCalibration(recorder_anc_state *state)
{
	//dsp_comp *player = state->setup->player;
	dsp_comp *in_router = state->setup->in_router;
	//dsp_comp *ftf = state->setup->ftf;
	dsp_comp *fir = state->setup->fir;
	dsp_comp *out_router = state->setup->out_router;
	dsp_comp *recorder = state->setup->recorder;

	if(state->calibrationStep == 0)
	{
		// Play noise on right speaker and start recording the secondary path

		in_router->int_params[0] = 0x1; // Route noise towards dac(r) and recorder(l)
		in_router->int_params[1] = 0x0; // Silence osc
		in_router->int_params[2] = 0x0; // Silence adc(l)
		in_router->int_params[3] = 0x2; // Route adc(r) towards recorder(r)

		fir->int_params[0] = 0; // Stop applying fir

		out_router->int_params[0] = 0x2 | 0x4; // Route noise to dac(r) and recorder(l)
		out_router->int_params[1] = 0x8; // Route adc(r) to recorder(r)
		out_router->int_params[2] = 0x0; // Silence fir

		recorder->int_params[0] = 0x1 | 0x2; // Start recording both channels

		state->calibrationStep = 1;
		state->defer = RA__SECONDARY_DEFER_CYCLES;
		state->iterations = 0;

		if(RA__DEBUG & 0x1) Log("0 -> 1\n");
	}
}

void ra__advanceIfDone(recorder_anc_state *state)
{
	if(!state->calibrationStep)
	{
		return;
	}

	dsp_comp *player = state->setup->player;
	dsp_comp *in_router = state->setup->in_router;
	dsp_comp *ftf = state->setup->ftf;
	dsp_comp *fir = state->setup->fir;
	dsp_comp *out_router = state->setup->out_router;
	dsp_comp *recorder = state->setup->recorder;

	switch(state->calibrationStep)
	{
	case 0:
		break;
	case 1:
		if(recorder->int_params[0] == 0x0)
		{
			// Done recording secondary path

			in_router->int_params[0] = 0x0; // Silence noise
			in_router->int_params[1] = 0x0; // Silence osc
			in_router->int_params[2] = 0x0; // Silence adc(l)
			in_router->int_params[3] = 0x0; // Silence adc(r)

			out_router->int_params[0] = 0x0; // Silence in_router
			out_router->int_params[1] = 0x0; // Silence in_router

			if(state->defer)
			{
				// Defer start of adaptive filter by a number of cycles to ensure the output audio buffer is zero

				state->defer--;
				break;
			}

			// Start adaptive filter on the recordings

			player->int_params[0] = 0x1 | 0x2; // Start playing both recorded channels

			in_router->int_params[4] = 0x4; // Route player(l) to ftf(l)
			in_router->int_params[5] = 0x8; // Route player(r) to ftf(r)

			ftf->int_params[0] = 1; // Start updating ftf
			if(state->iterations == 0)
			{
				ftf->int_params[1] = 1; // Reset ftf
			}

			state->calibrationStep = 2;

			if(RA__DEBUG & 0x1) Log("1 -> 2\n");
		}
		break;
	case 2:
		if(player->int_params[0] == 0x0)
		{
			// Done playing back the recordings

			ftf->int_params[0] = 0; // Stop updating ftf

			if(++state->iterations < RA__SECONDARY_ITERS)
			{
				// Resume playing noise on right speaker and recording the secondary path

				in_router->int_params[0] = 0x1; // Route noise towards dac(r) and recorder(l)
				in_router->int_params[1] = 0x0; // Silence osc
				in_router->int_params[2] = 0x0; // Silence adc(l)
				in_router->int_params[3] = 0x2; // Route adc(r) towards recorder(r)
				in_router->int_params[4] = 0x0; // Silence player(l)
				in_router->int_params[5] = 0x0; // Silence player(r)

				out_router->int_params[0] = 0x2 | 0x4; // Route noise to dac(r) and recorder(l)
				out_router->int_params[1] = 0x8; // Route adc(r) to recorder(r)

				recorder->int_params[0] = 0x1 | 0x2; // Start recording both channels

				state->calibrationStep = 1;
				state->defer = RA__SECONDARY_DEFER_CYCLES;

				if(RA__DEBUG & 0x1) Log("2 -> 1\n");
			}
			else
			{
				// Restart playback to allow time for new signal to fill the filter signal history
				// TODO: use defer for this

				player->int_params[0] = 0x1 | 0x2; // Start playing both recorded channels (silently, as a timer)

				if(RA__USE_LEFT_MIC)
				{
					in_router->int_params[0] = 0x1; // Route noise towards dac(l)
					in_router->int_params[2] = 0x10; // Route adc(l) to fir
				}
				else
				{
					in_router->int_params[0] = 0x1 | 0x10; // Route noise towards dac(l) and to fir
				}
				in_router->int_params[3] = 0x20; // Route adc(r) to inv
				in_router->int_params[4] = 0x0; // Silence player(l)
				in_router->int_params[5] = 0x0; // Silence player(r)


				// ftf and fir interpret the filter coefficients in opposite order, so we have to copy them in reverse
				arm_fir_instance_f32 *S = fir->pInstance;
				uint16_t j;
				for(j = 0; j < ftf->buffer[1].size; j++)
				{
					S->pCoeffs[ftf->buffer[1].size - j - 1] = ftf->buffer[1].buff[j];
				}

				fir->int_params[0] = 1; // Start applying fir

				out_router->int_params[0] = 0x1; // Route noise to dac(l)

				state->calibrationStep = 3;
				state->iterations = 0;

				if(RA__DEBUG & 0x1) Log("2 -> 3\n");

				if(RA__DEBUG & 0x2) sendWaveform(ftf->buffer[1].buff, ftf->buffer[1].size);
			}
		}
		break;
	case 3:
		if(player->int_params[0] == 0x0)
		{
			// Timer is up, start recording the primary path signals

			out_router->int_params[2] = 0x4; // Route fir towards recorder(l)
			out_router->int_params[3] = 0x8; // Route inv towards recorder(r)

			recorder->int_params[0] = 0x1 | 0x2; // Record both channels

			state->calibrationStep = 4;
			state->defer = RA__PRIMARY_DEFER_CYCLES;

			if(RA__DEBUG & 0x1) Log("3 -> 4\n");
		}
		break;
	case 4:
		if(recorder->int_params[0] == 0x0)
		{
			// Done recording the primary path

			in_router->int_params[0] = 0x0; // Silence noise
			in_router->int_params[2] = 0x0; // Silence adc(l)
			in_router->int_params[3] = 0x0; // Silence adc(r)

			fir->int_params[0] = 0; // Stop applying fir

			out_router->int_params[0] = 0x0; // Silence noise
			out_router->int_params[2] = 0x0; // Silence fir
			out_router->int_params[3] = 0x0; // Silence inv

			if(state->defer)
			{
				// Defer start of adaptive filter by a number of cycles to ensure the output audio buffer is zero

				state->defer--;
				break;
			}

			// Start adaptive filter on the recordings

			player->int_params[0] = 0x1 | 0x2; // Start playing both recorded channels

			in_router->int_params[4] = 0x4; // Route player(l) to ftf(l)
			in_router->int_params[5] = 0x8; // Route player(r) to ftf(r)

			ftf->int_params[0] = 1; // Start updating ftf
			if(state->iterations == 0)
			{
				ftf->int_params[1] = 1; // Reset ftf
			}

			state->calibrationStep = 5;

			if(RA__DEBUG & 0x1) Log("4 -> 5\n");
		}
		break;
	case 5:
		if(player->int_params[0] == 0x0)
		{
			// Done playing back the recordings

			if(!ftf->int_params[0]) ftf->int_params[1] = 1; // If ftf diverged, tell it to reset itself

			ftf->int_params[0] = 0; // Stop ftf

			if(++state->iterations < RA__PRIMARY_ITERS)
			{
				// Restart player as a timer to fill up the filter signal history

				player->int_params[0] = 0x1 | 0x2; // Start playing both recorded channels (silently, as a timer)

				if(RA__USE_LEFT_MIC)
				{
					in_router->int_params[0] = 0x1; // Route noise towards dac(l)
					in_router->int_params[2] = 0x10; // Route adc(l) to fir
				}
				else
				{
					in_router->int_params[0] = 0x1 | 0x10; // Route noise towards dac(l) and to fir
				}
				in_router->int_params[3] = 0x20; // Route adc(r) to inv
				in_router->int_params[4] = 0x0; // Silence player(l)
				in_router->int_params[5] = 0x0; // Silence player(r)

				fir->int_params[0] = 1; // Start applying fir

				out_router->int_params[0] = 0x1; // Route noise to dac(l)

				state->calibrationStep = 3;

				if(RA__DEBUG & 0x1) Log("5 -> 3\n");
			}
			else
			{
				// Go to cancellation mode

				if(RA__USE_LEFT_MIC)
				{
					in_router->int_params[1] = 0x1; // Route osc towards dac(l)
					in_router->int_params[2] = 0x10; // Route adc(l) to ftf(l)
				}
				else
				{
					in_router->int_params[1] = 0x1 | 0x10; // Route osc towards dac(l) and to ftf(l)
				}

				in_router->int_params[4] = 0x0; // Silence player(l)
				in_router->int_params[5] = 0x0; // Silence player(r)

				// ftf and fir interpret the filter coefficients in opposite order, so we have to copy them in reverse
				arm_fir_instance_f32 *S = fir->pInstance;
				uint16_t j;
				for(j = 0; j < ftf->buffer[1].size; j++)
				{
					S->pCoeffs[ftf->buffer[1].size - j - 1] = ftf->buffer[1].buff[j];
				}

				fir->int_params[0] = 1; // Start applying fir

				out_router->int_params[0] = 0x1; // Route osc to dac(l)
				out_router->int_params[2] = 0x2; // Route fir to dac(r)

				state->calibrationStep = 0;

				if(RA__DEBUG & 0x1) Log("5 -> 0\n");

				if(RA__DEBUG & 0x2) sendWaveform(ftf->buffer[1].buff, ftf->buffer[1].size);
			}
		}
		break;
	}
}
