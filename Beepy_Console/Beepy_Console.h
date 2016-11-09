#ifndef _BEEPY_CONSOLE_H_
#define _BEEPY_CONSOLE_H_

#include "tm_stm32_lcd.h"
#include "defines.h"
#include "xprintf.h"

#define CONSOLE_SIZE 1089 // (34W * 32H) + 1(null terminator)
#define BEEPY_COLOUR_A 0xFF0B
#define BEEPY_COLOUR_B 0x039A

typedef union
{
	float f;
	char bytes[sizeof(float)];
}f_to_c;

typedef union Beepy_Console_Wave_Buffer_Union_Type
{
	uint32_t u;
	int32_t i;
	float f;
	char c;
} Beepy_Console_Wave_Buffer_Union;

typedef enum Beepy_Console_Buffer
{
	Beepy_Console_Buffer_A,
	Beepy_Console_Buffer_B
}Beepy_Console_Buffer;

typedef enum Beepy_Console_Mode
{
	Beepy_Console_Buffer_Display_Mode,
	Beepy_Console_Dual_Buffer_Display_Mode,
	Beepy_Console_Log_Display_Mode
}Beepy_Console_Mode;

typedef enum Beepy_Console_Buffer_Draw_Mode
{
	Beepy_Console_Buffer_Draw_Mode_Line,
	Beepy_Console_Buffer_Draw_Mode_Dot
}Beepy_Console_Buffer_Draw_Mode;

typedef enum Beepy_Console_Buffer_Data_Type
{
	Beepy_Console_Float_32,
	Beepy_Console_UInt_32,
	Beepy_Console_Int_32,
	Beepy_Console_Char_8
}Beepy_Console_Buffer_Data_Type;

typedef struct Beepy_Console_Struct
{
	Beepy_Console_Wave_Buffer_Union* bufferA;
	Beepy_Console_Buffer_Data_Type bufferA_type;
	uint32_t bufferA_length;
	uint32_t bufferA_max_size;
	Beepy_Console_Wave_Buffer_Union* bufferB;
	Beepy_Console_Buffer_Data_Type bufferB_type;
	uint32_t bufferB_length;
	uint32_t bufferB_max_size;
	char* text_buffer; // This must be at least 34 wide * 32 high = 1088+1 bytes.
	Beepy_Console_Mode current_mode;
	Beepy_Console_Buffer_Draw_Mode bufferA_draw_mode;
	Beepy_Console_Buffer_Draw_Mode bufferB_draw_mode;
	uint8_t bufferA_active;
	uint8_t bufferB_active;
	char bufferA_name[17];
	char bufferB_name[17];
	uint8_t bufferA_name_length;
	uint8_t bufferB_name_length;
	uint8_t redraw;

}Beepy_Console_Struct;

static Beepy_Console_Struct console;

void Beepy_Console_Init(char* text_buffer, Beepy_Console_Wave_Buffer_Union* wave_bufferA, int32_t size_wave_bufferA, Beepy_Console_Wave_Buffer_Union* wave_bufferB, int32_t size_wave_bufferB);

void Beepy_Console_Set_Mode(Beepy_Console_Mode mode);

void Beepy_Console_Set_Buffer_Draw_Mode(Beepy_Console_Buffer buffer, Beepy_Console_Buffer_Draw_Mode mode);

void Beepy_Console_Set_Buffer(Beepy_Console_Buffer buffer_choice, Beepy_Console_Wave_Buffer_Union* buffer, Beepy_Console_Buffer_Data_Type type, uint32_t max_size, uint16_t length, Beepy_Console_Buffer_Draw_Mode mode, char* name);

void Beepy_Console_Reset_Buffer(Beepy_Console_Buffer buffer_choice, Beepy_Console_Buffer_Data_Type type, uint32_t max_size, uint16_t length, Beepy_Console_Buffer_Draw_Mode mode, char* name);

void Beepy_Console_Draw_Buffer();

int16_t Beepy_Console_Get_Buffer_Offset(uint32_t index);

//For internal use
float Beepy_Console_Get_Float_Buffer_Offset(Beepy_Console_Buffer buffer_name, uint32_t index);

int16_t Beepy_Console_Get_Buffer_A_Offset(uint32_t index);

int16_t Beepy_Console_Get_Buffer_B_Offset(uint32_t index);

void Beepy_Console_Draw_Logs();

void Beepy_Console_Draw_Dual_Buffer();

void Beepy_Console_Draw();

void Beepy_Console_Redraw_Buffer(uint16_t index);

void Beepy_Console_Redraw_Dual_Buffer_A(uint16_t index);

void Beepy_Console_Redraw_Dual_Buffer_B(uint16_t index);

void Beepy_Console_Add_Char(char c);

void Beepy_Console_Add_Line(char* newLine);

void Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Mode mode);

uint8_t Beepy_Console_Type_Is_Signed(Beepy_Console_Buffer buffer);

uint8_t Beepy_Console__Buffer(Beepy_Console_Buffer buffer, uint16_t index, Beepy_Console_Buffer_Data_Type type, Beepy_Console_Wave_Buffer_Union data);

void split16_to_8(uint16_t* l, char* r, uint32_t size);
void split32_to_8(uint32_t* l, char* r, uint32_t size);
uint16_t combine8to16(char* r);
uint32_t combine8to32(char* r);

#endif _BEEPY_CONSOLE_H_
