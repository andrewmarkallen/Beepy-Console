#ifndef __SERLCD_H
#define __SERLCD_H

#include <stdarg.h>
#include <tm_stm32_usart.h>
#include <arm_math.h>

#define SERLCD_USART USART3

void vserprintf(char* format, va_list args);
void serprintf(char* format, ...);
void Log(char* format, ...);
void sendFloat(float x);
void sendWaveform(float* buf, uint16_t num);
void sendWaveformB(float* buf, uint16_t num);


#endif
