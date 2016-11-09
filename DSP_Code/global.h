//For the STM32, the interrupt handlers will need access to the controller.

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define INPUT_CHANNELS 2 //Right now this is only used for the previous samples
#define N_HISTORY 32 //Number of previous samples to retain
#define BLOCK_SIZE 8 //Must be less than uint8_t max size because of block_counter.

#include "debug_settings.h"
#include <stdint.h> // for int16_t and int32_t
#include <stdio.h>


typedef void (*eventHandler)(void *);


#define DEBUG_MARK 1

#if DEBUG_MARK == 1

  #if RUNNING_ON_EMBEDDED == 1

	  #include <semihosting.h>

  #endif // RUNNING_ON_EMBEDDED == 1

	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

	#else

	#define DEBUG_PRINTF(...)

#endif

#if RUNNING_ON_EMBEDDED == 1


#include <arm_math.h>
//#include "controller.h"


//extern controller my_cont;


#endif // RUNNING_ON_EMBEDDED == 1

#if RUNNING_ON_EMBEDDED == 0

typedef float q15_t;
#include <math.h>

#endif // RUNNING_ON_EMBEDDED == 0

#include "history.h"

#endif // _GLOBAL_H_
