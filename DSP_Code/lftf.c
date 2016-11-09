#include "lftf.h"

// comp->buffer[ ].buff:       // Interpretation (length) (M: filter order)
//  - 0: x(n)                  // Input signal (M+1)
//  - 1: A(n)                  // Forward prediction filter (M)
//  - 2: G(n)                  // Backward prediction filter (M)
//  - 3: k(n)                  // Kalman gain (M)
//  - 4: k_ext(n)              // Kalman gain (M+1)
//  - 5: w(n)                  // Filter taps (M)
// comp->in[ ]:
//  - 0: x(n)                  // Input signal (sample)
//  - 1: eps(n)                // Error signal (sample)
// comp->pre_out[ ]:
//  - 0: y(n)                  // Filtered output sample
// comp->float_params[ ]:
//  - 0: lambda                // Exponential forgetting factor
//  - 1: mu                    // Initialization parameter
//  - 2: v1                    // Leakage factor
//  - 3: alpha(n)
//  - 4: beta(n)
//  - 5: gamma(n)
// comp->int_params[ ]:
//  - 0: update                // Whether to update the filter or just apply it
//  - 1: reset                 // Whether to reset before the next iteration
//  - 2: err_in                // Whether second input is error signal (1) or desired signal (0)
//  - 3: err_out               // Whether the output is the error signal (1) or filtered signal (0)


void lftf_reset(dsp_comp* comp)
{
  float *A_n = comp->buffer[1].buff;
  float *G_n = comp->buffer[2].buff;
  float *k_n = comp->buffer[3].buff;
  float *k_ext_n = comp->buffer[4].buff;
  float *w_n = comp->buffer[5].buff;
  
  float lambda = comp->float_params[0];
  float mu = comp->float_params[1];
  float v1 = comp->float_params[2];
  int32_t kappa = 1;
  uint32_t n_order = comp->buffer[0].size - 1;
  
  float *alpha_n = &comp->float_params[3];
  float *beta_n = &comp->float_params[4];
  float *gamma_n = &comp->float_params[5];
  
  uint32_t j;
  for(j=0; j<n_order; j++)
  {
    A_n[j] = 0.0f;
    G_n[j] = 0.0f;
    k_n[j] = 0.0f;
    k_ext_n[j] = 0.0f;
    w_n[j] = 0.0f;
  }
  k_ext_n[n_order] = 0.0f;
  
  *alpha_n = mu * pow(lambda / pow(v1, 1.0f/((float)kappa)), (float)n_order);
  *beta_n = mu / pow(v1, 1.0f/((float)kappa));
  *gamma_n = 1.0f;
}

uint16_t lftf_exec(dsp_comp* comp)
{ 
  // // LFTF parameters
  // float lambda = comp->float_params[0];
  // float v1 = comp->float_params[2];
  // int32_t *update = &comp->int_params[0];
  // int32_t *reset = &comp->int_params[1];
  // int32_t err_in = comp->int_params[2];
  // int32_t err_out = comp->int_params[3];
  //
  // // Signals
  // component_buffer *x_n = &comp->buffer[0];
  // uint32_t n_order = x_n->size - 1;
  // int32_t *filt_out = &comp->pre_out[0];
  //
  // // Get input samples
  // x_n->curr = hist_idx(x_n, 1);
  // hist(x_n, 0) = (float)comp->in[0]/32768.0f;
  // float eps_n = (float)comp->in[1]/32768.0f;
  //
  //
  // // Recursive vector quantities
  // float *A_n = comp->buffer[1].buff;
  // float *G_n = comp->buffer[2].buff;
  // float *k_n = comp->buffer[3].buff;
  // float *k_ext_n = comp->buffer[4].buff;
  // float *w_n = comp->buffer[5].buff;
  //
  // // Recursive scalar quantities
  // float *alpha_n = &comp->float_params[3];
  // float *beta_n = &comp->float_params[4];
  // float *gamma_n = &comp->float_params[5];
  //
  // // Temporary variables
  // uint32_t j;
  // float sum;
  // float eta, f, rf, rb, psi, b, e;
  // float epsilon;
  //
  // // Reset filter if flag is set
  // if(*reset)
  // {
  //   //DEBUG_PRINTF("LFTF: Resetting.\n");
  //   lftf_reset(comp);
  //   *reset = 0;
  // }
  //
  // // Apply FIR filter
  // sum = 0.0f;
  // for(j=0; j<n_order; j++)
  // {
  //   sum += w_n[j] * hist(x_n, -j);
  // }
  // if(err_out)
  // {
  //     // Output the error signal
  //     *filt_out = (int32_t)(32768.0f*(eps_n - sum));
  // }
  // else
  // {
  //     // Output the filtered signal
  //     *filt_out = (int32_t)(32768.0f*sum);
  // }
  // // Calculate the error, unless it is being input directly
  // epsilon = eps_n - sum * (!err_in);
  //
  // // Adapt filter taps if flag is set
  // if(*update)
  // {
  //   eta = hist(x_n, 0);
  //   for(j=0; j<n_order; j++)
  //     eta -= hist(x_n, -(j+1)) * A_n[j];
  //
  //   f = *gamma_n * eta;
  //   rf = eta/(*alpha_n * lambda);
  //
  //   k_ext_n[0] = v1 * rf;
  //   for(j=1; j<=n_order; j++)
  //     k_ext_n[j] = v1 * (k_n[j-1] - rf * A_n[j-1]);
  //
  //   rb = k_ext_n[n_order];
  //
  //   *alpha_n = *alpha_n * lambda + f * eta;
  //
  //   for(j=0; j<n_order; j++)
  //     A_n[j] = v1 * (A_n[j] + f * k_n[j]);
  //
  //   psi = *beta_n * lambda * rb;
  //
  //   for(j=0; j<n_order; j++)
  //     k_n[j] = k_ext_n[j] + v1 * rb * G_n[j];
  //
  //   sum = 1.0f;
  //   for(j=0; j<n_order; j++)
  //     sum += hist(x_n, -j) * k_n[j];
  //   *gamma_n = 1.0f/sum;
  //
  //   b = *gamma_n * psi;
  //
  //   *beta_n = *beta_n * lambda + b * psi;
  //
  //   for(j=0; j<n_order; j++)
  //     G_n[j] = v1 * (G_n[j] + b * k_n[j]);
  //
  //   e = *gamma_n * epsilon;
  //
  //   if(isinf(e) || e != e)
  //   {
  //     DEBUG_PRINTF("LFTF: e = %f\n", e);
  //     // Fail or reset algorithm
  //     *update = 0;
  //     //*reset = 1;
  //   }
  //   else
  //   {
  //     //DEBUG_PRINTF("LFTF: Epsilon = %e, gamma = %e\n", epsilon, *gamma_n);
  //     for(j=0; j<n_order; j++)
  //       w_n[j] += k_n[j] * e;
  //   }
  // }
  
  finalise(comp);
  return 0;
}
