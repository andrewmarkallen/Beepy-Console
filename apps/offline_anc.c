#include "offline_anc.h"

offline_anc_state* setupOfflineANC(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing)
{
	// Dynamically allocate memory for the struct that holds references to the components and their buffers
	offline_anc_setup* setup = (offline_anc_setup *)alloc_or_fail(1, sizeof(offline_anc_setup), "offline_anc_setup");


	// Dynamically allocate memory for each component and (where applicable) their buffers
	setup->noise = alloc_generic();
	setup->osc = alloc_generic();
	setup->in_router = alloc_generic();
	setup->out_router = alloc_generic();
	setup->inv = alloc_generic();
	setup->corr = alloc_corr(OA__CORR_SIZE);
	setup->delay = alloc_delay(OA__DELAY_SIZE);
	setup->ftf1 = alloc_ftf(OA__FTF1_SIZE);
	setup->ftf2 = alloc_ftf(OA__FTF2_SIZE);
	setup->fir = alloc_fir(setup->ftf2->buffer[1].size, setup->ftf2->buffer[1].buff);


	// Convenience
	dsp_comp* noise = setup->noise;
	dsp_comp* osc = setup->osc;
	dsp_comp* in_router = setup->in_router;
	dsp_comp* out_router = setup->out_router;
	dsp_comp* inv = setup->inv;
	dsp_comp* corr = setup->corr;
	dsp_comp* delay = setup->delay;
	dsp_comp* ftf1 = setup->ftf1;
	dsp_comp* ftf2 = setup->ftf2;
	dsp_comp* fir = setup->fir;


	// Register all components with the controller
	setup_and_reg_comp(control, noise, 0, 1, ANC_Noise_Exec, "noise");
	setup_and_reg_comp(control, osc, 0, 1, osc_exec, "osc");
	setup_and_reg_comp(control, in_router, 3, 7, router_exec, "in_router");
	setup_and_reg_comp(control, corr, 2, 0, corr_exec, "corr");
	setup_and_reg_comp(control, delay, 1, 1, delay_exec, "delay");
	setup_and_reg_comp(control, ftf1, 2, 1, msmftf_exec, "ftf1");
	setup_and_reg_comp(control, ftf2, 2, 1, msmftf_exec, "ftf2");
	setup_and_reg_comp(control, fir, 1, 1, fir_exec, "fir");
	setup_and_reg_comp(control, inv, 1, 1, inv_exec, "inv");
	setup_and_reg_comp(control, out_router, 2, 2, router_exec, "out_router");


	// Make connections
	attach_ADC(control, 0, &control->null_component, 0);
	attach_ADC(control, 1, in_router, 2);
	set_target(noise, 0, in_router, 0);
	set_target(osc, 0, in_router, 1);
	set_target(in_router, 0, out_router, 0);
	set_target(in_router, 1, fir, 0);
	set_target(fir, 0, inv, 0);
	set_target(inv, 0, out_router, 1);
	set_target(in_router, 2, corr, 0);
	set_target(in_router, 3, corr, 1);
	set_target(in_router, 4, delay, 0);
	set_target(delay, 0, ftf1, 0);
	set_target(in_router, 5, ftf1, 1);
	set_target(ftf1, 0, ftf2, 0);
	set_target(in_router, 6, ftf2, 1);
	set_target(out_router, 0, &control->dac_out, 0);
	set_target(out_router, 1, &control->dac_out, 1);


	// Set component parameters
	noise->float_params[0] = -1.0f; // Mininum value
	noise->float_params[1] = 1.0f; // Maximum value

	osc->int_params[0] = 50;

	in_router->int_params[0] = 0x1 | 0x4; // Route noise towards dac and to cross-correlation
	in_router->int_params[1] = 0x0; // Silence sinusoid
	in_router->int_params[2] = 0x8; // Route adc(r) to cross-correlation

	out_router->int_params[0] = 0x2; // Route noise to dac(r)
	out_router->int_params[1] = 0x0; // Silence fir

	delay->int_params[0] = 0; // Delay in samples

	corr->int_params[0] = 1; // Update cross-correlation
	corr->int_params[1] = 1; // Reset before first iteration
	corr->int_params[2] = OA__CORR_ITERS; // Finish after this many samples (can restart by setting above parameters to 1)

	ftf1->float_params[0] = 0.997; // lambda (forgetting factor)
	ftf1->float_params[1] = 0.5;    // mu (init value)
	ftf1->float_params[2] = 0.995; // v1 (leakage factor)
	ftf1->float_params[3] = 1e-07; // c1 (regularisation constant)

	ftf1->int_params[0] = 0; // update flag
	ftf1->int_params[1] = 0; // reset flag
	ftf1->int_params[2] = 0; // apply flag
	ftf1->int_params[3] = 0; // err_in flag
	ftf1->int_params[4] = 0; // err_out flag

	ftf2->float_params[0] = 0.997; // lambda (forgetting factor)
	ftf2->float_params[1] = 0.5;    // mu (init value)
	ftf2->float_params[2] = 0.995; // v1 (leakage factor)
	ftf2->float_params[3] = 1e-07; // c1 (regularisation constant)

	ftf2->int_params[0] = 0; // update flag
	ftf2->int_params[1] = 0; // reset flag
	ftf2->int_params[2] = 0; // apply flag
	ftf2->int_params[3] = 0; // err_in flag
	ftf2->int_params[4] = 0; // err_out flag

	fir->int_params[0] = 0; // apply flag


	// Evaluate DSP graph
	build_evaluation_order(control);


	// Set event handlers
	*onButtonDown = (eventHandler)oa__advanceCalibration;
	*beforeProcessing = (eventHandler)oa__saveCorrBusyState;
	*afterProcessing = (eventHandler)oa__maybeFinishCorrelation;


	// Initialise state structure
	offline_anc_state* state = (offline_anc_state *)alloc_or_fail(1, sizeof(offline_anc_state), "offline_anc_state");
	state->setup = setup;
	state->calibrationStep = 1;
	state->corrBusy = 1;


	return state;
}


