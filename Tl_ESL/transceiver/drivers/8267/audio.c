
/********************************************************************************************************
 * @file     audio.c
 *
 * @brief    This file provides set of driver functions to manage the audio module
 *
 * @author   junjun.xu@telink-semi.com; qiuwei.chen@telink-semi.com
 * @date     Oct. 8, 2016
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/
#include "pga.h"
#include "audio.h"
#include "gpio.h"

enum ADC_DataFormat{
	Fmt_UNSIGNED,
	Fmt_INVERT,
};

#define    REG_FHS_SEL                READ_REG8(0x70)
enum{
	FHS_SEL_HIGH = BIT(0),
};
#define    REG_SYSCLK_SEL            READ_REG8(0x66)
enum{
	SYSCLK_DIV   = BIT_RNG(0,4),
	FHS_SEL_LOW  = BIT(7),
};

/*******************************************************************
 *
 */
#define     PLL_OUTPUT_192M_AREG     0x88
#define     POWER_PLL_AREG           0x05
#define     PWOER_SAR_AREG           0x06

/*******************************************************************
 *
 */
#define     PE_FUNC_MUX         READ_REG8(0x5b4)
enum{
	E0_PWM0_EN  = BIT(0),
	E0_SDMP_DIS = BIT(0),
	E1_PWM1_EN  = BIT(1),
	E1_SDMM_DIS = BIT(1),
};

#define     PA_FUNC_MUX         READ_REG8(0x5b0)
enum{
	A0_DMIC_DI_EN = BIT(0),
	A0_PWM0_DIS   = BIT(0),
	A2_SPI_DO_EN  = BIT(1),
	A2_PWM0_N_DIS = BIT(1),
	A3_SPI_DI_EN  = BIT(2),
	A3_PWM1_DIS   = BIT(2),
	A4_SPI_CLK_EN = BIT(3),
	A4_PWM1_N_DIS = BIT(3),
};

/*******************************************************************
 *
 */

#define     I2S_STEP_REG          READ_REG8(0x67)
enum{
	I2S_STEP    = BIT_RNG(0,6),
	I2S_CLK_EN  = BIT(7),
};

#define     I2S_MODE_REG          READ_REG8(0x68)
enum{
	I2S_MODE    = BIT_RNG(0,7),
};
/*******************************************************************
 *
 */
#define    ASCL_STEP_REG         READ_REG16(0x564)
enum{
	ASCL_STEP_H_L  = BIT_RNG(0,15),
};

/*******************************************************************
 *
 */
#define     CLK_SEL_REG           READ_REG8(0x66)
enum{
	FHS_CLK_DIV    = BIT_RNG(0,4),
	SYSTEM_CLK_SEL = BIT_RNG(5,6),
	FHS_SEL_L      = BIT(7),
};
#define     FHS_SEL_REG           READ_REG8(0x70)
enum{
	FHS_SEL_H = BIT(0),
};

#define		SET_FHS_192M()			do{\
										CLK_SEL_REG &= ~FHS_SEL_L;\
										FHS_SEL_REG &= ~FHS_SEL_H;\
									}while(0)

/*******************************************************************
 *
 */
#define     CLK_EN_REG2            READ_REG8(0x65)
enum{
	DFIFO_CLK_EN = BIT(2),
};



/*******************************************************************
 *
 */
#define     DMIC_STEP_REG          READ_REG8(0x6c)
enum{
	DMIC_STEP = BIT_RNG(0,6),
	DMIC_CLK_EN = BIT(7),
};
#define     SET_DMIC_STEP(v)		do{\
										DMIC_STEP_REG &= (~DMIC_STEP);\
										DMIC_STEP_REG |= (v)&0x7F;\
									}while(0)

/*******************************************************************
 *
 */
#define     DMIC_MODE_REG          READ_REG8(0x6d)
enum{
	DMIC_MODE = BIT_RNG(0,7),
};

#define     SET_DMIC_MODE(v)		do{\
										DMIC_MODE_REG &= 0x00;\
										DMIC_MODE_REG |= (unsigned char)(v);\
									}while(0)

/*****************************************************************
 * @brief  config the mic buffer address and size
 * @param[in] pbuff - the first address of SRAM buffer MIC store data in.
 * @param[in] sizebuff - the size of pbuff.
 * @return none
 */
void Audio_ConfigMicBuf(unsigned int* pbuff,unsigned int sizebuff)
{
	DFIFO_BUFF_ADDR = (unsigned int)pbuff;
	DFIFO_BUFF_SIZE = (sizebuff>>4)-1;
}
/*****************************************************************
 * @brief  config the SDM buffer address and size
 * @param[in] pbuff - the first address of SDM store data.
 * @param[in] sizebuff- the size of pbuff.
 * @return none
 */
