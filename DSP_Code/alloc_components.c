#include "alloc_components.h"


float allFloats[ALLOC_FLOATS];
float *reservePtr = allFloats;
uint32_t floatsReserved = 0;


void alloc_fail(uint32_t numBytes, char compName[])
{
	DEBUG_PRINTF("Could not allocate %lu bytes for: %s\n", numBytes, compName);
	//Log("Could not allocate %lu bytes for: %s\n", numBytes, compName);
	// TODO: halt execution?
}

void *alloc_or_fail(uint32_t num, uint32_t size, char compName[])
{
	void *ptr = calloc(num, size);
	if(ptr == NULL)
	{
		alloc_fail(num * size, compName);
	}
	return ptr;
}

float *reserve_floats_or_fail(uint32_t num, char buffName[])
{
	if(floatsReserved + num > ALLOC_FLOATS)
	{
		alloc_fail(num * sizeof(float), buffName);
		return NULL;
	}

	floatsReserved += num;

	float *start = reservePtr;
	reservePtr += num;

	return start;
}

void reset_float_reservations()
{
	floatsReserved = 0;
	reservePtr = allFloats;
}

dsp_comp *alloc_generic()
{
	dsp_comp *comp = alloc_or_fail(1, sizeof(*comp), "generic");
	return comp;
}

void free_generic(dsp_comp* comp)
{
	free(comp);
}

dsp_comp *alloc_delay(uint32_t delay_size)
{
	dsp_comp *delay = alloc_or_fail(1, sizeof(*delay), "delay");

	uint32_t delay_dblbuf = DOUBLE_BUFFER_SIZE(delay_size);

	component_buffer *delay_buffer = alloc_or_fail(1, sizeof(*delay_buffer), "delay_buffer");

	float *delay_hist = reserve_floats_or_fail(delay_dblbuf, "delay_hist");

	init_component_buffer(delay_buffer, delay_hist, delay_dblbuf);

	delay->buffer = delay_buffer;

	return delay;
}

void free_delay(dsp_comp* delay)
{
	free(delay->buffer);
	free(delay);
}

dsp_comp *alloc_corr(uint32_t corr_size)
{
	dsp_comp *corr = alloc_or_fail(1, sizeof(*corr), "corr");

	uint32_t corr_dblbuf = DOUBLE_BUFFER_SIZE(corr_size);
	uint32_t corr_cmptm = COMPATIBLE_M(corr_size);

	component_buffer *corr_buffers = alloc_or_fail(3, sizeof(*corr_buffers), "corr_buffers");

	float *corr_hist = reserve_floats_or_fail(corr_dblbuf, "corr_hist");
	float *corr_vec  = reserve_floats_or_fail(corr_cmptm,  "corr_vec");
	float *corr_temp = reserve_floats_or_fail(corr_cmptm,  "corr_temp");

	init_reverse_component_buffer(&corr_buffers[0], corr_hist, corr_dblbuf);
	init_component_buffer(&corr_buffers[1], corr_vec, corr_cmptm);
	init_component_buffer(&corr_buffers[2], corr_temp, corr_cmptm);

	corr->buffer = corr_buffers;

	return corr;
}

void free_corr(dsp_comp* corr)
{
	free(corr->buffer);
	free(corr);
}

dsp_comp *alloc_fir(uint32_t fir_size, float *fir_coeffs)
{
	dsp_comp *fir = alloc_or_fail(1, sizeof(*fir), "fir");

	if(fir_coeffs == NULL)
	{
		fir_coeffs = reserve_floats_or_fail(fir_size, "fir_coeffs");
	}

	float *fir_state = reserve_floats_or_fail(fir_size+BLOCK_SIZE-1, "fir_state");

	fir->pInstance = alloc_or_fail(1, sizeof(arm_fir_instance_f32), "fir_instance");

	arm_fir_init_f32(fir->pInstance, fir_size, fir_coeffs, fir_state, BLOCK_SIZE);

	return fir;
}

void free_fir(dsp_comp *fir)
{
	free(fir->pInstance);
	free(fir);
}

dsp_comp *alloc_recorder(uint8_t rec_num, uint32_t rec_size)
{
	dsp_comp *recorder = alloc_or_fail(1, sizeof(*recorder), "recorder");

	component_buffer *recorder_buffers = alloc_or_fail(rec_num, sizeof(*recorder_buffers), "recorder_buffers");

	uint8_t j;
	for(j = 0; j < rec_num; j++)
	{
		float *rec_buff = reserve_floats_or_fail(rec_size, "rec_buff");
		init_component_buffer(&recorder_buffers[j], rec_buff, rec_size);
	}

	recorder->buffer = recorder_buffers;

	return recorder;
}

void free_recorder(dsp_comp *recorder)
{
	free(recorder->buffer);
	free(recorder);
}

