#include "Beepy_Console.h"

void Beepy_Console_Init(char* text_buffer, Beepy_Console_Wave_Buffer_Union* wave_bufferA, int32_t size_wave_bufferA, Beepy_Console_Wave_Buffer_Union* wave_bufferB, int32_t size_wave_bufferB)
{
	console.current_mode = Beepy_Console_Log_Display_Mode;
	console.bufferA_draw_mode = Beepy_Console_Buffer_Draw_Mode_Line;
	console.bufferB_draw_mode = Beepy_Console_Buffer_Draw_Mode_Line;
	console.text_buffer = text_buffer;
	for(int i = 0; i < CONSOLE_SIZE - 1; i++)
	{
		console.text_buffer[i] = ' ';
	}
	console.text_buffer[CONSOLE_SIZE - 1] = 0;

	console.bufferA = wave_bufferA;
	console.bufferA_max_size = size_wave_bufferA;
	console.bufferB = wave_bufferB;
	console.bufferB_max_size = size_wave_bufferB;

	TM_LCD_Init();
	/* Fill LCD with color */
	TM_LCD_Fill(0x4123);
	/* Put string on the middle of LCD */
	TM_LCD_SetFont(&TM_Font_7x10);
	TM_LCD_SetColors(BEEPY_COLOUR_A, BEEPY_COLOUR_B);
	console.redraw = 1;
	console.bufferA_name[0] = 0;
	console.bufferB_name[0] = 0;
	console.bufferA_name_length = 0;
	console.bufferB_name_length = 0;
}

void Beepy_Console_Set_Mode(Beepy_Console_Mode mode)
{
	console.redraw = 1;
	console.current_mode = mode;
}

void Beepy_Console_Set_Buffer_Draw_Mode(Beepy_Console_Buffer buffer, Beepy_Console_Buffer_Draw_Mode mode)
{
	if(buffer == Beepy_Console_Buffer_A)
	{
		console.bufferA_draw_mode = mode;
	}
	else if(buffer == Beepy_Console_Buffer_B)
	{
		console.bufferB_draw_mode = mode;
	}
	Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Buffer_Display_Mode);
	Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Dual_Buffer_Display_Mode);
}

void Beepy_Console_Set_Buffer(Beepy_Console_Buffer buffer_choice, Beepy_Console_Wave_Buffer_Union* buffer, Beepy_Console_Buffer_Data_Type type, uint32_t max_size, uint16_t length, Beepy_Console_Buffer_Draw_Mode mode, char* name)
{
	if(buffer_choice == Beepy_Console_Buffer_A)
	{
		console.bufferA = buffer;
		console.bufferA_max_size = max_size;
		console.bufferA_length = length;
		console.bufferA_type = type;
		console.bufferA_draw_mode = mode;
		int  i = 0;
		while(name[i] != 0 && i < 16)
		{
			console.bufferA_name[i] =  name[i];
			i++;
		}
		console.bufferA_name[i] = 0;
		console.bufferA_name_length = i;
	}
	else if(buffer_choice == Beepy_Console_Buffer_B)
	{
		console.bufferB = buffer;
		console.bufferB_max_size = max_size;
		console.bufferB_length = length;
		console.bufferB_type = type;
		console.bufferB_draw_mode = mode;
		int  i = 0;
		while(name[i] != 0 && i < 16)
		{
			console.bufferB_name[i] =  name[i];
			i++;
		}
		console.bufferB_name[i] = 0;
		console.bufferB_name_length = i;
	}
	Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Buffer_Display_Mode);
	Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Dual_Buffer_Display_Mode);
}

void Beepy_Console_Reset_Buffer(Beepy_Console_Buffer buffer_choice, Beepy_Console_Buffer_Data_Type type, uint32_t max_size, uint16_t length, Beepy_Console_Buffer_Draw_Mode mode, char* name)
{
	if(buffer_choice == Beepy_Console_Buffer_A)
		{
			console.bufferA_max_size = max_size;
			console.bufferA_length = length;
			console.bufferA_type = type;
			console.bufferA_draw_mode = mode;
			int  i = 0;
			while(name[i] != 0 && i < 16)
			{
				console.bufferA_name[i] =  name[i];
				i++;
			}
			console.bufferA_name[i] = 0;
			console.bufferA_name_length = i;
		}
		else if(buffer_choice == Beepy_Console_Buffer_B)
		{
			console.bufferB_max_size = max_size;
			console.bufferB_length = length;
			console.bufferB_type = type;
			console.bufferB_draw_mode = mode;
			int  i = 0;
			while(name[i] != 0 && i < 16)
			{
				console.bufferB_name[i] =  name[i];
				i++;
			}
			console.bufferB_name[i] = 0;
			console.bufferB_name_length = i;
		}
}

