#ifndef _DMA_CALLBACK_H_
#define  _DMA_CALLBACK_H_

#include "stm32f4xx_hal.h"
#include "mxconstants.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_it.h"

I2C_HandleTypeDef hi2c3;

I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;

// ST
DMA_HandleTypeDef hdma_i2s2_ext_tx;
DMA_HandleTypeDef hdma_i2s2_ext_rx;
// END ST

DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_i2s3_ext_rx;
DMA_HandleTypeDef hdma_spi3_tx;



#endif