void Audio_ConfigSdmBuf(unsigned int* pbuff,unsigned int sizebuff)
{
	SDM_BUFF_ADDR = (unsigned int)pbuff;
	SDM_BUFF_SIZE = (sizebuff>>4) - 1;
}

/***************************************************************
*	@brief	audio AMIC init function, call the adc init function, configure ADC,
*	        PGA and filter parameters used for audio sample and process
*
*	@param[in]	mFlag - audio input mode flag, '1' differ mode; '0' single end mode.
*	@param[in]  adc_max_m - Misc period set parameter, T_Misc = 2 * adc_max_m
*	@param[in]  adc_max_l - Left channel period set, T_Left = 16*adc_max_l
*	@param[in]  d_samp - decimation filter down sample rate
*
*	@return	None
*/
void Audio_AmicInit(Audio_AmicMTypeDef modeFlag,unsigned short adc_max_m, unsigned char adc_max_l,Audio_SampRateTypeDef d_samp)
{
	unsigned char tmp_shift = 0;
	unsigned char fhsBL,fhsBH,adc_mod;

	CLK_EN_REG2 |= DFIFO_CLK_EN; //enable the DFIFO clock
	/******config FHS as 192M PLL******/
	fhsBL = (REG_FHS_SEL & FHS_SEL_HIGH);  //0x70[0]
	fhsBH = (REG_SYSCLK_SEL & FHS_SEL_LOW);//0x66[7]
	if((0 == fhsBL)&&(0 == fhsBH)) {
		adc_mod = 192;
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0x7f); //power on pll
	}
	else if((0x80 == fhsBL)&&(0 == fhsBH)){//FHS select 32MHz RC
		adc_mod = 32;
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xfb); //power on 32M rc
	}
	else {  //FHS select 16MHz pad
		adc_mod = 16;
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xf7); //power on 16M pad
	}
	/*****config AMIC channal and ADC ref_v resolution etc*******/
	/*******1.config adc clk as 4MHz*****/
	SET_ADC_STEP(0x04);
	SET_ADC_MODE(adc_mod);
	WriteAnalogReg(0x06, ReadAnalogReg(0x06) & 0xfe); //power on sar
	ADC_STEP_MODE |= ADC_CLK_EN;

	/*******2.ADC setting for analog audio sample**************************/
	SET_LEFT_REF_VOL(RV_AVDD);       //****Set reference voltage for sampling AVDD****/
	SET_LEFT_DATA_FORMAT(Fmt_INVERT);//****signed adc data****/
	SET_LEFT_RES(RES14);             //****set adc resolution to 14 bits****/
	SET_SAMPLE_CYCLE_LEFT(S_3);      //****set adc sample time to 3 cycles****/

	if(modeFlag == DIFF_MODE){//diff mode
		SET_LEFT_INPUT_P(AUD_PGAVOM);
		SET_LEFT_INPUT_M(PGAVOPM);
	}
	else{                      //single end
		SET_LEFT_INPUT_M(SINGLEEND);//set L channel single end mode
	}

	BIT_SET(DFIFO_AIN_REG,WPTR_EN|DFIFO_EN|INPUT_AMIC_EN);//audio input select AMIC, enable dfifo, enable wptr
	SET_PFM(adc_max_m);//set Misc channel period
	SET_PFL(adc_max_l);//set L channel period

	ADC_CTRL_REG &= (~BIT_RNG(0,7));
	BIT_SET(ADC_CTRL_REG,(AUDIO_OUTPUT_EN|AUTO_MODE_LEFT_EN)|(MONO_AUDIO<<4)|(ADC_DONE_FALLING<<6));//audio output enable,enable left auto mode,mono
	/***decimation/down sample[3:0]Decimation rate [6:4]decimation shift select(0~5)***/
	#if TL_MIC_32K_FIR_16K
		switch(d_samp&0x0f){
		case R2:
			tmp_shift = 0x01;
			break;
		case R3:
			tmp_shift = 0x02;
			break;
		default:
			tmp_shift = 0x01;
			break;
		}
	#else
		switch(d_samp&0x0f){
		case R3:
			tmp_shift = 0x01;
			break;
		case R5:
			tmp_shift = 0x03;
			break;
		case R6:
			tmp_shift = 0x04;
			break;
		default:
			tmp_shift = 0x04;
			break;
		}
	#endif
	SET_DEC_RATIO(d_samp);//setting down sample rate
	SET_DEC_SHIFT(tmp_shift);
	/***************HPF setting[3:0]HPF shift [4]bypass HPF [5]bypass ALC [6]bypass LPF***************/
	SET_HPF_SHIFT(0x09);/////0x0B;different PCB maybe set different vaule.
	/***************ALC Volume[5:0]manual volume [6]0:manual 1:auto**************************/
	SET_MANUAL_VOL(0x1c);
}

