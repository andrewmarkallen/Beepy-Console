#include "debug_settings.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "component.h"
#include "controller.h"
//#include "RLS.h"
//#include "NLMS.h"
#include "test_components.h"
#include "wav.h"


#if RUNNING_ON_EMBEDDED == 0

wavfile prepare_output_header(FILE* i1, FILE* i2);

int prepare_data_for_adc(FILE* i1, FILE* i2, int16_t* di1, int16_t* di2, int16_t* do1, int16_t* do2, uint32_t n_samples);

int write_data_from_dac(wavfile* output_header, FILE* o1, FILE* o2, int16_t* do1, int16_t* do2, uint32_t n_samples);

int run_simulation(controller* controller);

void sample_setup(controller* controller, dsp_comp* amplifier, dsp_comp* mixer, dsp_comp* pass_through1, dsp_comp* pass_through2, dsp_comp* swapper, dsp_comp* amp0, dsp_comp* amp1);

int main()
{
  controller control;
  init(&control);

  dsp_comp noise;
  dsp_comp mixer;
  dsp_comp swap;
  dsp_comp amp1;
  dsp_comp amp2;

  setup_and_reg_comp(&control, &noise, 0, 1, noise_exec, "noise");
  setup_and_reg_comp(&control, &mixer, 2, 1, mixer_exec, "mixer");
  setup_and_reg_comp(&control, &swap, 2, 2, swap_exec, "swap");
  setup_and_reg_comp(&control, &amp1, 1, 1, amplifier_exec, "amp1");
  setup_and_reg_comp(&control, &amp2, 1, 1, amplifier_exec, "amp2");

  amp1.float_params[0] = 1;
  amp2.float_params[0] = 1;
  noise.int_params[0] = 0xBEEF; //Random seed

  attach_ADC(&control, 0, &mixer, 1);
  attach_ADC(&control, 1, &swap, 1);

  set_target(&noise, 0, &mixer, 0);
  set_target(&mixer, 0, &swap, 0);
  set_target(&swap, 0, &amp1, 0);
  set_target(&swap, 1, &amp2, 0);
  set_target(&amp1, 0, &control.dac_out, 0);
  set_target(&amp2, 0, &control.dac_out, 1);
  
  run_simulation(&control);  
  
  //EXAMPLE
  /*
  controller my_controller;
  init(&my_controller);

  dsp_comp amplifier;
  dsp_comp mixer;
  dsp_comp pass_through1;
  dsp_comp pass_through2;
  dsp_comp swapper;
  dsp_comp amp0;
  dsp_comp amp1;
  sample_setup(&my_controller, &amplifier, &mixer, &pass_through1, &pass_through2, &swapper, &amp0, &amp1);

  run_simulation(&my_controller);
  */
  //END EXAMPLE
}

int run_simulation(controller* controller)
{
  
  FILE* input1 = fopen("input1_sine.wav", "rb");
  FILE* input2 = fopen("input2_square.wav", "rb");
  FILE* output1 = fopen("output1.wav", "w+");
  FILE* output2 = fopen("output2.wav", "w+");
    
  wavfile output_header;
  
  output_header = prepare_output_header(input1, input2);
  
  if(output_header.bytes_in_data == 0)
  {
    printf("Failed to prepare data.\n");
    return 0;
  } 
  else
  {
    uint32_t n_samples = output_header.bytes_in_data/2; //Assume 16-bit audio
    //n_samples = 128; //Test value
    
    int16_t in_1[n_samples];
    int16_t in_2[n_samples];
    int16_t out_1[n_samples];
    int16_t out_2[n_samples];
    
    prepare_data_for_adc(input1, input2, &in_1[0], &in_2[0], &out_1[0], &out_2[0], n_samples);
    
    int i;
    for(i = 0; i < n_samples; i++)
    {
      step(controller,(float)in_1[i],(float)in_2[i]);         
      out_1[i] = (int16_t)controller->out[0];
      out_2[i] = (int16_t)controller->out[1];
    }    
    
    int16_t out_final_1[n_samples];
    int16_t out_final_2[n_samples];
    for(int i = 0; i < n_samples; i++)
    {
      out_final_1[i] = out_1[i];
      out_final_2[i] = out_2[i];
      
    }
    write_data_from_dac(&output_header, output1, output2, &out_1[0], &out_2[0], n_samples);
    return 1;    
  }  
}