void Beepy_Console_Draw_Buffer()
{
	TM_LCD_SetOrientation(2);
	TM_LCD_Fill(BEEPY_COLOUR_B);
	TM_LCD_DrawRectangle(5,5, 311, 231, BEEPY_COLOUR_A);
	if(Beepy_Console_Type_Is_Signed(Beepy_Console_Buffer_A))
	{
		TM_LCD_DrawLine(5,120, 315, 120, BEEPY_COLOUR_A);
	}

	//If the number is unsigned, 0 Y-value is at Y = 230 in LCD coordinates.
	//If the number is signed however, it is half-way up the screen, i.e. Y = 115 = 230 - signed_offset
	uint16_t signed_offset = 0;
	if(console.bufferA_type == Beepy_Console_Int_32 || console.bufferA_type == Beepy_Console_Float_32)
	{
		signed_offset =  115;
	}
	float ratio = ((float)console.bufferA_length)/310;
	float j = 0;
	int j_2 = 0;
	for(int i = 0; i < 310; i++)
	{
		j = ratio * i;
		j_2 = (int)j;
		while(j_2 < ratio*(i+1))
		{
			int16_t offset = Beepy_Console_Get_Buffer_Offset(j_2);
			if(console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Dot)
			{
				TM_LCD_DrawPixel(i+5, 235 - offset, BEEPY_COLOUR_A);
			}
			else if (console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Line)
			{
				TM_LCD_DrawLine(i+5, 235-signed_offset, i+5,235 - offset, BEEPY_COLOUR_A);
			}
			else
			{
				Beepy_Console_Add_Line("unsupported draw mode!");
				Beepy_Console_Set_Mode(Beepy_Console_Log_Display_Mode);
			}
			j_2++;
		}
	}
	TM_LCD_SetXY(315 - (console.bufferA_name_length*7 + (17*7)),5);
	TM_LCD_Puts(console.bufferA_name);
	TM_LCD_SetXY(315 - (16*7),5);
	char str[16];
	xsprintf(str, "Y:%d, N:%d", console.bufferA_max_size, console.bufferA_length);
	TM_LCD_Puts(str);
}

int16_t Beepy_Console_Get_Buffer_Offset(uint32_t index)
{
	float offset = Beepy_Console_Get_Float_Buffer_Offset(Beepy_Console_Buffer_A, index);
	offset = 230 * offset;
	int16_t uint_offset = (int16_t)offset;
	return uint_offset;
}

float Beepy_Console_Get_Float_Buffer_Offset(Beepy_Console_Buffer buffer_name, uint32_t index)
{
	Beepy_Console_Wave_Buffer_Union* buffer;
	Beepy_Console_Buffer_Data_Type type;
	float max_size = 0;

	if(buffer_name == Beepy_Console_Buffer_A)
	{
		type = console.bufferA_type;
		buffer = console.bufferA;
		max_size = console.bufferA_max_size;
	}
	else if(buffer_name == Beepy_Console_Buffer_B)
	{
		type = console.bufferB_type;
		buffer = console.bufferB;
		max_size = console.bufferB_max_size;
	}
	else
	{
		Beepy_Console_Add_Line("unsupported buffer");
		return 0;
	}
	float value = 0;
	float offset = 0;
	switch(type)
	{
	case Beepy_Console_Float_32:
		value = (float)buffer[index].f;
		value = value/2;
		offset = 0.5;
		break;
	case Beepy_Console_UInt_32:
		value = (float)buffer[index].u;
		offset = 0;
		break;
	case Beepy_Console_Int_32:
		value = (float)buffer[index].i;
		value = value/2;
		offset = 0.5;
		break;
	case Beepy_Console_Char_8:
		value = (float)buffer[index].i;
		offset = 0;
		break;
	}
	//Suppose we have floats. The first element has value -512. Last has 512.
	//Max_value is taken as 512.
	//Then value/max = -0.5 , 0.5 for first , last.
	//Then we take 0.5 + these; which gives us 0,1.

	//Alternatively supposed we have unsigned ints. First has value 0, last has 1024.
	//Again assume Max_value is 1024.
	//Then first, last: 0, 1024
	//value/max



	value = (value/max_size);
	value = offset + value;
	return value;
}

int16_t Beepy_Console_Get_Buffer_A_Offset(uint32_t index)
{
	return (int16_t) 110 * Beepy_Console_Get_Float_Buffer_Offset(Beepy_Console_Buffer_A, index);
}

