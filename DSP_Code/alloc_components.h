#ifndef __ALLOC_COMPONENTS_H_
#define __ALLOC_COMPONENTS_H_


#include "global.h"
#include "component.h"
#include "serlcd.h"


#define ALLOC_FLOATS 20000 // Statically allocate this many floats for usage by components


float *reserve_floats_or_fail(uint32_t num, char buffName[]);
void reset_float_reservations();


void alloc_fail(uint32_t numBytes, char compName[]);
void *alloc_or_fail(uint32_t num, uint32_t size, char compName[]);

dsp_comp *alloc_generic();
void free_generic(dsp_comp *comp);

dsp_comp *alloc_delay(uint32_t delay_size);
void free_delay(dsp_comp *delay);

dsp_comp *alloc_corr(uint32_t corr_size);
void free_corr(dsp_comp *corr);

dsp_comp *alloc_fir(uint32_t fir_size, float *fir_coeffs);
void free_fir(dsp_comp *fir);

dsp_comp *alloc_recorder(uint8_t rec_num, uint32_t rec_size);
void free_recorder(dsp_comp *recorder);

dsp_comp *alloc_player(uint8_t play_num, uint32_t play_size, float **play_buffers);
void free_player(dsp_comp *player);

dsp_comp *alloc_rls(uint32_t rls_size);
void free_rls(dsp_comp *rls);

dsp_comp *alloc_ftf(uint32_t ftf_size);
void free_ftf(dsp_comp *ftf);


#endif
