// stm32_wm5102_init.c

#include "wolfson.h"

typedef struct BITVAL    // used in function prbs()
{
 unsigned short b0:1, b1:1, b2:1, b3:1, b4:1, b5:1, b6:1;
 unsigned short b7:1, b8:1, b9:1, b10:1, b11:1, b12:1,b13:1;
 unsigned short dweebie:2; //Fills the 2 bit hole - bits 14-15
} bitval;

typedef union SHIFT_REG
{
 unsigned short regval;
 bitval bt;
} shift_reg;
int fb = 1;                        // feedback variable
shift_reg sreg = {0xFFFF};         // shift register

short prbs(void) 			       //gen pseudo-random sequence {-1,1}
{
  int prnseq;
  if(sreg.bt.b0)
	prnseq = -NOISELEVEL;		           //scaled negative noise level
  else
	prnseq = NOISELEVEL;		           //scaled positive noise level
  fb =(sreg.bt.b0)^(sreg.bt.b1);   //XOR bits 0,1
  fb^=(sreg.bt.b11)^(sreg.bt.b13); //with bits 11,13 -> fb
  sreg.regval<<=1;
  sreg.bt.b0=fb;			       //close feedback path
  return prnseq;			       //return noise sequence value
}



volatile int16_t pingIN[BUFSIZE], pingOUT[BUFSIZE], pongIN[BUFSIZE], pongOUT[BUFSIZE];

__IO uint32_t CODECTimeout = CODEC_LONG_TIMEOUT;
__IO uint8_t OutputDev = 0;

static  void DonaldDelay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

uint32_t Codec_TIMEOUT_UserCallback(void)
{
while(1);
}

uint32_t Codec_WriteRegister(uint32_t RegisterAddr, uint16_t RegisterValue)
{
	uint8_t Byte5 = (RegisterValue>>8) & 0xFF;
	uint8_t Byte6 = (RegisterValue) & 0xFF;

	uint8_t RegisterValueArray[2] = {Byte5, Byte6};
	TM_I2C_WriteMulti(I2C3, W5102_ADDR_0 << 1, RegisterAddr, RegisterValueArray, 2);

}

