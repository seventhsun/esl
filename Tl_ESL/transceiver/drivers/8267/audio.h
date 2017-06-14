/********************************************************************************************************
 * @file     audio.h
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

#ifndef _AUDIO_H
#define _AUDIO_H
#include "bsp.h"
#include "adc.h"
/************************
* @brief   1:enable  FIR
*          0:disable FIR
*          if enable FIR, the compression algorithm may be different from disable FIR
*/
#ifndef    TL_MIC_32K_FIR_16K
#define    TL_MIC_32K_FIR_16K    0
#endif
/***********************************************
 *
 */
typedef enum {
	PGA_CH = 0,
	AUD_C0,
	AUD_C1,
	AUD_C6,
	AUD_C7,
	AUD_B0,
	AUD_B1,
	AUD_B2,
	AUD_B3,
	AUD_B4,
	AUD_B5,
	AUD_B6,
	AUD_B7,
	AUD_PGAVOM,
	AUD_PGAVOP,
}AUD_InputPTypeDef;
/***********************************************
 *
 */
#define     DFIFO_BUFF_ADDR     READ_REG16(0xb00)
#define     DFIFO_BUFF_SIZE     READ_REG8(0xb02)
#define     SDM_BUFF_ADDR       READ_REG16(0x568)
#define     SDM_BUFF_SIZE       READ_REG8(0x56a)
/***********************************************
 *
 */
typedef enum {
	R1,R2,R3,R4,R5,R6,
	R7,R8,R16,R32,R64,R128,
}Audio_SampRateTypeDef;
/***********************************************
 *
 */
 typedef enum {
 	DIFF_MODE,
 	SINGLE_END_MODE,
}Audio_AmicMTypeDef;
/***********************************************
 *
 */
#define     DFIFO_AIN_REG       READ_REG8(0xb03)
enum{
	DMIC_EDGE_SEL  = BIT(0),
	INPUT_AMIC_EN  = BIT(1),
	INPUT_DMIC_DIS = BIT(1),
	DMIC_CH_DIS    = BIT(3),
	DFIFO_EN       = BIT(4),
	WPTR_EN        = BIT(5),
	WPTR_CLR       = BIT(6),
};
/*******************************************************************
 *
 */
#define    DFIFO_DEC_REG        READ_REG8(0xb04)
enum{
	DEC_RATIO    = BIT_RNG(0,3),
	DEC_SHIFT    = BIT_RNG(4,6), //the value not larger than 5
};
#define    SET_DEC_RATIO(v)		do{\
									DFIFO_DEC_REG &= ~DEC_RATIO;\
									DFIFO_DEC_REG |= ((v)&0x0f);\
								}while(0)
#define    SET_DEC_SHIFT(v)		do{\
									DFIFO_DEC_REG &= ~DEC_SHIFT;\
									DFIFO_DEC_REG |= (((v)&0x07)<<4);\
								}while(0)
/*******************************************************************
 *
 */
#define   ALC_HPF_LPF_REG       READ_REG8(0xb05)
enum{
	HPF_SHIFT   = BIT_RNG(0,3),
	HPF_DIS      = BIT(4),
	ALC_DIS      = BIT(5),
	LPF_DIS      = BIT(6),
};
#define    SET_HPF_SHIFT(v)		do{\
									ALC_HPF_LPF_REG &= ~HPF_SHIFT;\
									ALC_HPF_LPF_REG |= ((v)&0x0f);\
								}while(0)
/*******************************************************************
 *
 */
#define    ALC_VOL_REG           READ_REG8(0xb06)
enum{
	MANUAL_VOL_VAL  = BIT_RNG(0,5),
	MANUAL_VOL_EN   = BIT(6),
	AUTO_VOL_DIS    = BIT(6),
};
#define   SET_MANUAL_VOL(v)		do{\
									ALC_VOL_REG &= ~MANUAL_VOL_VAL;\
									ALC_VOL_REG |= ((v)&0x3f);\
								}while(0)	
/*******************************************************************
 *
 */
#define   ALC_VOL_STEP          READ_REG8(0xb0b)
enum{
	INCREASE_STEP = BIT_RNG(0,3),
	DECREASE_STEP = BIT_RNG(4,7),
};

#define   ALC_VOL_TICK          READ_REG16(0xb0c)
enum{
	VOL_TICK_L    = BIT_RNG(0,7),
	VOL_TICK_H    = BIT_RNG(0,5),
	VOL_TIC_HL    = BIT_RNG(0,12),
};															
/*******************************************************************
 *
 */
 #define    AUDIO_CTRL_REG         READ_REG8(0x560)
enum{
	AUDIO_EN        = BIT(0),
	SDM_PLAYER_EN   = BIT(1),
	PN_GENERATOR_SHAP_BYPASS = BIT(2),
	SHAPING_EN      = BIT(3),
	PN2_GENERATOR_EN= BIT(4),
	PN1_GENERATOR_EN= BIT(5),
	CONST_INPUT_EN  = BIT(6),
};
/*******************************************************************
 *
 */
