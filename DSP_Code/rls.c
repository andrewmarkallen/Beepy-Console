#include "rls.h"

void rls_reset(dsp_comp *comp)
{
	component_buffer* W_n = &comp->buffer[1];
	arm_matrix_instance_f32* P = (arm_matrix_instance_f32*)comp->pInstance;

	uint16_t n_order = W_n->size;
	float delta = comp->float_params[1];

	arm_fill_f32(0.0f, W_n->buff, n_order);
	arm_fill_f32(0.0f, P->pData, n_order*n_order);

	uint16_t i;
	for(i = 0; i < n_order; i++)
	{
		P->pData[i*n_order + i] = delta;
	}
}

uint16_t rls_exec(dsp_comp *comp)
{
	int32_t *update = &comp->int_params[0];
	int32_t *reset = &comp->int_params[1];
	int32_t apply = comp->int_params[2];
	int32_t err_in = comp->int_params[3];
	int32_t err_out = comp->int_params[4];

	float lambda = comp->float_params[0];

	float *filt_out = comp->pre_out[0];

	component_buffer* X_n = &comp->buffer[0];
	float* x_n = X_n->buff;
	component_buffer* W_n = &comp->buffer[1];
	float* w_n = W_n->buff;

	uint32_t n_order = W_n->size;

	arm_matrix_instance_f32* matrices = (arm_matrix_instance_f32*)comp->pInstance;
	arm_matrix_instance_f32* P = &matrices[0];
	arm_matrix_instance_f32* tempCol1 = &matrices[1];
	arm_matrix_instance_f32* tempCol2 = &matrices[2];
	arm_matrix_instance_f32* tempRow = &matrices[3];
	arm_matrix_instance_f32* tempSquare1 = &matrices[4];
	arm_matrix_instance_f32* tempSquare2 = &matrices[5];

	float sum, epsilon;

	if(*reset)
	{
		rls_reset(comp);
		*reset = 0;
	}

	advance_reverse_double_buffer(X_n, comp->in[0], BLOCK_SIZE);
	uint32_t rw_index = X_n->curr;

	uint8_t i;
	for(i = 0; i < BLOCK_SIZE; i++)
	{
		// Get right input sample
		float d_n = comp->in[1][i];

		// Align left buffer to correspond to the current right sample
		x_n = X_n->buff + rw_index - n_order + 1 - i;
		tempCol1->pData = x_n;

		if(apply || *update)
		{
			arm_dot_prod_f32(w_n, x_n, n_order, &sum);

			if(err_out)
			{
				// Output the error signal
				filt_out[i] = d_n - sum;
			}
			else
			{
				// Output the filtered signal
				filt_out[i] = sum;
			}
			// Calculate the error, unless it is being input directly
			epsilon = d_n - sum * (!err_in);
		}
		else
		{
			filt_out[i] = 0.0f;
			epsilon = 0.0f;
		}

		if(*update)
		{
			arm_mat_mult_f32(P, tempCol1, tempCol2);
			arm_dot_prod_f32(x_n, tempCol2->pData, n_order, &sum);
			sum += lambda;
			arm_scale_f32(tempCol2->pData, 1.0f/sum, tempCol2->pData, n_order);
			arm_mat_mult_f32(tempCol2, tempRow, tempSquare1);
			arm_mat_mult_f32(tempSquare1, P, tempSquare2);
			arm_mat_sub_f32(P, tempSquare2, P);
			arm_mat_scale_f32(P, 1.0f/lambda, P);
			arm_scale_f32(tempCol2->pData, epsilon, tempCol2->pData, n_order);
			arm_add_f32(w_n, tempCol2->pData, w_n, n_order);
		}
	}

	finalise(comp);
	return 0;
}