void configure_codec(uint16_t fs, int select_input, I2S_HandleTypeDef* i2s_handle)
{
	__HAL_I2S_DISABLE(i2s_handle);

	Codec_WriteRegister(0x0000, 0x0000);   // reset codec
  DonaldDelay(10000);                    // may or may not need delay here

  Codec_WriteRegister(0x0019, 0x0001);   // patch codec (supplied by Wolfson)
	Codec_WriteRegister(0x0080, 0x0003);
	Codec_WriteRegister(0x0081, 0xE022);
	Codec_WriteRegister(0x0410, 0x6080);
	Codec_WriteRegister(0x0418, 0xa080);
	Codec_WriteRegister(0x0420, 0xa080);
	Codec_WriteRegister(0x0428, 0xE000);
	Codec_WriteRegister(0x0443, 0xDC1A);
	Codec_WriteRegister(0x04B0, 0x0066);
	Codec_WriteRegister(0x0458, 0x000B);
	Codec_WriteRegister(0x0212, 0x0000);
	Codec_WriteRegister(0x0171, 0x0000);
	Codec_WriteRegister(0x035E, 0x000C);
	Codec_WriteRegister(0x02D4, 0x0000);
	Codec_WriteRegister(0x0080, 0x0000);  // end of patch

	Codec_WriteRegister(0x192, 0x8008);   // FLL2 12.000 MHz MCLK1 -> 24.576 MHz SYSCLK
	Codec_WriteRegister(0x193, 0x0018);   // could make this part of a switch in order
	Codec_WriteRegister(0x194, 0x007D);   // to allow 44.1kHz-related sample rates
	Codec_WriteRegister(0x195, 0x0008);
	Codec_WriteRegister(0x196, 0x0000);
	Codec_WriteRegister(0x191, 0x0001);

  DonaldDelay(10000);                   // may or may not be needed

	Codec_WriteRegister(0x101, 0x0245);   // clock set up as SYSCLK = 24.576 MHz, src FLL2, SYSCLK_FRAC = 0
	                                      // SYSCLK enabled

	switch(fs) // set sample rate 1 - currently only 48kHz-related sample rates allowed
	{
		case FS_8000_HZ:
			Codec_WriteRegister(0x102, 0x11);
			Codec_WriteRegister(0x580, 0x26);
		  break;
		case FS_12000_HZ:
			Codec_WriteRegister(0x102, 0x01);
			Codec_WriteRegister(0x580, 0x27);
		  break;
		case FS_16000_HZ:
			Codec_WriteRegister(0x102, 0x12);
			Codec_WriteRegister(0x580, 0x28);
		  break;
		case FS_24000_HZ:
			Codec_WriteRegister(0x102, 0x02);
			Codec_WriteRegister(0x580, 0x29);
		  break;
		case FS_32000_HZ:
			Codec_WriteRegister(0x102, 0x13);
			Codec_WriteRegister(0x580, 0x2A);
		  break;
		case FS_48000_HZ:
			Codec_WriteRegister(0x102, 0x03);
			Codec_WriteRegister(0x580, 0x2B);
		  break;
		default:
			Codec_WriteRegister(0x102, 0x11); // default is 8kHz sample rate
			Codec_WriteRegister(0x580, 0x26);
		  break;
	}

	Codec_WriteRegister(0x458, 0x0009);   // output noise gate enabled, threshold -84dB (important??)

	Codec_WriteRegister(0x200, 0x0001);   // not used prior to 20 March but I think necessary for CP2/LDO2 - analog inputs
                                        // Wolfson example write 0x0007 but bit 0 is CP2_ENA
	Codec_WriteRegister(0x210, 0x00D5);   // LDO1 control 0x00D5 -> LDO1 enabled, normal, 1V2

  Codec_WriteRegister(0x584, 0x0002);   // AIF3 I2S format
  Codec_WriteRegister(0x582, 0x0005);   // AIF3 LRCLK master - this takes LRC high, reliably I hope...
  DonaldDelay(10000);

  // MARK: Guessing this is right
  //MARK: IMPORTANT: ENABLE I2S HERE.
  //__HAL_I2S_ENABLE(i2s_handle);

  __HAL_I2S_ENABLE_IT(&hi2s2, I2S_IT_TXE);
  __HAL_I2S_ENABLE_IT(&hi2s2, I2S_IT_RXNE);
  __HAL_I2S_ENABLE_IT(&hi2s2, I2S_IT_ERR);
  //__HAL_I2S_ENABLE(i2s_handle2);
  //__HAL_I2S_ENABLE(&insext_handle);
  //I2S_Cmd(I2Sx, ENABLE);                // ...because it's necessary for correct startup of STM32F4
  //I2S_Cmd(I2Sxext, ENABLE);             // I2S interface (STM32F4 I2S module MUST be enabled while


  DonaldDelay(10000);                   // LRC is high - as per October 2013 errata)

  Codec_WriteRegister(0x587, 0x1010);   // AIF3 TX WL and SLOT_LEN both 16-bit
	Codec_WriteRegister(0x588, 0x1010);   // AIF3 RX WL and SLOT_LEN both 16-bit
	Codec_WriteRegister(0x59A, 0x0003);   // enable AIF3 RX channels (L and R)
	Codec_WriteRegister(0x599, 0x0003);   // enable AIF3 TX channels (L and R)
	Codec_WriteRegister(0x585, 0x0020);   // AIF3 32 BCLK cycles per LRC TX frame
	Codec_WriteRegister(0x586, 0x0020);   // AIF3 32 BCLK cycles per LRC RX frame

                                      // LINE OUT and HP OUT enabled in parallel
  Codec_WriteRegister(0x690, 0x0030); // OUT2L (LINE OUT) mixer input is AIF3 RX1 (from I2S) 30
  Codec_WriteRegister(0x691, 0x0080); // associated volume is 0dB

  Codec_WriteRegister(0x698, 0x0031); // OUT2R (LINE OUT) mixer input is AIF3 RX2 (from I2S) 31
  Codec_WriteRegister(0x699, 0x0080); // associated volume is 0dB

  Codec_WriteRegister(0x680, 0x0030); // OUT1L (HP OUT) mixer input is AIF3 RX1 (from I2S)
  Codec_WriteRegister(0x681, 0x0080); // associated volume is 0dB

  Codec_WriteRegister(0x688, 0x0031); // OUT1R (HP OUT) mixer input is AIF3 RX2 (from I2S)
  Codec_WriteRegister(0x689, 0x0080); // associated volume is 0dB

// route LHPFs to AIF3TX
  Codec_WriteRegister(0x0780, 0x0060); // AIF3TX mixer from LHPF1 60
	Codec_WriteRegister(0x0788, 0x0061); // AIF3TX mixer from LHPF2 61
  Codec_WriteRegister(0x0781, 0x0080); // AIF3TX mixer gain 0dB
	Codec_WriteRegister(0x0789, 0x0080); // AIF3TX mixer gain 0dB

  Codec_WriteRegister(0x0EC0, 0x0003); // LHPF1 HPF enabled
  Codec_WriteRegister(0x0EC1, 0xF09e); // LHPF1 cutoff frequency in Hz depends on fs
  Codec_WriteRegister(0x0EC4, 0x0003); // LHPF2 HPF enabled
  Codec_WriteRegister(0x0EC5, 0xF09E); // LHPF2 cutoff frequency in Hz depends on fs

  Codec_WriteRegister(0x0901, 0x0080); // LHPF1 mixer source 1 gain 0dB
  Codec_WriteRegister(0x0909, 0x0080); // LHPF2 mixer source 1 gain 0dB

switch(select_input)
{
	case WM5102_LINE_IN:
    Codec_WriteRegister(0x0900, 0x0014); // LHPF1 mixer from IN3 (LINE IN)
	  Codec_WriteRegister(0x0908, 0x0015); // LHPF2 mixer from IN3
  	Codec_WriteRegister(0x0300, 0x0030); // enable IN3L and IN3R 0030 LINE IN
	  Codec_WriteRegister(0x0320, 0x2290); // IN3L PGA gain +8.0dB LINE IN (potential divider comp.)
	  Codec_WriteRegister(0x0321, 0x0280); // IN3L ADC volume 0dB LINE IN
	  Codec_WriteRegister(0x0324, 0x0090); // IN3R PGA gain +8.0dB LINE IN (potential divider comp.)
	  Codec_WriteRegister(0x0325, 0x0280); // IN3R ADC volume 0dB LINE IN
  break;
	case WM5102_DMIC_IN:
    Codec_WriteRegister(0x0900, 0x0012); // LHPF1 mixer from IN2 (DMIC IN)
  	Codec_WriteRegister(0x0908, 0x0013); // LHPF2 mixer from IN2
    Codec_WriteRegister(0x0300, 0x000C); // enable IN2L and IN2R 000C DMIC IN
	  Codec_WriteRegister(0x0318, 0x3480); // IN2 DMIC IN IN2L PGA vol 0dB
	  Codec_WriteRegister(0x031D, 0x0280); // IN2R ADC volume 0dB DMIC IN
	  Codec_WriteRegister(0x0319, 0x0280); // IN2L ADC volume 0dB DMIC IN
	  Codec_WriteRegister(0x0219, 0x01A7); // MICBIAS2 enable DMIC IN
  break;
	case WM5102_MIC_IN:
    Codec_WriteRegister(0x0900, 0x0010); // LHPF1 mixer from IN1 (MIC IN)
  	Codec_WriteRegister(0x0908, 0x0011); // LHPF2 mixer from IN1
  	Codec_WriteRegister(0x0300, 0x0003); // enable IN1L and IN1R 0003 MIC IN
	  Codec_WriteRegister(0x0310, 0x2A80); // IN1L PGA vol 0dB MIC IN
	  Codec_WriteRegister(0x0314, 0x0080); // IN1R PGA volume 0dB MIC IN
	  Codec_WriteRegister(0x0311, 0x0280); // IN1L ADC volume 0dB MIC IN
	  Codec_WriteRegister(0x0315, 0x0280); // IN1R ADC volume 0dB DMIC IN
	  Codec_WriteRegister(0x0218, 0x01A7); // MICBIAS1 enable MIC IN
  break;
	default:
    Codec_WriteRegister(0x0900, 0x0014); // LHPF1 mixer from IN3 (LINE IN)
	  Codec_WriteRegister(0x0908, 0x0015); // LHPF2 mixer from IN3
  	Codec_WriteRegister(0x0300, 0x0030); // enable IN3L and IN3R 0030 LINE IN
	  Codec_WriteRegister(0x0320, 0x2290); // IN3L PGA gain +8.0dB LINE IN (potential divider comp.)
	  Codec_WriteRegister(0x0321, 0x0280); // IN3L ADC volume 0dB LINE IN
	  Codec_WriteRegister(0x0324, 0x0090); // IN3R PGA gain +8.0dB LINE IN (potential divider comp.)
	  Codec_WriteRegister(0x0325, 0x0280); // IN3R ADC volume 0dB LINE IN
  break;
}

	Codec_WriteRegister(0x419, 0x0280); // DAC 2 volume L 0dB (LINE OUT)
	Codec_WriteRegister(0x41D, 0x0280); // DAC 2 volume R 0dB (LINE OUT)
	Codec_WriteRegister(0x411, 0x0280); // DAC 1 volume L 0dB (HP OUT)
	Codec_WriteRegister(0x415, 0x0280); // DAC 1 volume R 0dB (HP OUT)

	Codec_WriteRegister(0x400, 0x000F); // enable outputs OUT2L, OUT2R, OUT1L, OUT1R

	//StartDMAs();



}