int16_t Beepy_Console_Get_Buffer_B_Offset(uint32_t index)
{
	return (int16_t) 110 * Beepy_Console_Get_Float_Buffer_Offset(Beepy_Console_Buffer_B, index);
}

void Beepy_Console_Draw_Logs()
{
	TM_LCD_SetOrientation(1);
	TM_LCD_SetXY(0,0);
	TM_LCD_Puts(console.text_buffer);
}

void Beepy_Console_Draw_Dual_Buffer()
{
	TM_LCD_SetOrientation(2);
	TM_LCD_Fill(BEEPY_COLOUR_B);
	TM_LCD_DrawRectangle(5,5, 311, 111, BEEPY_COLOUR_A);
	if(Beepy_Console_Type_Is_Signed(Beepy_Console_Buffer_A))
	{
		TM_LCD_DrawLine(5,60, 315, 60, BEEPY_COLOUR_A);
	}
	TM_LCD_DrawRectangle(5,125, 311, 111, BEEPY_COLOUR_A);
	if(Beepy_Console_Type_Is_Signed(Beepy_Console_Buffer_B))
	{
		TM_LCD_DrawLine(5,180, 315, 180, BEEPY_COLOUR_A);
	}

	//If the number is unsigned, 0 Y-value is at Y = 115 in LCD coordinates.
	//If the number is signed however, it is half-way up the screen, i.e. Y = 60 = 115 - signed_offset
	uint16_t signed_offset = 0;
	if(console.bufferA_type == Beepy_Console_Int_32 || console.bufferA_type == Beepy_Console_Float_32)
	{
		signed_offset =  55;
	}
	float ratio = ((float)console.bufferA_length)/310;
	float j = 0;
	int j_2 = 0;
	for(int i = 0; i < 310; i++)
	{
		j = ratio * i;
		j_2 = (int)j;
		while(j_2 < ratio*(i+1))
		{
			int16_t offset = Beepy_Console_Get_Buffer_A_Offset(j_2);
			if(console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Dot)
			{
				TM_LCD_DrawPixel(i+5, 115 - offset, BEEPY_COLOUR_A);
			}
			else if (console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Line)
			{
				TM_LCD_DrawLine(i+5, 115-signed_offset, i+5,115 - offset, BEEPY_COLOUR_A);
			}
			else
			{
				Beepy_Console_Add_Line("unsupported draw mode!");
				Beepy_Console_Set_Mode(Beepy_Console_Log_Display_Mode);
			}
			j_2++;
		}
	}

	//Write the information for buffer A
	TM_LCD_SetXY(315 - (console.bufferA_name_length*7 + (17*7)),5);
	TM_LCD_Puts(console.bufferA_name);
	TM_LCD_SetXY(315 - (16*7),5);
	char str[16];
	xsprintf(str, "Y:%d, N:%d", console.bufferA_max_size, console.bufferA_length);
	TM_LCD_Puts(str);


	//Buffer B

	signed_offset = 0;
	if(console.bufferB_type == Beepy_Console_Int_32 || console.bufferB_type == Beepy_Console_Float_32)
	{
		signed_offset =  0;
	}
	ratio = ((float)console.bufferB_length)/310;
	j = 0;
	j_2 = 0;
	for(int i = 0; i < 310; i++)
	{
		j = ratio * i;
		j_2 = (int)j;
		while(j_2 < ratio*(i+1))
		{
			int16_t offset = Beepy_Console_Get_Buffer_B_Offset(j_2);
			if(console.bufferB_draw_mode == Beepy_Console_Buffer_Draw_Mode_Dot)
			{
				TM_LCD_DrawPixel(i+5, 235 - offset, BEEPY_COLOUR_A);
			}
			else if (console.bufferB_draw_mode == Beepy_Console_Buffer_Draw_Mode_Line)
			{
				TM_LCD_DrawLine(i+5, 235-signed_offset, i+5,235 - offset, BEEPY_COLOUR_A);
			}
			else
			{
				Beepy_Console_Add_Line("unsupported draw mode!");
				Beepy_Console_Set_Mode(Beepy_Console_Log_Display_Mode);
			}
			j_2++;
		}
	}
	TM_LCD_SetXY(315 - (console.bufferB_name_length*7 + (17*7)),125);
	TM_LCD_Puts(console.bufferB_name);
	TM_LCD_SetXY(315 - (16*7),125);
	xsprintf(str, "Y:%d, N:%d", console.bufferB_max_size, console.bufferB_length);
	TM_LCD_Puts(str);
}

