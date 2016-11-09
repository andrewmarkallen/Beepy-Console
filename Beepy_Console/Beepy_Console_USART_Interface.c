#include "Beepy_Console_USART_Interface.h"

void Beepy_Console_USARTI_Init()
{
	TM_USART_Init(USART3, TM_USART_PinsPack_2, (19200*9600)/3072); //'Fixes' error from conflict with LCD clock set-up, fix this properly!
	interface.state = BCUSART_Ready;
	interface.curr = 0;
	interface.length_accumulator = 0;
	interface.scale_accumulator = 0;
	interface.data_values_received = 0;
	interface.receive_mode = Beepy_USARTI_Text;
}

void Beepy_Console_USARTI_Check()
{
	char strc[3];
	char c;
	uint8_t data_characters_received = 0;
	uint8_t done = 0;
	while(!done)
	{
		if(interface.receive_mode == Beepy_USARTI_Text)
		{
			c = TM_USART_Getc(USART3);
		}
		else if(interface.receive_mode == Beepy_USARTI_Data)
		{
			data_characters_received = 0;
			if(TM_USART_BufferEmpty(USART3))
			{

			}
			else
			{
				c = TM_USART_Getc(USART3);
				data_characters_received = 1;
			}
		}
		if((interface.receive_mode == Beepy_USARTI_Text && c) || (((interface.receive_mode == Beepy_USARTI_Data) && data_characters_received)))
		{
			//Beepy_Console_Add_Char(c);
			done = 0;
			//Simple way to reset interface over USART if stuck is send '*' character.
			if((interface.state != BCUSART_Wav_Buff_Type_Style_Name_Scale_Length) && (c == '*'))
			{
				Beepy_Console_Error_State_Restart(c);
				Beepy_Console_Add_Line("USARTI Reset Called!");
			}
			else
			{
				switch(interface.state)
				{
				case BCUSART_Ready:
					Beepy_Console_BCUSART_Ready(c);
					break;
				case BCUSART_Log:
					Beepy_Console_BCUSART_Log(c);
					break;
				case BCUSART_Wav:
					Beepy_Console_BCUSART_Wav(c);
					break;
				case BCUSART_Wav_Buff:
					Beepy_Console_BCUSART_Wav_Buff(c);
					break;
				case BCUSART_Wav_Buff_Type:
					Beepy_Console_BCUSART_Wav_Buff_Type(c);
					break;
				case BCUSART_Wav_Buff_Type_Style:
					Beepy_Console_BCUSART_Wav_Buff_Type_Style(c);
					break;
				case BCUSART_Wav_Buff_Type_Style_Name:
					Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name(c);
					break;
				case BCUSART_Wav_Buff_Type_Style_Name_Scale:
					Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name_Scale(c);
					break;
				case BCUSART_Wav_Buff_Type_Style_Name_Scale_Length:
					Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name_Scale_Length(c);
					break;
				case BCUSART_Amend_Now_Get_Buff:
					BCUSART_Amend_Now_Get_Buff_f(c);
					break;
				case BCUSART_Amend_Have_Buff_Now_Get_Index:
					BCUSART_Amend_Have_Buff_Now_Get_Index_f(c);
					break;
				case BCUSART_Amend_Have_Buff_Index_Now_Get_Value:
					BCUSART_Amend_Have_Buff_Index_Now_Get_Value_f(c);
					break;

				}
			}
		}
		else
		{
			done = 1;
		}
	}
}

void Beepy_Console_BCUSART_Ready(char c)
{
	if(interface.curr < 4)
	{
		if(c == Beepy_Console_USART_Log[interface.curr] || c == Beepy_Console_USART_Wav[interface.curr] || c == Beepy_Console_USART_Amend[interface.curr])
		{
			interface.temp_buffer[interface.curr] = c;
		}
		else
		{
			//Invalid code received
			Beepy_Console_Error_State_Restart(c);
		}
		interface.curr++;
		if(interface.curr == 4)
		{
			if(strcmp(interface.temp_buffer, Beepy_Console_USART_Log) == 0)
			{
				interface.state = BCUSART_Log;
				Beepy_Console_USARTI_Reset_Temp_Buffer();
			}
			else if(strcmp(interface.temp_buffer, Beepy_Console_USART_Wav) == 0)
			{
				interface.state = BCUSART_Wav;
				Beepy_Console_USARTI_Reset_Temp_Buffer();
			}
			else if(strcmp(interface.temp_buffer, Beepy_Console_USART_Amend) == 0)
			{
				interface.state = BCUSART_Amend_Now_Get_Buff;
				Beepy_Console_USARTI_Reset_Temp_Buffer();
			}
			else //invalid code received
			{
				Beepy_Console_Error_State_Restart(c);
			}
		}
	}
}


