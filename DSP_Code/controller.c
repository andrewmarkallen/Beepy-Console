#include "controller.h"

void init(controller* c)
{
	init_sample_history(&c->history);
	c->n_comp = 0;
	c->failed_executions = 0;
  c->block_counter = 0;

	//dac_output component is what the last components in chain point to. Its execute is never called.
	c->dac_out.n_in = 2;
	c->dac_out.n_out = 0;
	c->dac_out.execute =  NULL; //Never gets called because dac_output is never registered
	comp_init(&c->dac_out);
	strcpy(c->dac_out.name,"dac_component");
  
	//When we initialise our controller, connect our ADC's to this initially. That way if we later don't connect our ADC's to any other component, there's no error when init_components() is called.
	c->null_component.n_in = N_INPUTS; //Same as number of ADC channels
	c->null_component.n_out = 0;
	c->null_component.execute =  NULL; //Never gets called because null_component is never registered
	comp_init(&c->null_component);
	strcpy(c->null_component.name,"null_component");

	//Connect our ADC's to the null_component
	int i = 0;
	for(i = 0; i < N_INPUTS; i++)
	{
		c->entry_comp[i].comp = &c->null_component;
		c->entry_comp[i].in = i;
	}
}

int reg_comp(controller* c, dsp_comp* comp)
{
  c->comp_list[c->n_comp] = comp;
  c->n_comp++;
  //comp->control = c;
  return 1;
}

int step(controller* c, float adc_1_payload, float adc_2_payload)
{  
  float payload[2] = {adc_1_payload, adc_2_payload};
  int i = 0;
  for(i = 0 ; i < N_OUTPUTS; i++)
  {
    //dac_out.in[i] contains an array of length BLOCK_SIZE which will be the previous block computed.
    //We output the elements of this array one by one, then when block_counter is incremented to be BLOCK_SIZE, the block is recomputed and counter reset to zero.
    c->out[i] = c->dac_out.in[i][c->block_counter];
  }
  for(i = 0; i < N_INPUTS; i++)
  {
    //We have N entry components. (At the moment hardcoded to 2)
    //Everytime step is called, we get the relevent inlet from the target for the i'th input.
    //Then we set the payload for the block_counter'th component of that inlet.
    uint8_t inlet = c->entry_comp[i].in;
    c->entry_comp[i].comp->in[inlet][c->block_counter] = payload[i];    
  }
  //Call execute for all our components if it's time. (i.e. we have all the elements of our block)
  c->block_counter++;
  if(c->block_counter == BLOCK_SIZE)
  {
    c->block_counter = 0;
    for(i = 0; i < c->n_comp; i++)
    {
      c->comp_list[i]->execute(c->comp_list[i]);
    }
  }
  return 1;
}
  
  
    //
  //
  // int i = 0;
  //
  // //Initialise starting components.
  // for(i = 0; i < N_INPUTS; i++)
  // {
  //   uint8_t inlet = c->entry_comp[i].in;
  //   c->entry_comp[i].comp->in[inlet] = payload[i];
  // }
  // for(i = 0; i < c->n_comp; i++)
  // {
  //   c->comp_list[i]->execute(c->comp_list[i]);
  // }
  // for(i = 0; i < N_OUTPUTS; i++)
  // {
  //   c->out[i] = c->dac_out.in[i];
  // }
  //
  //


int attach_ADC(controller* c, uint16_t outlet, dsp_comp* dest, uint16_t inlet)
{
  c->entry_comp[outlet].comp = dest;
  c->entry_comp[outlet].in = inlet;
  dest->parent[inlet].comp = &c->null_component; // hack
  return 0;
}


int setup_and_reg_comp(controller* c, dsp_comp* comp, uint16_t n_in, uint16_t n_out, execute_fptr exec, char* name_string)
{
  setup_comp(comp, n_in, n_out, exec, name_string);
  reg_comp(c, comp);
  return 1;
}