/****************************************************************
 * @brief  audio DMIC init function, config the speed of DMIC and get actual audio data.
 * @param[in] set the DMIC speed. such as 1M or 2M.
 * @param[in] set the decimation. ie div_speed.actually audio data is dmic_speed/d_samp.
 * @return none.
 */
void Audio_DmicInit(unsigned char dmic_speed,Audio_SampRateTypeDef d_samp)
{
	unsigned char tmp_shift = 0;
	unsigned char fhsBL,fhsBH,adc_mod;

	CLK_EN_REG2 |= DFIFO_CLK_EN; //enable the DFIFO clock
	/******config FHS as 192M PLL******/
	fhsBL = (REG_FHS_SEL & FHS_SEL_HIGH);  //0x70[0]
	fhsBH = (REG_SYSCLK_SEL & FHS_SEL_LOW);//0x66[7]
	if((0 == fhsBL)&&(0 == fhsBH)) {
		adc_mod = (192 - 4);
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0x7f); //power on pll
	}
	else if((0x80 == fhsBL)&&(0 == fhsBH)){//FHS select 32MHz RC
		adc_mod = (32 - 1);
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xfb); //power on 32M rc
	}
	else {  //FHS select 16MHz pad
		adc_mod = 16;
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xf7); //power on 16M pad
	}
	/*******config DMIC clk as dmic_speed MHz*****/
	GPIO_SetGPIOEnable(GPIO_DMIC_DI,Bit_RESET);///disable gpio func
	GPIO_SetGPIOEnable(GPIO_DMIC_CK,Bit_RESET);///disable gpio func
	GPIO_SetInputEnable(GPIO_DMIC_DI,Bit_SET);///enable input
	PA_FUNC_MUX |= A0_DMIC_DI_EN;   ///PA0 as DMIC_DI

	SET_DMIC_STEP(dmic_speed);
	SET_DMIC_MODE(adc_mod); // dmic clk mode as 192 ///for 0xC0,0xBC is close to 16k.
	DMIC_STEP_REG |= DMIC_CLK_EN;
	BIT_SET(DFIFO_AIN_REG,(WPTR_EN|DFIFO_EN) & (~INPUT_DMIC_DIS));//audio input select DMIC, enable dfifo, enable wptr
	/***************decimation/down sample[3:0]Decimation rate [6:4]decimation shift select(0~5)***/
	SET_DEC_RATIO(d_samp);
	tmp_shift = 0x05;
	SET_DEC_SHIFT(tmp_shift);
	/***************HPF setting[3:0]HPF shift [4]bypass HPF [5]bypass ALC [6]bypass LPF*************/
	SET_HPF_SHIFT(0x05);/////0x0B;different PCB maybe set different vaule.
	/***************ALC Volume[5:0]manual volume [6]0:manual 1:auto*********************************/
	SET_MANUAL_VOL(0x24);
	/*************enable HPF LPF ALC***/
	ALC_HPF_LPF_REG &= (~BIT_RNG(4,6));
	ALC_HPF_LPF_REG |= LPF_DIS;
}

/************************************************************************************
*
*	@brief	audio input set function, select analog audio input channel, start the filters
*
*	@param[in]	adc_ch:	if audio input as signle end mode, should identify an analog audio signal input channel,
*	            enum variable of ADCINPUTCH
*
*	@return	none
*/
void Audio_AmicInputSet(AUD_InputPTypeDef adc_ch)
{
	if(adc_ch == PGA_CH){
		/****PGA Setting****/
		PGA_Init();
		PGA_PreGainAdjust(DB0);//set pre pga gain to 0  ////DB20 by sihui
		PGA_PostGainAdjust(DB9);//set post pga gain to 0 ////DB9 by sihui
	}else{
		//L channel's input as C[0]
		//adc_AnaChSet(LCHANNEL,adc_ch);
		SET_LEFT_INPUT_P(adc_ch);
	}
	/*************enable HPF LPF ALC***/
	ALC_HPF_LPF_REG &= (~BIT_RNG(4,6));
	ALC_HPF_LPF_REG |= LPF_DIS;
}

