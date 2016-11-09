#include "controller.h"
#include "global.h"


#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <string.h>
#include <stdio.h>
#include "controller.h"
#include "history.h"
#include <stdint.h>

#define PARAM_SIZE 6
#define INPUT_SIZE 8
#define NAME_SIZE 16
#define HALF_RANGE 32768
#define OUTPUT_SIZE INPUT_SIZE


typedef uint16_t (*execute_fptr)();

struct dsp_comp_struct;

typedef struct
{
  struct dsp_comp_struct* comp;
  uint8_t in; //inlet number
}target;

typedef struct dsp_comp_struct
{
  //Dynamic allocation of memory is discouraged on embedded devices, so I pre-define the size of the input array.
  char name[NAME_SIZE];
  int32_t int_params[PARAM_SIZE];
  float float_params[PARAM_SIZE];
  void *pInstance; // Some CMSIS DSP functions need to refer to an instance variable

  uint16_t n_in;
  uint16_t n_out;
  float in[INPUT_SIZE][BLOCK_SIZE]; 
  float pre_out[OUTPUT_SIZE][BLOCK_SIZE];
  
  target tgt[OUTPUT_SIZE];  
  target parent[INPUT_SIZE];
  //controller* control; //So far this is unneeded
  component_buffer* buffer; //Array of component buffers. Only needed if it's needed.

  execute_fptr execute; //function pointer to execute function
}dsp_comp;

//Simple function that takes in the parameters below and sets them
int setup_comp(dsp_comp* comp, uint16_t n_in, uint16_t n_out, execute_fptr exec, char* name_string);

//Same as above but lets you pass in a buffer for past inputs if necessary
int setup_comp_with_buffer(dsp_comp* comp, uint16_t n_inlets, uint16_t n_outlets, execute_fptr exec, char* name_string, component_buffer* buff);

// Call this at the end of the execute step to send outputs to input of next component, and set ready flag
int finalise(dsp_comp* comp);

//Calls the initialise function, should be called for each component before starting dsp loop. Right now all this does is clears the name memory.
int comp_init(dsp_comp* comp);

//Helper function for wiring two components together
int set_target(dsp_comp* source, uint16_t outlet, dsp_comp* dest, uint16_t inlet);


#endif
