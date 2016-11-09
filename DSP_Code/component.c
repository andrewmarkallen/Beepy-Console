#include "component.h"

int setup_comp(dsp_comp* comp, uint16_t n_inlets, uint16_t n_outlets, execute_fptr exec, char* name_string)
{
  comp->n_in = n_inlets;
  comp->n_out = n_outlets;
  comp->execute = exec;
  comp_init(comp);
  strcpy(comp->name, name_string);
  return 1;
}

int setup_comp_with_buffer(dsp_comp* comp, uint16_t n_inlets, uint16_t n_outlets, execute_fptr exec, char* name_string, component_buffer* buff)
{
	setup_comp(comp, n_inlets, n_outlets, exec, name_string);
	comp->buffer = buff;
  return 1;
}


int comp_init(dsp_comp* comp)
{
	memset(comp->name, 0, NAME_SIZE);
  int i = 0;
  for(i = 0; i < comp->n_in; i++)
  {
    comp->parent[i].comp = 0;
  }
  for(i = 0; i < comp->n_out; i++)
  {
    comp->tgt[i].comp = 0;
  }
  return 1;
}

int finalise(dsp_comp* comp)
{
  int i;
  for(i = 0; i < comp->n_out; i++)
  {
    uint8_t slot = comp->tgt[i].in;
    memcpy(comp->tgt[i].comp->in[slot], comp->pre_out[i], BLOCK_SIZE*sizeof(float));
  }
  return 1;
}

//Helper function to wire the output of a component to the input of another
int set_target(dsp_comp* source, uint16_t outlet, dsp_comp* dest, uint16_t inlet)
{
  source->tgt[outlet].comp = dest;
  source->tgt[outlet].in = inlet;
  dest->parent[inlet].comp = source;
  //TODO: Check that input and output number are valid
  return 0;
}