void oa__advanceCalibration(offline_anc_state *state)
{
	dsp_comp* in_router = state->setup->in_router;
	dsp_comp* out_router = state->setup->out_router;
	dsp_comp* corr = state->setup->corr;
	dsp_comp* ftf1 = state->setup->ftf1;
	dsp_comp* ftf2 = state->setup->ftf2;
	dsp_comp* fir = state->setup->fir;

	switch(state->calibrationStep)
	{
	case 0:
		//Log("c%d load:%.0f%%(%d)\n", calibrating, lastLoad, lastSamplesProcessed);
		in_router->int_params[0] = 0x1 | 0x4; // Route noise towards dac and to cross-correlation
		in_router->int_params[1] = 0x0;       // Silence sinusoid
		in_router->int_params[2] = 0x8;       // Route adc(r) to cross-correlation
		out_router->int_params[0] = 0x2;       // Route noise to dac(r)
		out_router->int_params[1] = 0x0;       // Silence fir
		fir->int_params[0] = 0;             // Stop applying fir
		corr->int_params[0] = 1;            // Start updating cross-correlation
		corr->int_params[0] = 1;            // Reset cross-correlation
		state->calibrationStep = 1;
		break;
	case 1:
		// Cross-correlation mode will end automatically, ignore button input
		break;
	case 2:
		sendWaveform(ftf1->buffer[1].buff, ftf1->buffer[1].size);
		//Log("c%d load:%.0f%%(%d)\n", calibrating, lastLoad, lastSamplesProcessed);
		in_router->int_params[0] = 0x1 | 0x2 | 0x10; // Route noise towards dac, to ftf2 and to delay
		in_router->int_params[1] = 0x0;              // Silence sinusoid
		in_router->int_params[2] = 0x40;             // Route adc(r) to ftf2
		out_router->int_params[0] = 0x1;              // Route noise to dac(l)
		out_router->int_params[1] = 0x0;              // Silence fir
		ftf1->int_params[0] = 0;                   // Stop updating ftf1
		ftf1->int_params[2] = 1;                   // Start applying ftf1
		ftf2->int_params[0] = 1;                   // Start updating ftf2
		ftf2->int_params[1] = 1;                   // Reset ftf2
		state->calibrationStep = 3;
		break;
	case 3:
		sendWaveform(ftf2->buffer[1].buff, ftf2->buffer[1].size);
		//Log("c%d load:%.0f%%(%d)\n", calibrating, lastLoad, lastSamplesProcessed);
		in_router->int_params[0] = 0x0;       // Silence noise
		in_router->int_params[1] = 0x1 | 0x2; // Route sinusoid towards dac and to fir
		in_router->int_params[2] = 0x0;       // Silence adc(r)
		out_router->int_params[0] = 0x1;       // Route sinusoid to dac(l)
		out_router->int_params[1] = 0x2;       // Route fir to dac(r)
		ftf1->int_params[0] = 0;            // Stop updating ftf1
		ftf1->int_params[2] = 0;            // Stop applying ftf1
		ftf2->int_params[0] = 0;            // Stop updating ftf2
		ftf2->int_params[2] = 0;            // Stop applying ftf2
		fir->int_params[0] = 1;             // Start applying fir
		state->calibrationStep = 0;
		break;
	}
}