/********************************************************
*
*	@brief		reg0x30[1:0] 2 bits for fine tuning, divider for slow down sample rate
*	@param[in]	unsigned char fine_tune
*	@return	none
*/
void Audio_FineTuneSampleRate(unsigned char fine_tune)
{
    //if(fine_tune>3) return;
    unsigned char tmp = REG_ADC_MISC_PERIOD;
    tmp |= (fine_tune&0x03); ////reg0x30[1:0] 2 bits for fine tuning

    REG_ADC_MISC_PERIOD = tmp;
}

/**********************************************************
 *  @brief:      tune decimation shift. .i.e register 0xb04 in datasheet.
 *  @param[in]   range from 0 to 5.
 *  @return none
 */
unsigned char Audio_TuneDeciShift(unsigned char deci_shift)
{
	if(deci_shift>0x05){
		return 0;
	}
	SET_DEC_SHIFT(deci_shift);
	return 1;
}

/**************************************************
 *   @brief:       tune the HPF shift. .i.e register 0xb05 in datasheet.
 *   @param[in]:   range from 0 to 0x0f
 *   @return none
 */
unsigned char Audio_TuneHPFshift(unsigned char hpf_shift)
{
	if(hpf_shift > 0x0f){
		return 0;
	}
	SET_HPF_SHIFT(hpf_shift);
	return 1;
}

/************************************************************************************
*
*	@brief	sdm set function, enabl or disable the sdm output, configure SDM output paramaters
*
*	@param[in]	audio_out_en: audio output enable or disable set, '1' enable audio output; '0' disable output
*	@param[in]	sdm_setp:	  SDM clk divider
*	@param[in]	sdm_clk:	  SDM clk, default to be 8Mhz
*
*	@return	none
*/
void Audio_SDMOutputSet(unsigned char audio_out_en,unsigned short sdm_step,unsigned char sdm_clk)
{
	if(audio_out_en){
		/***1.Set SDM output clock to 8 Mhz
		 *  I2s clock = fhs*step[6:0]/mod[7:0],
		 */
		I2S_STEP_REG &= ~I2S_STEP;
		I2S_STEP_REG |= (sdm_clk|I2S_CLK_EN);
		I2S_MODE_REG &= ~I2S_MODE;
		I2S_MODE_REG |= 0xc0;  //mod_i = 192MHz

		ASCL_STEP_REG &= (~ASCL_STEP_H_L);
		ASCL_STEP_REG |= sdm_step;

		/***2.Enable PN generator as dither control, clear bit 2, 3, 6***/
		BIT_CLR(AUDIO_CTRL_REG,PN_GENERATOR_SHAP_BYPASS|SHAPING_EN|CONST_INPUT_EN);
		BIT_SET(AUDIO_CTRL_REG,PN2_GENERATOR_EN|PN1_GENERATOR_EN);//set bit 4 bit 5
		BIT_CLR(AUDIO_CTRL_REG,AUDIO_EN|SDM_PLAYER_EN);//Close audio output

		PN1_CTRL_REG = 0x08;    //PN generator 1 bits ussed
		PN2_CTRL_REG = 0x08;    //PN generator 2 bits ussed

        /***3.Enable SDM pins(sdm_n, sdm_p)***/
		GPIO_SetGPIOEnable(GPIO_SDMP,Bit_RESET);
		GPIO_SetGPIOEnable(GPIO_SDMN,Bit_RESET);

		BIT_CLR(PE_FUNC_MUX,E0_SDMP_DIS|E1_SDMM_DIS);//enable SDMP and SDMM

		/***4.enable audio, enable sdm player***/
		BIT_SET(AUDIO_CTRL_REG,AUDIO_EN|SDM_PLAYER_EN);
	}
	else{
		BIT_CLR(AUDIO_CTRL_REG,SDM_PLAYER_EN);
	}
}

/*******************************************************************************************
*	@brief	set audio volume level
*
*	@param[in]	input_output_select:	select the tune channel, '1' tune ALC volume; '0' tune SDM output volume
*	@param[in]	volume_set_value:		volume level
*
*	@return	none
*/
void Audio_VolumeSet(unsigned char input_output_select,unsigned char volume_set_value)
{

	if(input_output_select){
		SET_MANUAL_VOL(volume_set_value);
	}
	else{
		SET_SDM_VOL(volume_set_value);
	}
}


/*************************************************************
*
*	@brief	automatically gradual change volume
*
*	@param[in]	vol_step - volume change step, the high part is decrease step while the low part is increase step
*			    gradual_interval - volume increase interval
*
*	@return	none
*/
void Audio_VolumeStepChange(unsigned char vol_step,unsigned short gradual_interval)
{
	ALC_VOL_STEP = vol_step;
	ALC_VOL_TICK = gradual_interval;
}