void Beepy_Console_BCUSART_Log(char c)
{
	if(c == '\n')
	{
		interface.state = BCUSART_Ready;
		Beepy_Console_Add_Line(interface.temp_buffer);
		Beepy_Console_USARTI_Reset_Temp_Buffer();
	}
	else if(interface.curr < 34)
	{
		interface.temp_buffer[interface.curr] = c;
		interface.curr++;
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

void Beepy_Console_BCUSART_Wav(char c)
{
	if(c == 'A')
	{
		interface.buffer = Beepy_Console_Buffer_A;
		interface.state = BCUSART_Wav_Buff;
	}
	else if(c == 'B')
	{
		interface.buffer = Beepy_Console_Buffer_B;
		interface.state = BCUSART_Wav_Buff;
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

void Beepy_Console_BCUSART_Wav_Buff(char c)
{
	if(c == 'f')
	{
		interface.type = Beepy_Console_Float_32;
		interface.state = BCUSART_Wav_Buff_Type;
	}
	else if(c == 'u')
	{
		interface.type = Beepy_Console_UInt_32;
		interface.state = BCUSART_Wav_Buff_Type;
	}
	else if(c == 's')
	{
		interface.type = Beepy_Console_Int_32;
		interface.state = BCUSART_Wav_Buff_Type;

	}
	else if(c == 'c')
	{
		interface.type = Beepy_Console_Char_8;
		interface.state = BCUSART_Wav_Buff_Type;
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

void Beepy_Console_BCUSART_Wav_Buff_Type(char c)
{
	if(c == 'd')
	{
		interface.mode = Beepy_Console_Buffer_Draw_Mode_Dot;
		interface.state = BCUSART_Wav_Buff_Type_Style;
	}
	else if(c == 'l')
	{
		interface.mode = Beepy_Console_Buffer_Draw_Mode_Line;
		interface.state = BCUSART_Wav_Buff_Type_Style;
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

//We have the style, now we try and get the name
void Beepy_Console_BCUSART_Wav_Buff_Type_Style(char c)
{
	if(c == '\n')
	{
		interface.state = BCUSART_Wav_Buff_Type_Style_Name;
		interface.curr = 0;
		Beepy_Console_Add_Line(interface.temp_buffer);
	}
	else if(interface.curr < 16)
	{
		interface.temp_buffer[interface.curr] = c;
		interface.curr++;
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}
//We have the name, now we try to get scale.
void Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name(char c)
{
	if(c == '\n')
	{
		//We have length, we now try and get data.
		interface.curr = 0;
		interface.state = BCUSART_Wav_Buff_Type_Style_Name_Scale;
	}
	else if(interface.curr < 8)
	{
		//Convert '0' to 0, '1' to 1, etc.
		int16_t temp_val = c - '0';
		if(temp_val < 0 || temp_val > 9)
		{
			Beepy_Console_Error_State_Restart(c);
		}
		else
		{
			interface.scale_accumulator *= 10;
			interface.scale_accumulator += temp_val;
			interface.curr++;
		}
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

//We have the scale, now we try and get the length.
void Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name_Scale(char c)
{
	if(c == '\n')
	{
		//We have length, we now try and get data.
		interface.curr = 0;
		interface.state = BCUSART_Wav_Buff_Type_Style_Name_Scale_Length;
		interface.receive_mode = Beepy_USARTI_Data;
		Beepy_Console_Reset_Buffer(interface.buffer, interface.type, interface.scale_accumulator, interface.length_accumulator, interface.mode, interface.temp_buffer);
	}
	else if(interface.curr < 4)
	{
		//Convert '0' to 0, '1' to 1, etc.
		int16_t temp_val = c - '0';
		if(temp_val < 0 || temp_val > 9)
		{
			Beepy_Console_Error_State_Restart(c);
		}
		else
		{
			interface.length_accumulator *= 10;
			interface.length_accumulator += temp_val;
			interface.curr++;
		}
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

void Beepy_Console_BCUSART_Wav_Buff_Type_Style_Name_Scale_Length(char c)
{
	Beepy_Console_Wave_Buffer_Union u;
	if(interface.type == Beepy_Console_Float_32)
	{
		//interface.temp_buffer[interface.curr] = c;
		interface.iface_f_to_c.bytes[interface.curr] = c;
		if(interface.curr == 3)
		{
			float f_val = interface.iface_f_to_c.f;
			u.f = f_val;
			Beepy_Console_Update_Buffer(interface.buffer, interface.data_values_received, interface.type, u);
			interface.data_values_received++;
			interface.curr = 0;
			if(interface.data_values_received >= interface.length_accumulator)
			{
				//We are done
				Beepy_Console_Add_Line("all values received!");
				Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Log_Display_Mode);
				Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Buffer_Display_Mode);
				Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Dual_Buffer_Display_Mode);
				Beepy_Console_State_Restart();
				return;
			}
		}
		else
		{
			interface.curr++;
		}
	}
	else if(interface.type == Beepy_Console_UInt_32)
	{
		interface.temp_buffer[interface.curr] = c;
		if(interface.curr == 3)
		{
			uint32_t u_val = combine8to32(interface.temp_buffer);
			u.u = u_val;
			Beepy_Console_Update_Buffer(interface.buffer, interface.data_values_received, interface.type, u);
			interface.data_values_received++;
			interface.curr = 0;
		}
		else
		{
			interface.curr++;
		}
	}
	else if(interface.type == Beepy_Console_Int_32)
	{
		interface.temp_buffer[interface.curr] = c;
		if(interface.curr == 3)
		{
			int32_t i_val = combine8to32(interface.temp_buffer);
			u.i =i_val;
			Beepy_Console_Update_Buffer(interface.buffer, interface.data_values_received, interface.type, u);
			interface.data_values_received++;
			interface.curr = 0;
		}
		else
		{
			interface.curr++;
		}
	}
	else if(interface.type == Beepy_Console_Char_8)
	{
		u.c = c;
		Beepy_Console_Update_Buffer(interface.buffer, interface.data_values_received, interface.type, u);
		interface.data_values_received++;
	}
	if(interface.data_values_received >= interface.length_accumulator)
	{
		//We are done
		Beepy_Console_Add_Line("all values received!");
		Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Log_Display_Mode);
		Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Buffer_Display_Mode);
		Beepy_Console_Redraw_If_Current_Mode(Beepy_Console_Dual_Buffer_Display_Mode);
		Beepy_Console_State_Restart();
	}
	//Right now does nothing
}

void BCUSART_Amend_Now_Get_Buff_f(char c)
{
	if(c == 'A')
	{
		interface.buffer = Beepy_Console_Buffer_A;
		interface.state = BCUSART_Amend_Have_Buff_Now_Get_Index;
	}
	else if(c == 'B')
	{
		interface.buffer = Beepy_Console_Buffer_B;
		interface.state = BCUSART_Amend_Have_Buff_Now_Get_Index;
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

void BCUSART_Amend_Have_Buff_Now_Get_Index_f(char c)
{
	if(c == '\n')
	{
		//We have the index, onto next step.
		interface.curr = 0;
		interface.receive_mode = Beepy_USARTI_Data;
		interface.state = BCUSART_Amend_Have_Buff_Index_Now_Get_Value;
	}
	else if(interface.curr < 5)
	{
		//Convert '0' to 0, '1' to 1, etc.
		int16_t temp_val = c - '0';
		if(temp_val < 0 || temp_val > 9)
		{
			Beepy_Console_Error_State_Restart(c);
		}
		else
		{
			//Repurposing the scale accumulator for this.
			interface.scale_accumulator *= 10;
			interface.scale_accumulator += temp_val;
			interface.curr++;
		}
	}
	else
	{
		Beepy_Console_Error_State_Restart(c);
	}
}

void BCUSART_Amend_Have_Buff_Index_Now_Get_Value_f(char c)
{
	interface.iface_f_to_c.bytes[interface.curr] = c;
	if(interface.curr == 3)
	{
		Beepy_Console_Wave_Buffer_Union u;
		float f_val = interface.iface_f_to_c.f;
		u.f = f_val;
		Beepy_Console_Update_Buffer(interface.buffer, interface.scale_accumulator, interface.type, u);
		interface.curr = 0;
		if(console.current_mode == Beepy_Console_Buffer_Display_Mode)
		{
			Beepy_Console_Redraw_Buffer( interface.scale_accumulator);
		}
		if(console.current_mode == Beepy_Console_Dual_Buffer_Display_Mode)
		{
			Beepy_Console_Redraw_Dual_Buffer_A(interface.scale_accumulator);
			Beepy_Console_Redraw_Dual_Buffer_B(interface.scale_accumulator);
		}
		Beepy_Console_State_Restart();
	}
	else
	{
		interface.curr++;
	}
}

void Beepy_Console_USARTI_Reset_Temp_Buffer()
{
	for(int i = 0; i < 34; i++)
	{
		interface.temp_buffer[i] = 0;
	}
	interface.curr = 0;
}

void Beepy_Console_Error_State_Restart(char c)
{
	char* error = "Error!: _";
	if(c!=0)
	{
		error[8] = c;
	}
	TM_USART_Puts(USART3, "*");
	Beepy_Console_USARTI_Reset_Temp_Buffer();
	interface.curr = 0;
	interface.length_accumulator = 0;
	interface.scale_accumulator = 0;
	interface.state = BCUSART_Ready;
	Beepy_Console_Add_Line(error);
	interface.data_values_received = 0;
	interface.receive_mode = Beepy_USARTI_Text;
}

void Beepy_Console_State_Restart()
{
	Beepy_Console_USARTI_Reset_Temp_Buffer();
	interface.curr = 0;
	interface.length_accumulator = 0;
	interface.scale_accumulator = 0;
	interface.state = BCUSART_Ready;
	interface.data_values_received = 0;
	interface.receive_mode = Beepy_USARTI_Text;
}