dsp_comp *alloc_player(uint8_t play_num, uint32_t play_size, float **play_buffers)
{
	dsp_comp *player = alloc_or_fail(1, sizeof(*player), "player");

	component_buffer *player_buffers = alloc_or_fail(play_num, sizeof(*player_buffers), "player_buffers");

	uint8_t j;
	for(j = 0; j < play_num; j++)
	{
		init_component_buffer(&player_buffers[j], *play_buffers++, play_size);
	}

	player->buffer = player_buffers;

	return player;
}

void free_player(dsp_comp *player)
{
	free(player->buffer);
	free(player);
}

dsp_comp *alloc_rls(uint32_t rls_size)
{
	dsp_comp *rls = alloc_or_fail(1, sizeof(*rls), "rls");

	uint32_t rls_dblbuf = DOUBLE_BUFFER_SIZE(rls_size);
	uint32_t rls_cmptm = COMPATIBLE_M(rls_size);

	component_buffer *rls_buffers = alloc_or_fail(2, sizeof(*rls_buffers), "rls_buffers");

	float *rls_x = reserve_floats_or_fail(rls_dblbuf, "rls_x");
	float *rls_w = reserve_floats_or_fail(rls_cmptm, "rls_w");

	init_reverse_component_buffer(&rls_buffers[0], rls_x, rls_dblbuf);
	init_component_buffer(&rls_buffers[1], rls_w, rls_cmptm);

	rls->buffer = rls_buffers;

	arm_matrix_instance_f32 *rls_instances = alloc_or_fail(6, sizeof(*rls_instances), "rls_instances");

	float *rls_P           = reserve_floats_or_fail(rls_cmptm*rls_cmptm, "rls_P");
	float *rls_tempCol     = reserve_floats_or_fail(rls_cmptm,           "rls_tempCol");
	float *rls_tempRow     = reserve_floats_or_fail(rls_cmptm,           "rls_tempRow");
	float *rls_tempSquare1 = reserve_floats_or_fail(rls_cmptm*rls_cmptm, "rls_tempSquare1");
	float *rls_tempSquare2 = reserve_floats_or_fail(rls_cmptm*rls_cmptm, "rls_tempSquare2");

	arm_mat_init_f32(&rls_instances[0], rls_cmptm, rls_cmptm, rls_P);
	arm_mat_init_f32(&rls_instances[1], rls_cmptm, 1,         NULL);
	arm_mat_init_f32(&rls_instances[2], rls_cmptm, 1,         rls_tempCol);
	arm_mat_init_f32(&rls_instances[3], 1,         rls_cmptm, rls_tempRow);
	arm_mat_init_f32(&rls_instances[4], rls_cmptm, rls_cmptm, rls_tempSquare1);
	arm_mat_init_f32(&rls_instances[5], rls_cmptm, rls_cmptm, rls_tempSquare2);

	rls->pInstance = rls_instances;

	return rls;
}

void free_rls(dsp_comp *rls)
{
	free(rls->buffer);
	free(rls->pInstance);
	free(rls);
}

dsp_comp *alloc_ftf(uint32_t ftf_size)
{
	dsp_comp *ftf = alloc_or_fail(1, sizeof(*ftf), "ftf");

	uint32_t ftf_dblbuf = DOUBLE_BUFFER_SIZE(ftf_size+1);
	uint32_t ftf_cmptm = COMPATIBLE_M(ftf_size+1)-1;

	component_buffer *ftf_buffers = alloc_or_fail(5, sizeof(*ftf_buffers), "ftf_buffers");

	float *ftf_x     = reserve_floats_or_fail(ftf_dblbuf,  "ftf_x");
	float *ftf_w     = reserve_floats_or_fail(ftf_cmptm,   "ftf_w");
	float *ftf_a     = reserve_floats_or_fail(ftf_cmptm,   "ftf_a");
	float *ftf_k     = reserve_floats_or_fail(ftf_cmptm,   "ftf_k");
	float *ftf_k_ext = reserve_floats_or_fail(ftf_cmptm+1, "ftf_k_ext");

	init_reverse_component_buffer(&ftf_buffers[0], ftf_x, ftf_dblbuf);
	init_component_buffer(&ftf_buffers[1], ftf_w, ftf_cmptm);
	init_component_buffer(&ftf_buffers[2], ftf_a, ftf_cmptm);
	init_component_buffer(&ftf_buffers[3], ftf_k, ftf_cmptm);
	init_component_buffer(&ftf_buffers[4], ftf_k_ext, ftf_cmptm+1);

	ftf->buffer = ftf_buffers;

	return ftf;
}

void free_ftf(dsp_comp *ftf)
{
	free(ftf->buffer);
	free(ftf);
}
