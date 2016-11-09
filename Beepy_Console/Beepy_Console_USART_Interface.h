#ifndef _BEEPY_CONSOLE_USART_INTERFACE_H_
#define _BEEPY_CONSOLE_USART_INTERFACE_H_

#include "defines.h"
#include "Beepy_Console.h"
#include "tm_stm32_usart.h"
#include "string.h"

static const char Beepy_Console_USART_Log[] = "Log:";
static const char Beepy_Console_USART_Wav[] = "Wav:";
static const char Beepy_Console_USART_Amend[] = "Upd:"; //for updating a wave

typedef enum
{
BCUSART_Ready, //Ready to receive input
BCUSART_Log, //Log: has been received, waiting for message followed by newline char.
BCUSART_Wav, //Wav: has been received, waiting for buffer (A or B) followed by newline char
BCUSART_Wav_Buff, //Wav:\n(A|B)\n has been received, waiting for buffer length, followed by newline char
BCUSART_Wav_Buff_Type,
BCUSART_Wav_Buff_Type_Style,
BCUSART_Wav_Buff_Type_Style_Name,
BCUSART_Wav_Buff_Type_Style_Name_Scale,
BCUSART_Wav_Buff_Type_Style_Name_Scale_Length, //Wav:\n(A|B)\nLENGTH\n has been received, will now receive message
BCUSART_Amend_Now_Get_Buff,
BCUSART_Amend_Have_Buff_Now_Get_Index,
BCUSART_Amend_Have_Buff_Index_Now_Get_Value,

}Beepy_USARTI_State;

typedef enum
{
	Beepy_USARTI_Text,
	Beepy_USARTI_Data
}Beepy_USARTI_Receive_Mode;

typedef struct Beepy_USART_Interface_Struct
{
	char temp_buffer[34];
	uint16_t curr;
	uint16_t data_values_received;
	Beepy_USARTI_State state;
	Beepy_Console_Buffer buffer;
	Beepy_Console_Buffer_Data_Type type;
	Beepy_Console_Buffer_Draw_Mode mode;
	uint32_t length_accumulator;
	uint32_t scale_accumulator;
	f_to_c iface_f_to_c;
	Beepy_USARTI_Receive_Mode receive_mode;
}Beepy_USART_Interface;

static Beepy_USART_Interface interface;

void Beepy_Console_USARTI_Init();

void Beepy_Console_USARTI_Check();

void Beepy_Console_USARTI_Reset_Temp_Buffer();

void Beepy_Console_Error_State_Restart(char c);

//Handlers for the various states
void Beepy_Console_BCUSART_Ready(char c); //Ready to receive input
void Beepy_Console_BCUSART_Log(char c); //Log: has been received, waiting for message followed by newline char.
void Beepy_Console_BCUSART_Wav(char c); //Wav: has been received, waiting for buffer (A or B) followed by newline char
void Beepy_Console_BCUSART_Wav_Buff(char c); //Wav:\n(A|B)\n has been received, waiting for buffer length, followed by newline char
void Beepy_Console_BCUSART_Wav_Buff_Type(char c);
void Beepy_Console_BCUSART_Wav_Buff_Type_Style(char c);
void Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name(char c);
void Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name_Scale(char c);
void Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name_Scale_Length(char c);
void BCUSART_Amend_Now_Get_Buff_f(char c);
void BCUSART_Amend_Have_Buff_Now_Get_Index_f(char c);
void BCUSART_Amend_Have_Buff_Index_Now_Get_Value_f(char c);


#endif // _BEEPY_CONSOLE_USART_INTERFACE_H_
