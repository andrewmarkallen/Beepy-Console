#include "recorder_test.h"


recorder_test_state *setupRecorderTest(controller *control, eventHandler *onButtonDown, eventHandler *onButtonUp, eventHandler *beforeProcessing, eventHandler *afterProcessing)
{
	recorder_test_setup *setup = alloc_or_fail(1, sizeof(*setup), "recorder_test_setup");

	setup->recorder = alloc_recorder(1, RT__REC_SIZE);
	setup->player = alloc_player(1, RT__REC_SIZE, &setup->recorder->buffer->buff);

	setup_and_reg_comp(control, setup->recorder, 1, 0, recorder_exec, "recorder");
	setup_and_reg_comp(control, setup->player, 0, 1, player_exec, "player");

	attach_ADC(control, 0, &control->null_component, 0);
	attach_ADC(control, 1, setup->recorder, 0);
	set_target(&control->null_component, 0, &control->dac_out, 0);
	set_target(setup->player, 0, &control->dac_out, 1);

	setup->recorder->int_params[0] = 0x0;
	setup->player->int_params[0] = 0x0;
	setup->player->int_params[1] = RT__LOOP;

	build_evaluation_order(control);

	*onButtonDown = (eventHandler)rt__record;
	*onButtonUp = (eventHandler)rt__play;

	recorder_test_state *state = alloc_or_fail(1, sizeof(*state), "recorder_test_state");
	state->setup = setup;

	return state;
}


void destroyRecorderTest(recorder_test_state *state)
{
	free_player(state->setup->player);
	free_recorder(state->setup->recorder);
	free(state->setup);
	free(state);
}


void rt__record(recorder_test_state *state)
{
	state->setup->player->int_params[0] = 0x0;
	state->setup->recorder->int_params[0] = 0x1;
	state->setup->recorder->buffer[0].curr = 0;
}


void rt__play(recorder_test_state *state)
{
	state->setup->recorder->int_params[0] = 0x0;
	state->setup->player->int_params[0] = 0x1;
	state->setup->player->buffer[0].curr = 0;
}