void destroyOfflineANC(offline_anc_state *state)
{
	free_generic(state->setup->noise);
	free_generic(state->setup->osc);
	free_generic(state->setup->in_router);
	free_generic(state->setup->out_router);
	free_fir(state->setup->fir);
	free_generic(state->setup->inv);
	free_corr(state->setup->corr);
	free_delay(state->setup->delay);
	free_ftf(state->setup->ftf1);
	free_ftf(state->setup->ftf2);
	free(state->setup);
	free(state);
}


void oa__saveCorrBusyState(offline_anc_state *state)
{
	state->corrBusy = state->setup->corr->int_params[0];
}


void oa__maybeFinishCorrelation(offline_anc_state *state)
{
	dsp_comp* in_router = state->setup->in_router;
	dsp_comp* out_router = state->setup->out_router;
	dsp_comp* corr = state->setup->corr;
	dsp_comp* delay = state->setup->delay;
	dsp_comp* ftf1 = state->setup->ftf1;

	if(state->calibrationStep == 1 && state->corrBusy && !corr->int_params[0])
	{
		sendWaveform(corr->buffer[1].buff, corr->buffer[1].size);
		//Log("c%d load:%.0f%%(%d)\n", calibrating, lastLoad, lastSamplesProcessed);

		float maxval = 0.0f;
		uint16_t k, maxidx = 0;
		for(k = 0; k < corr->buffer[1].size; k++)
		{
			if(fabs(corr->buffer[1].buff[k]) > maxval)
			{
				maxval = fabs(corr->buffer[1].buff[k]);
				maxidx = k;
			}
		}
		delay->int_params[0] = maxidx - OA__PREDELAY_OFFSET;
		DEBUG_PRINTF("predelay: %ld\n", delay->int_params[0]);
		if(delay->int_params[0] < 0) delay->int_params[0] = 0;

		in_router->int_params[0] = 0x1 | 0x10; // Route noise towards dac and to delay
		in_router->int_params[1] = 0x0;        // Silence sinusoid
		in_router->int_params[2] = 0x20;       // Route adc(r) to ftf1
		out_router->int_params[0] = 0x2;        // Route noise to dac(r)
		out_router->int_params[1] = 0x0;        // Silence fir

		ftf1->int_params[0] = 1; // Start updating ftf1
		ftf1->int_params[1] = 1; // Reset ftf1

		state->calibrationStep = 2;
	}
}


/*

void correlate(component_buffer* buff1, component_buffer* buff2, component_buffer* outbuff)
{
	// NOTE: need outbuff->size == 2*max(buff1->size, buff2->size) - 1
	outbuff->curr = 0;

	arm_correlate_f32(buff1->buff, buff1->size, buff2->buff, buff2->size, outbuff->buff);
}

uint16_t findDelay(component_buffer* corr_buff)
{
	float maxval = 0.0f, val = 0.0f;
	uint16_t maxidx = 0, j = 0;

	DEBUG_PRINTF("CC:\n");
	for(j = 0; j < corr_buff->size; j++)
	{
		val = fabs(hist(corr_buff, j));
		if(val > maxval)
		{
			maxval = val;
			maxidx = j;
		}
		DEBUG_PRINTF("%.1e,\n", hist(corr_buff, j));
	}

	for(j = maxidx; hist(corr_buff, j)*hist(corr_buff, j-1) > 0; j--) ;

	return j-1;
}

*/

