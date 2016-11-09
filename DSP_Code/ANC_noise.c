#include "ANC_noise.h"
#include "global.h"
#include "component.h"
#include "test_components.h"

#if RUNNING_ON_EMBEDDED == 1

#include "tm_stm32_rng.h"

uint16_t ANC_Noise_Exec(dsp_comp* comp)
{
	// Generates random numbers between float_params[0] and float_params[1].

	uint8_t j;
	for(j=0; j<BLOCK_SIZE; j++)
	{
		uint32_t rnum = TM_RNG_Get();
		comp->pre_out[0][j] = (comp->float_params[1] - comp->float_params[0])*((float)rnum/0xffffffff) + comp->float_params[0];
	}

	finalise(comp);
	return 0;
}

#else

uint16_t ANC_Noise_Exec(dsp_comp* comp)
{
	return noise_exec(dsp_comp* comp);
}

#endif