void Beepy_Console_Draw()
{
	if(console.redraw == 1)
	{
		switch(console.current_mode)
		{
		case Beepy_Console_Buffer_Display_Mode:
			Beepy_Console_Draw_Buffer();
			break;

		case Beepy_Console_Dual_Buffer_Display_Mode:
			Beepy_Console_Draw_Dual_Buffer();
			break;

		case Beepy_Console_Log_Display_Mode:
			Beepy_Console_Draw_Logs();
		}
	}
	console.redraw = 0;
}

void Beepy_Console_Redraw_Buffer(uint16_t index)
{
	uint16_t signed_offset = 0;
	if(console.bufferA_type == Beepy_Console_Int_32 || console.bufferA_type == Beepy_Console_Float_32)
	{
		signed_offset =  115;
	}
	float ratio = ((float)console.bufferA_length)/310;
	float j = 0;
	int j_2 = 0;
	for(int i = 0; i < 310; i++)
	{
		j = ratio * i;
		j_2 = (int)j;
		if((j_2 <= index) && (index < ratio*(i+1)))
		{
			while(j_2 < ratio*(i+1))
			{
				int16_t offset = Beepy_Console_Get_Buffer_Offset(j_2);
				if(console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Dot)
				{
					TM_LCD_DrawPixel(i+5, 235 - offset, BEEPY_COLOUR_A);
				}
				else if (console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Line)
				{
					TM_LCD_DrawLine(i+5, 235-signed_offset, i+5,235 - offset, BEEPY_COLOUR_A);
				}
				else
				{
					Beepy_Console_Add_Line("unsupported draw mode!");
					Beepy_Console_Set_Mode(Beepy_Console_Log_Display_Mode);
				}
				j_2++;
			}
		}
	}
}

void Beepy_Console_Redraw_Dual_Buffer_A(uint16_t index)
{
	//If the number is unsigned, 0 Y-value is at Y = 115 in LCD coordinates.
	//If the number is signed however, it is half-way up the screen, i.e. Y = 60 = 115 - signed_offset
	uint16_t signed_offset = 0;
	if(console.bufferA_type == Beepy_Console_Int_32 || console.bufferA_type == Beepy_Console_Float_32)
	{
		signed_offset =  55;
	}
	float ratio = ((float)console.bufferA_length)/310;
	float j = 0;
	int j_2 = 0;
	for(int i = 0; i < 310; i++)
	{
		j = ratio * i;
		j_2 = (int)j;
		if((j_2 <= index) && (index < ratio*(i+1)))
		{
			while(j_2 < ratio*(i+1))
			{
				int16_t offset = Beepy_Console_Get_Buffer_A_Offset(j_2);
				if(console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Dot)
				{
					TM_LCD_DrawPixel(i+5, 115 - offset, BEEPY_COLOUR_A);
				}
				else if (console.bufferA_draw_mode == Beepy_Console_Buffer_Draw_Mode_Line)
				{
					TM_LCD_DrawLine(i+5, 115-signed_offset, i+5,115 - offset, BEEPY_COLOUR_A);
				}
				else
				{
					Beepy_Console_Add_Line("unsupported draw mode!");
					Beepy_Console_Set_Mode(Beepy_Console_Log_Display_Mode);
				}
				j_2++;
			}
		}
	}

}

void Beepy_Console_Redraw_Dual_Buffer_B(uint16_t index)
{
	//If the number is unsigned, 0 Y-value is at Y = 115 in LCD coordinates.
	//If the number is signed however, it is half-way up the screen, i.e. Y = 60 = 115 - signed_offset
	uint16_t signed_offset = 0;
	if(console.bufferB_type == Beepy_Console_Int_32 || console.bufferB_type == Beepy_Console_Float_32)
	{
		signed_offset =  55;
	}
	float ratio = ((float)console.bufferB_length)/310;
	float j = 0;
	int j_2 = 0;
	for(int i = 0; i < 310; i++)
	{
		j = ratio * i;
		j_2 = (int)j;
		if((j_2 <= index) && (index < ratio*(i+1)))
		{
			while(j_2 < ratio*(i+1))
			{
				int16_t offset = Beepy_Console_Get_Buffer_B_Offset(j_2);
				if(console.bufferB_draw_mode == Beepy_Console_Buffer_Draw_Mode_Dot)
				{
					TM_LCD_DrawPixel(i+5, 235 - offset, BEEPY_COLOUR_A);
				}
				else if (console.bufferB_draw_mode == Beepy_Console_Buffer_Draw_Mode_Line)
				{
					TM_LCD_DrawLine(i+5, 235-signed_offset, i+5,235 - offset, BEEPY_COLOUR_A);
				}
				else
				{
					Beepy_Console_Add_Line("unsupported draw mode!");
					Beepy_Console_Set_Mode(Beepy_Console_Log_Display_Mode);
				}
				j_2++;
			}
		}
	}

}