//returns the output wav header for the output files. If the bytes_in_data field is 0, then this function failed.
wavfile prepare_output_header(FILE* i1, FILE* i2)
{  
  wavfile out_header;
  out_header.bytes_in_data = 0;
  
  wavfile h1;
  wavfile h2;
  if((i1 == NULL) || (i2 == NULL))
  {
    printf("One or both input files NULL\n");
    return out_header;
  }  
  if (!(read_header(i1, &h1)) || !(read_header(i2, &h2)))
  {
    printf("couldn't read one or both headers\n");
    return out_header;
  } 
  if( !(is_wav_format(&h1)) || !(is_wav_format(&h2)))
  {
    printf("One or both files not wav format\n");
    return out_header;
  }
  else
  {    
    if(h1.bytes_in_data > h2.bytes_in_data)
    {
      out_header = h2;
    }
    else
    {
      out_header = h1;
    }    
    return out_header;
  }
}

int prepare_data_for_adc(FILE* i1, FILE* i2, int16_t* di1, int16_t* di2, int16_t* do1, int16_t* do2, uint32_t n_samples)
{    
  load_data(i1, di1, n_samples);
  load_data(i2, di2, n_samples);
  memset(do1, 0, n_samples*2);
  memset(do2, 0, n_samples*2);
  return 0;
}

int write_data_from_dac(wavfile* output_header, FILE* o1, FILE* o2, int16_t* do1, int16_t* do2, uint32_t n_samples)
{   
    printf("Audio length (samples): %u\n", n_samples);
    
    int i = 0;
    for(i = 0; i < n_samples; i++)
    {
      if(i % 1000 == 0)
      {
        //printf("%u\n",(unsigned int)data_in_2[i]);
      }
    }    
    write_data(output_header, o1, do1, n_samples);
    write_data(output_header, o2, do2, n_samples);
    
    fclose(o1);
    fclose(o2);
    return 1;
}

//A simple sample setup

//  ADC1 ---> PASS_THROUGH1 --->|\       /| ---> AMP0(*2) -----> DAC1
//                              | SWAPPER |
//  ADC2 ---> PASS_THROUGH2 --->|/       \| ---> AMP1(*0.5) ---> DAC2
void sample_setup(controller* controller, dsp_comp* amplifier, dsp_comp* mixer, dsp_comp* pass_through1, dsp_comp* pass_through2, dsp_comp* swapper, dsp_comp* amp0, dsp_comp* amp1)
{
  //Note that setup_component only sets up n_inputs, n_outputs, execution function and name.
  //If you need to set any parameters, e.g. amplification value, you'll have to do that seperately.
  
  setup_comp(amplifier, 1, 1, amplifier_exec, "amplifier"); //Not used
  setup_comp(mixer, 2, 1, noise_exec, "mixer"); //Not used
  setup_comp(pass_through1, 1, 1, pass_through_exec, "pass_through1");
  setup_comp(pass_through2, 1, 1, pass_through_exec, "pass_through2");
  setup_comp(swapper, 2, 2, swap_exec, "swapper");
  setup_comp(amp0, 1, 1, amplifier_exec, "amp0");
  setup_comp(amp1, 1, 1, amplifier_exec, "amp1");
  
  amplifier->int_params[0] = 2048; //Not used
  amp0->int_params[0] = 1024; //Quarter volume
  amp1->int_params[0] = 1024; //Half volume
  
  reg_comp(controller, pass_through1);
  reg_comp(controller, pass_through2);
  reg_comp(controller, swapper);
  reg_comp(controller, amp0);
  reg_comp(controller, amp1);  
  
  //attach ADC(output_num) to dest(input_num)
  attach_ADC(controller,  0, pass_through1,  0);
  attach_ADC(controller,  1, pass_through2,  0);
  
  set_target(pass_through1, 0, swapper, 0);
  set_target(pass_through2, 0, swapper, 1);
  
  set_target(swapper, 0, amp0, 0);
  set_target(swapper, 1, amp1, 0);
  
  set_target(amp0, 0, &controller->dac_out, 0);
  set_target(amp1, 0, &controller->dac_out, 1);    
}

#endif
