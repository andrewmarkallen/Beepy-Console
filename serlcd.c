#include "serlcd.h"


void vserprintf(char* format, va_list args)
{
	char msg[33];
	vsprintf(msg, format, args);
	TM_USART_Puts(SERLCD_USART, msg);
}

void serprintf(char* format, ...)
{
	va_list args;
	va_start(args, format);
	vserprintf(format, args);
	va_end(args);
}

void Log(char* format, ...)
{
	TM_USART_Puts(SERLCD_USART, "Log:");
	va_list args;
	va_start(args, format);
	vserprintf(format, args);
	va_end(args);
	//TM_USART_Puts(SERLCD_USART, "\n");
}

void sendFloat(float x)
{
	union {
		float f;
		unsigned char bytes[4];
	} fu;

	fu.f = x;

	uint8_t j;
	for(j=0; j<4; j++)
	{
		TM_USART_Putc(SERLCD_USART, fu.bytes[j]);
	}
}

void sendWaveform(float* buf, uint16_t num)
{

	// Find max value in buffer for scaling
	uint16_t j;
	float maxval = 0.0f;
	for(j=0; j<num; j++)
	{
		maxval = fabs(buf[j]) > maxval ? fabs(buf[j]) : maxval;
	}
	if(maxval == 0.0f)
	{
		maxval = 1.0f;
	}

	// Send metadata
	serprintf("Wav:Afl%.1e\n1\n%d\n", maxval, num);

	// Send correlation vector
	for(j=0; j<num; j++)
	{
		sendFloat(buf[j]/maxval);
	}
}

void sendWaveformB(float* buf, uint16_t num)
{

	// Find max value in buffer for scaling
	uint16_t j;
	float maxval = 0.0f;
	for(j=0; j<num; j++)
	{
		maxval = fabs(buf[j]) > maxval ? fabs(buf[j]) : maxval;
	}
	if(maxval == 0.0f)
	{
		maxval = 1.0f;
	}

	// Send metadata
	serprintf("Wav:Bfl%.1e\n1\n%d\n", maxval, num);

	// Send correlation vector
	for(j=0; j<num; j++)
	{
		sendFloat(buf[j]/maxval);
	}
}
