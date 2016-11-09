#include "msmftf.h"

// comp->buffer[ ].buff:       // Interpretation (length) (M: filter order)
//  - 0: x(n)                  // Input signal (2*(M+1))
//  - 1: w(n)                  // Filter taps (M)
//  - 2: a(n)                  // Forward prediction filter (M)
//  - 3: k(n)                  // Kalman gain (M)
//  - 4: k_ext(n)              // Kalman gain (M+1)
// comp->in[ ]:
//  - 0: x(n)                  // Input signal (sample)
//  - 1: eps(n)                // Error signal (sample)
// comp->pre_out[ ]:
//  - 0: y(n)                  // Filtered output sample
// comp->float_params[ ]:
//  - 0: lambda                // Exponential forgetting factor
//  - 1: mu                    // Initialization parameter
//  - 2: v1                    // Leakage factor
//  - 3: c1                    // Regularisation constant
//  - 4: alpha(n)
//  - 5: gamma(n)
// comp->int_params[ ]:
//  - 0: update                // Whether to update the filter or just apply it
//  - 1: reset                 // Whether to reset before the next iteration
//  - 2: apply                 // Whether to apply the filter or output 0
//  - 3: err_in                // Whether second input is error signal (1) or desired signal (0)
//  - 4: err_out               // Whether the output is the error signal (1) or filtered signal (0)


void msmftf_reset(dsp_comp* comp)
{
	float *w_n = comp->buffer[1].buff;
	float *a_n = comp->buffer[2].buff;
	float *k_n = comp->buffer[3].buff;
	float *k_ext_n = comp->buffer[4].buff;

	float lambda = comp->float_params[0];
	float mu = comp->float_params[1];
	//float v1 = comp->float_params[2];
	uint32_t n_order = comp->buffer[1].size;

	float *alpha_n = &comp->float_params[4];
	float *gamma_n = &comp->float_params[5];

	arm_fill_f32(0.0f, w_n, n_order);
	arm_fill_f32(0.0f, a_n, n_order);
	arm_fill_f32(0.0f, k_n, n_order);
	arm_fill_f32(0.0f, k_ext_n, n_order+1);

	*alpha_n = mu * pow(lambda, (float)n_order);
	*gamma_n = 1.0f;
}

uint16_t msmftf_exec(dsp_comp* comp)
{ 
	// MSMFTF parameters
	float lambda = comp->float_params[0];
	float v1 = comp->float_params[2];
	float c1 = comp->float_params[3];
	int32_t *update = &comp->int_params[0];
	int32_t *reset = &comp->int_params[1];
	int32_t apply = comp->int_params[2];
	int32_t err_in = comp->int_params[3];
	int32_t err_out = comp->int_params[4];

	// Signals
	component_buffer *X_n = &comp->buffer[0];
	float *x_n = X_n->buff;
	float *filt_out = comp->pre_out[0];

	// Recursive vector quantities
	float *w_n = comp->buffer[1].buff;
	float *a_n = comp->buffer[2].buff;
	float *k_n = comp->buffer[3].buff;
	float *k_ext_n = comp->buffer[4].buff;

	// Recursive scalar quantities
	float *alpha_n = &comp->float_params[4];
	float *gamma_n = &comp->float_params[5];

	uint32_t n_order = comp->buffer[1].size;

	// Temporary variables
	uint8_t i;
	float sum;
	float e, c, delta;
	float epsilon;

	// Reset filter if flag is set
	if(*reset)
	{
		//DEBUG_PRINTF("MSMFTF: Resetting.\n");
		msmftf_reset(comp);
		*reset = 0;
	}

	// Get left input block
	advance_reverse_double_buffer(X_n, comp->in[0], BLOCK_SIZE);
	uint32_t rw_index = X_n->curr;

	for(i=0; i<BLOCK_SIZE; i++)
	{
		// Get right input sample
		float d_n = comp->in[1][i];

		// Align left buffer to correspond to the current right sample
		x_n = X_n->buff + rw_index - n_order - i;

		// Apply FIR filter
		if(apply || *update)
		{
			arm_dot_prod_f32(w_n, x_n, n_order, &sum);

			if(comp->n_out)
			{
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
			}
			// Calculate the error, unless it is being input directly
			epsilon = d_n - sum * (!err_in);
		}
		else
		{
			filt_out[i] = 0.0f;
			epsilon = 0.0f;
		}

		// Adapt filter taps if flag is set
		if(*update)
		{
			arm_dot_prod_f32(a_n, x_n + 1, n_order, &e);
			e = x_n[0] - e;

			k_ext_n[0] = e/(*alpha_n * lambda + c1);

			// Equivalent to: k_ext_n[j+1] = k_n[j] - k_ext_n[0] * a_n[j]
			arm_scale_f32(a_n, k_ext_n[0], k_ext_n + 1, n_order);
			arm_sub_f32(k_n, k_ext_n + 1, k_ext_n + 1, n_order);

			// Equivalent to: a_n[j] = v1 * (a_n[j] + gamma_n * e * k_n[j])
			arm_scale_f32(k_n, *gamma_n * e, k_n, n_order);
			arm_add_f32(k_n, a_n, a_n, n_order);
			arm_scale_f32(a_n, v1, a_n, n_order);

			*alpha_n = *alpha_n * lambda + *gamma_n * e * e;

			// Equivalent to: k_n[j] = k_ext_n[j]
			arm_copy_f32(k_ext_n, k_n, n_order);

			c = k_ext_n[n_order];
			delta = e * k_ext_n[0] - c * x_n[n_order];
			*gamma_n = *gamma_n / (1.0f + *gamma_n * delta);

			epsilon *= *gamma_n;

			if(isinf(epsilon) || epsilon != epsilon)
			{
				DEBUG_PRINTF("MSMFTF: epsilon = %f\n", epsilon);
				// Fail or reset algorithm
				*update = 0;
				//*reset = 1;
			}
			else
			{
				//DEBUG_PRINTF("MSMFTF: Epsilon = %e, gamma = %e\n", epsilon, *gamma_n);

				// Equivalent to: w_n[j] += epsilon * k_n[j]
				// NOTE: k_n mustn't be changed, so we use k_ext_n which holds the same first n_order values
				arm_scale_f32(k_ext_n, epsilon, k_ext_n, n_order);
				arm_add_f32(w_n, k_ext_n, w_n, n_order);
			}
		}
	}

	finalise(comp);
	return 0;
}