int setup_and_reg_comp_with_buffer(controller* c, dsp_comp* comp, uint16_t n_in, uint16_t n_out, execute_fptr exec, char* name_string, component_buffer* buffer)
{
  setup_comp_with_buffer(comp, n_in, n_out, exec, name_string, buffer);
  reg_comp(c, comp);
  return 1;
}

    //Firstly we build our list of orphans.
    //Secondly we add the orphans to the evaluation list and then remove them from the tree,
    //by deleting all references to them from their children.
    //Then we go through the tree again, and see which nodes are now orphans.
uint16_t build_evaluation_order(controller* c)
{
  //print_eval_order(c->comp_list, c->n_comp);
  //DEBUG_PRINTF("^^^^^ OLD ORDER BEFORE SORTING ^^^^^\n");
  
	uint16_t i = 0;
  uint16_t ordering[MAX_COMPONENTS];
  for(i = 0; i < MAX_COMPONENTS; i++)
  {
    ordering[i] = MAX_COMPONENTS;
  }  
	uint16_t size = 0;    
  uint16_t loops = 0;
  while(size < c->n_comp && loops < c->n_comp) //TODO: Figure out the max number of loops a bit more scientifically.
  {
    for(i = 0; i < c->n_comp; i++)
    {
      if(num_parents(c, c->comp_list[i]) == 0  && ordering[i] == MAX_COMPONENTS ) //Ordering[i] = MAX_COMPONENTS if component hasn't already been added to the evaluation order.
      {
        //DEBUG_PRINTF("FOUND ORPHAN: %s, giving ordering: %d\n", c->comp_list[i]->name, size);
        abandon_children(c, c->comp_list[i]);
        ordering[i] = size;
        size++; //Increment the total size of evaluation list counter.
      }      
    }  
    loops++;
  } 
  if(size < c->n_comp)
  {
    DEBUG_PRINTF("EVALUATION LIST BUILD FAILED.");
    return 0;
  }  
  //Now we sort.
  for(i = 0; i < c->n_comp; i++)
  {
    int j = 0;
    for(j = 0; j < c->n_comp; j++)
    {
      if(ordering[j] == i)
      {
        dsp_comp* swap_comp; 
        int swap_ordering;
        swap_comp = c->comp_list[i];        
        c->comp_list[i] = c->comp_list[j];
        c->comp_list[j] = swap_comp;
        swap_ordering = ordering[j];
        ordering[j] = ordering[i];
        ordering[i] = swap_ordering;
      }      
    }
  }
  //print_eval_order(c->comp_list, c->n_comp);
  return 1;
}

uint16_t num_parents(controller* c, dsp_comp* node)
{
  uint16_t num_parents = 0;
  int i = 0;
  for(i = 0; i < node->n_in; i++)
  {
    if((node->parent[i].comp != 0) && (node->parent[i].comp != &c->null_component))
    {
      num_parents++;
    }
  }
  return num_parents;
}


void print_eval_order(dsp_comp* node_list[], uint16_t size)
{
  DEBUG_PRINTF("EVALUATION ORDER:\n");
  int i = 0;
  for(i = 0; i < size; i++)
  {
    DEBUG_PRINTF("%d: %s\n", i, node_list[i]->name);
  }  
}

//Returns the number of children abandoned.
uint16_t abandon_children(controller* c, dsp_comp* comp)
{
  int i = 0;
  int total = 0;
  for(i = 0; i < comp->n_out; i++)
  {
    dsp_comp* child = comp->tgt[i].comp;
    uint16_t inlet = comp->tgt[i].in;
    
    if(child != &c->null_component && child != 0)
    {
      child->parent[inlet].comp = &c->null_component;
      //DEBUG_PRINTF("%s[%d] --X--> [%d]%s \n", comp->name, i, inlet, child->name);
      total++;
    }
  }
  return total;
}

float conv_from_adc(int16_t val)
{
	return (float)val/0xffff;
}

int16_t conv_to_dac(float val)
{
	return 0xffff & (int16_t)(val*0xffff);
}