// interrupt version of initialisation routine

void stm32_wm5102_init(uint16_t fs, int select_input, int io_method)
{

// MARK: All my HAL replacement code goes here. Old code afterwards.

	/*
	//Create I2S Handle.

	I2S_HandleTypeDef hi2s;
	DMA_HandleTypeDef hdma_i2s2_ext_tx;
	DMA_HandleTypeDef hdma_i2s2_ext_rx;

	hi2s.Instance = SPI2;
	hi2s.Init.Mode = I2S_MODE_SLAVE_RX;
	hi2s.Init.Standard = I2S_STANDARD_PHILIPS;
	hi2s.Init.DataFormat = I2S_DATAFORMAT_16B;
	hi2s.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE; // MARK: DO WE NEED THIS?
	hi2s.Init.AudioFreq = I2S_AUDIOFREQ_48K; // MARK: SET THIS
	hi2s.Init.CPOL = I2S_CPOL_HIGH;
	hi2s.Init.ClockSource = I2S_CLOCK_EXTERNAL;
	hi2s.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
	HAL_I2S_Init(&hi2s);

	__HAL_I2S_DISABLE();

	*/
// declare structures for I2S, NVIC, and GPIO initialisation
//  //I2S_InitTypeDef I2S_InitStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  GPIO_InitTypeDef GPIO_InitStructure;
//  DMA_InitTypeDef DMA_InitStructure;
//
//// GPIOB, GPIOC and GPIOD clocks on
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
//
//
//  /* Configure PD15 and PD13 in output pushpull mode */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_11 ;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOD, &GPIO_InitStructure);
//
//// set up GPIO initialisation structure for I2C pins SCL and SDA
//  GPIO_InitStructure.GPIO_Pin = CODEC_I2C_SCL_PIN | CODEC_I2C_SDA_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//// initialise I2C pins SCL and SDA
//  GPIO_Init(CODEC_I2C_GPIO, &GPIO_InitStructure);
//// configure GPIO pins for I2C alternative functions
//	GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SCL_PINSRC, CODEC_I2C_GPIO_AF);
//	GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SDA_PINSRC, CODEC_I2C_GPIO_AF);
//
//// set up GPIO initialisation structure for I2S pins WS, CK, SD and SD
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//
//  GPIO_InitStructure.GPIO_Pin = I2Sx_WS_PIN | I2Sx_CK_PIN;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
//  GPIO_InitStructure.GPIO_Pin = I2Sx_SD_PIN | I2Sxext_SD_PIN ;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);
//
//  /* connect I2S pins to alternative functions */
//  GPIO_PinAFConfig(I2Sx_WS_GPIO_PORT, I2Sx_WS_SOURCE, I2Sx_WS_AF);
//  GPIO_PinAFConfig(I2Sx_CK_GPIO_PORT, I2Sx_CK_SOURCE, I2Sx_CK_AF);
//  GPIO_PinAFConfig(I2Sx_SD_GPIO_PORT, I2Sx_SD_SOURCE, I2Sx_SD_AF);
//  GPIO_PinAFConfig(I2Sxext_SD_GPIO_PORT, I2Sxext_SD_SOURCE, I2Sxext_SD_AF);
//
//  //I2S_Cmd(I2Sx, DISABLE);
//  //I2S_Cmd(I2Sxext, DISABLE);
//
//// I2S clocks on
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
//
//  //I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;
//  //I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
//  //I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;
//  //I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveRx;
//
//  //I2S_Init(I2Sx, &I2S_InitStructure);
//
//  //I2S_FullDuplexConfig(I2Sxext, &I2S_InitStructure); // enable the I2Sx_ext peripheral for Full Duplex mode */
//
//
//  DonaldDelay(10000);
//
  //    GPIO_SetBits(GPIOD, GPIO_Pin_11); // pi board reset high
//
  //Codec_CtrlInterface_Init();
	//configure_codec(fs, select_input);
//
  DonaldDelay(10000);

//
//switch(io_method)
//{
//case IO_METHOD_INTR:
//
//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
//NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//NVIC_Init(&NVIC_InitStructure);
//
//
//  DonaldDelay(10000);
//
//// do we need to do this?
//  SPI_I2S_ClearFlag(I2Sx, I2S_FLAG_CHSIDE);
//  SPI_I2S_ClearFlag(I2Sxext, I2S_FLAG_CHSIDE);
//
//// enable the Rx buffer not empty interrupt
//  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
//
//
//break;
//case IO_METHOD_DMA:
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
//DMA_Cmd(DMA1_Stream3,DISABLE);
//DMA_DeInit(DMA1_Stream3);
//
//		DMA_InitStructure.DMA_Channel = DMA_Channel_0;                           // correct channel
//    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                   // apparently we don't need FIFO ?
//    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                  // yes
//    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;      // we need 16-bit
//    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                          // yes (although forced by double-buffer)
//    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;         // no incr on I2S address
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                  // yes
//    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(SPI2_BASE + 0x0C);        // ck address of I2S2
//    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                      // yes
//    DMA_InitStructure.DMA_BufferSize = BUFSIZE;                              // needs to be 2x no. of sample instants
//    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;        // don't want burst
//    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;        // don't want burst
//    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;            // yes?
//    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // should be 16-bit?
//    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) pingIN;            // do we need type casting?
//
//    DMA_Init(DMA1_Stream3,&DMA_InitStructure);
//
//
//    DMA_DoubleBufferModeConfig(DMA1_Stream3, (uint32_t) pongIN, DMA_Memory_0);
//    DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);
//    DMA_InitStructure.DMA_Channel = DMA_Channel_2;                           // correct channel
//    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                   // apparently we don't need FIFO ?
//    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                  // yes
//    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;      // we need 16-bit
//    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                          // yes (although forced by double-buffer)
//    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;         // no incr on I2S address
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                  // yes
//    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(I2S2ext_BASE + 0x0C);        // check address of I2S2_ext
//    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                      // yes
//    DMA_InitStructure.DMA_BufferSize = BUFSIZE;                              // needs to be 2x no. of sample instants
//    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;        // don't want burst
//    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;            // yes?
//    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // should be 16-bit?
//    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) pingOUT;            // do we need type casting?
//
//    DMA_Init(DMA1_Stream4,&DMA_InitStructure);
//
//
//    DMA_DoubleBufferModeConfig(DMA1_Stream4, (uint32_t) pongOUT, DMA_Memory_0);
//    DMA_DoubleBufferModeCmd(DMA1_Stream4, ENABLE);
//
//
//    DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
//    DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
//
//
//	  SPI_I2S_DMACmd(I2Sx, SPI_I2S_DMAReq_Rx, ENABLE);
//  	DMA_Cmd(DMA1_Stream3,ENABLE);
//	  SPI_I2S_DMACmd(I2Sxext, SPI_I2S_DMAReq_Tx, ENABLE);
//  	DMA_Cmd(DMA1_Stream4,ENABLE);
//
//    NVIC_EnableIRQ(DMA1_Stream3_IRQn);
//    NVIC_EnableIRQ(DMA1_Stream4_IRQn);
//
//
//
//break;
//default:
//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
//NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//NVIC_Init(&NVIC_InitStructure);
//
//
//  DonaldDelay(10000);
//
//// do we need to do this?
//  SPI_I2S_ClearFlag(I2Sx, I2S_FLAG_CHSIDE);
//  SPI_I2S_ClearFlag(I2Sxext, I2S_FLAG_CHSIDE);
//
//// enable the Rx buffer not empty interrupt
//  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
//
//break;
//}

}