#define   SDM_VOL_CTRL            READ_REG8(0x561)
enum{
	SDM_VOL_ADD_QUARTER  = BIT(0),
	SDM_VOL_ADD_HALF     = BIT(1),
	SDM_SHIFT_LEFT       = BIT_RNG(2,6),
	SDM_MUTE_EN          = BIT(7),
};
#define   SET_SDM_VOL(v)		do{\
									BIT_CLR(SDM_VOL_CTRL,BIT_RNG(0,6));\
									SDM_VOL_CTRL |= ((v)&0x7f);\
								}while(0)
/*******************************************************************
 *
 */
#define     PN1_CTRL_REG          READ_REG8(0x562)
enum{
	PN1_GENERATOR_BITS_USED = BIT_RNG(0,4),
};

#define     PN2_CTRL_REG          READ_REG8(0x563)
enum{
	PN2_GENERATOR_BITS_USED = BIT_RNG(0,4),
};

#define  ADC2AUDIO()		do{\
								ADC_CTRL_REG &= 0x00;\
								ADC_CTRL_REG |= (LEFT_CH_AUTO_EN|AUDIO_ADC_OUTPUT_EN|(AUDIO_ADC_MODE_MONO<<4)|(ADC_DONE_SIGNAL_FALLING)<<6);\
							}while(0)
#define  AUDIO2ADC()		do{\
								ADC_CTRL_REG &= 0x00;\
								ADC_CTRL_REG |= ((ADC_DONE_SIGNAL_FALLING<<6)|MISC_CH_AUTO_EN);\
							}while(0)

/*******************************************************************
 *
 */
#define  PAD_FUNC_MUX_PA     READ_REG8(0x5b0)
enum{
	PA0_FUNC_DMIC_DI_EN  = BIT(0),
};
#define  DMIC_CFG_GPIO_PA0_PA1()    do{\
								       GPIO_SetGPIOEnable(GPIOA_GP0|GPIOA_GP1,Bit_RESET);\
									   PAD_FUNC_MUX_PA |= PA0_FUNC_DMIC_DI_EN;\
								    }while(0)

/*****************************************************************
 * @brief  config the mic buffer address and size
 * @param[in] pbuff - the first address of SRAM buffer MIC store data in.
 * @param[in] sizebuff - the size of pbuff.
 * @return none
 */
extern void Audio_ConfigMicBuf(unsigned int* pbuff,unsigned int sizebuff);

/*****************************************************************
 * @brief  config the SDM buffer address and size
 * @param[in] pbuff - the first address of SDM store data.
 * @param[in] sizebuff- the size of pbuff.
 * @return none
 */
extern void Audio_ConfigSdmBuf(unsigned int* pbuff,unsigned int sizebuff);

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
extern void Audio_AmicInit(Audio_AmicMTypeDef modeFlag,unsigned short adc_max_m, unsigned char adc_max_l,Audio_SampRateTypeDef d_samp);

/****************************************************************
 * @brief  audio DMIC init function, config the speed of DMIC and get actual audio data.
 * @param[in] set the DMIC speed. such as 1M or 2M.
 * @param[in] set the decimation. ie div_speed.actually audio data is dmic_speed/d_samp.
 * @return none.
 */
extern void Audio_DmicInit(unsigned char dmic_speed,Audio_SampRateTypeDef d_samp);

/************************************************************************************
*
*	@brief	audio input set function, select analog audio input channel, start the filters
*
*	@param[in]	adc_ch:	if audio input as signle end mode, should identify an analog audio signal input channel,
*	            enum variable of ADCINPUTCH
*
*	@return	none
*/
extern void Audio_AmicInputSet(AUD_InputPTypeDef adc_ch);

/**********************************************************
 *  @brief:      tune decimation shift. .i.e register 0xb04 in datasheet.
 *  @param[in]   range from 0 to 5.
 *  @return none
 */
extern unsigned char Audio_TuneDeciShift(unsigned char deci_shift);

/**************************************************
 *   @brief:       tune the HPF shift. .i.e register 0xb05 in datasheet.
 *   @param[in]:   range from 0 to 0x0f
 *   @return none
 */
extern unsigned char Audio_TuneHPFshift(unsigned char hpf_shift);

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
extern void Audio_SDMOutputSet(unsigned char audio_out_en,unsigned short sdm_step,unsigned char sdm_clk);


/*******************************************************************************************
*	@brief	set audio volume level
*
*	@param[in]	input_output_select:	select the tune channel, '1' tune ALC volume; '0' tune SDM output volume
*	@param[in]	volume_set_value:		volume level
*
*	@return	none
*/
extern void Audio_VolumeSet(unsigned char input_output_select,unsigned char volume_set_value);


/********************************************************
*
*	@brief		get the battery value
*	@return		unsigned long - return the sampled value, 7 bits resolution
*/
extern unsigned short Audio_BatteryValueGet(void);


/********************************************************
*
*	@brief		reg0x30[1:0] 2 bits for fine tuning, divider for slow down sample rate
*	@param[in]	unsigned char fine_tune
*	@return	none
*/
extern void Audio_FineTuneSampleRate(unsigned char fine_tune);


#endif
