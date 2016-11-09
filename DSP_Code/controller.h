#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <stdio.h>
#include "component.h"
#include "global.h"
#include "history.h"

#define MAX_COMPONENTS 64
#define N_INPUTS 2 // ADC channels
#define N_OUTPUTS 2
#define MAX_TRIES 3 //Number of tries before giving up in step loop

//Again, not good to dynamically allocate memory, so preallocate MAX_COMPONENT size array for components.

typedef struct
{
  dsp_comp* comp_list[MAX_COMPONENTS];
  uint8_t block_counter; // Counts how many samples we've received
  uint16_t n_comp; //Number of components in our system
  
  dsp_comp dac_out;
  
  
  
  //dsp_comp* comp_list[MAX_COMPONENTS];
  //uint8_t block_counter; //Counts how many samples we've received.
  //uint16_t n_comp;
  //uint16_t unused; //Can write unused inputs here
  target entry_comp[N_INPUTS];
  dsp_comp null_component; //When we initialise our controller, connect our ADC's to this initially. That way if we later don't connect our ADC's to any other component, there's no error when init_components() is called.
  uint32_t failed_executions;
  float out[N_OUTPUTS];
  adc_sample_history history;
  
  //float out_b[N_OUTPUTS][BLOCK_SIZE];

}controller;

void init(controller* c);

int reg_comp(controller* c, dsp_comp* comp);
int step(controller* c, float adc_1_payload, float adc_2_payload); //Run through the components in the list. This assumes stereo input.
int attach_ADC(controller* c, uint16_t adc_outlet, dsp_comp* dest, uint16_t inlet); //Attach the ADC to another component
int setup_and_reg_comp(controller* c, dsp_comp* comp, uint16_t n_in, uint16_t n_out, execute_fptr exec, char* name_string);
int setup_and_reg_comp_with_buffer(controller* c, dsp_comp* comp, uint16_t n_in, uint16_t n_out, execute_fptr exec, char* name_string, component_buffer* buffer);
uint16_t build_evaluation_order(controller* c);
uint16_t num_parents(controller* c, dsp_comp* node);
void print_eval_order(dsp_comp* node_list[], uint16_t size);
uint16_t abandon_children(controller* c, dsp_comp* comp);

float conv_from_adc(int16_t val);
int16_t conv_to_dac(float val);

#endif