void Beepy_Console_Add_Char(char c)
{
	char line[] = "_";
	line[0] = c;
	Beepy_Console_Add_Line(line);
}


void Beepy_Console_Add_Line(char* newLine)
{
	//34 characters = width of screen at 7 x 10

	//For now I just truncate if it's more than 34 characters long.

	//For now just do things as simply as possible...move everything up 34 characters.

	//Pad remainder of blank lines with spaces.
	for(int i=0; i < CONSOLE_SIZE - 34; i++)
	{
		console.text_buffer[i] = console.text_buffer[i + 34];
	}
	uint8_t end_of_string_reached = 0;
	for(int i=0; i < 34; i++)
	{
		if(newLine[i] == 0)
		{
			end_of_string_reached = 1;
		}
		if(end_of_string_reached != 1)
		{
			console.text_buffer[(CONSOLE_SIZE - 35) + i] = newLine[i];
		}
		else
		{
			console.text_buffer[(CONSOLE_SIZE - 35) + i] = ' ';
		}
	}
	console.text_buffer[CONSOLE_SIZE - 1] = 0;

	Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Log_Display_Mode);
}

void Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Mode mode)
{
	if(mode == console.current_mode)
	{
		console.redraw = 1;
	}
}

uint8_t Beepy_Console_Type_Is_Signed(Beepy_Console_Buffer buffer)
{
	Beepy_Console_Buffer_Data_Type type;
	if (buffer == Beepy_Console_Buffer_A)
	{
		type = console.bufferA_type;
	}
	else if (buffer == Beepy_Console_Buffer_B)
	{
		type = console.bufferB_type;
	}

	if(type == Beepy_Console_Float_32 || type == Beepy_Console_Int_32)
	{
		return 1;
	}
	return 0;
}

uint8_t Beepy_Console_Update_Buffer(Beepy_Console_Buffer buffer_choice, uint16_t index, Beepy_Console_Buffer_Data_Type type, Beepy_Console_Wave_Buffer_Union data)
{
	if(buffer_choice == Beepy_Console_Buffer_A && index < console.bufferA_length)
	{
		if(type == Beepy_Console_Float_32)
		{
			console.bufferA[index].f = data.f;
		}
		else if(type == Beepy_Console_UInt_32)
		{
			console.bufferA[index].u = data.u;
		}
		else if(type == Beepy_Console_Int_32)
		{
			console.bufferA[index].i = data.i;
		}
		else if(type == Beepy_Console_Char_8)
		{
			console.bufferA[index].c = data.c;
		}
	}
	else if(buffer_choice == Beepy_Console_Buffer_B && index < console.bufferB_length)
	{
		if(type == Beepy_Console_Float_32)
		{
			console.bufferB[index].f = data.f;
		}
		else if(type == Beepy_Console_UInt_32)
		{
			console.bufferB[index].u = data.u;
		}
		else if(type == Beepy_Console_Int_32)
		{
			console.bufferB[index].i = data.i;
		}
		else if(type == Beepy_Console_Char_8)
		{
			console.bufferB[index].c = data.c;
		}
	}
	else
	{
		Beepy_Console_Add_Line("Something went wrong in update buffer!");
		return 0;
	}
}

//size is number of 16 bit values.
void split16_to_8(uint16_t* l, char* r, uint32_t size)
{
	for(int i = 0; i < size; i++)
	{
		uint16_t a = l[i] & 0xff;
		uint16_t b =  (l[i] >> 8) & 0xff;
		char c = a;
		char d = b;
		r[2*i] = c;
		r[(2*i)+1] =d;
	}
}

//size is number of 32 bit values.
void split32_to_8(uint32_t* l, char* r, uint32_t size)
{
	for(int i = 0; i < size; i++)
	{
		r[(4*i)] = l[i] & 0xff;
		r[(4*i)+1] = (l[i] >> 8) & 0xff;
		r[(4*i)+2] = (l[i] >> 16) & 0xff;
		r[(4*i)+3] = (l[i] >> 24) & 0xff;
	}
}

uint16_t combine8to16(char* r)
{
	uint16_t l = r[0] + (r[1] << 8);
	return l;
}

uint32_t combine8to32(char* r)
{
	uint32_t l = r[0] + (r[1] << 8) + (r[2] << 16) + (r[3] << 24);
	return l;
}