void StartDMAs(){




  HAL_I2S_MspInit(&hi2s2);
  //MX_I2S2_Init();

  hi2s2.Instance = SPI2;
     hi2s2.Init.Mode = I2S_MODE_SLAVE_RX;
     hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
     hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
     hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
     hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_48K;
     hi2s2.Init.CPOL = I2S_CPOL_HIGH; //Different
     hi2s2.Init.ClockSource = I2S_CLOCK_EXTERNAL;
     hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
     if (HAL_I2S_Init(&hi2s2) != HAL_OK)
     {
       Error_Handler();
     }


  __HAL_I2S_DISABLE(&hi2s2);
  //while(HAL_GPIO_ReadPin(GPIOB,1<<12) != GPIO_PIN_RESET);
  //while(HAL_GPIO_ReadPin(GPIOB,1<<12) != GPIO_PIN_SET);


  hdma_i2s2_ext_tx.Init.Channel = DMA_CHANNEL_0;
  HAL_DMA_Init(&hdma_i2s2_ext_tx);
  __HAL_LINKDMA(&hi2s2,hdmatx,hdma_i2s2_ext_tx);
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

  HAL_StatusTypeDef status = HAL_I2SEx_TransmitReceive_DMA(&hi2s2, (uint16_t*)&pingOUT[0], (uint16_t*)&pingIN[0], BUFSIZE * 2);
  assert_param(status == HAL_OK);

  __HAL_I2S_ENABLE(&hi2s2);



  //completeDMA();
}

