/********************************************************************************************************
 * @file     adc.c
 *
 * @brief    This file provides set of driver functions to manage the adc module
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
#include "bsp.h"
#include "adc.h"

//Select ADC mannul mode   tomorrow go on
#define		EN_MISC_AUTO    	WRITE_REG8(0x33,READ_REG8(0x33)|0x88)  //only enable M auto mode,close L/R auto mode.

#define     REG_ADC_MODE_STEP   READ_REG8(0x6b)

//Start sampling and conversion process for mannual mode
#define		STARTSAMPLING		WRITE_REG8(0x35,0x80)

//Read sampling data
#define     REG_ADC_RESULT      0x38
#define		READOUTPUTDATA		READ_REG16(REG_ADC_RESULT)

/*******************************************************************
 *
 */
 #define    REG_SYSCLK_SEL            READ_REG8(0x66)
 enum{
 	SYSCLK_DIV   = BIT_RNG(0,4),
 	FHS_SEL_LOW  = BIT(7),
 };
 #define    REG_FHS_SEL                READ_REG8(0x70)
 enum{
 	FHS_SEL_HIGH = BIT(0),
 };

 /**
  * @brief This function sets ADC reference voltage for the Misc and L channel
  * @param[in]   adcCha enum variable adc channel.
  * @param[in]   adcRF enum variable of adc reference voltage.
  * @return none
  */
void ADC_RefVoltageSet(ADC_RefVolTypeDef adcRF){
	unsigned char st;

	st = (unsigned char)adcRF;
	SET_MISC_REF_VOL(st);
}

/**
 * @brief This function sets ADC resolution for channel Misc
 * @param[in]   adcRes enum variable adc resolution.
 * @return none
 */
void ADC_ResSet(ADC_ResTypeDef adcRes){
	unsigned char resN;
	resN = (unsigned char )adcRes;
	REG_ADC_MISC_RES_SAMPLE &= (~MISC_RESOLUTION);
	REG_ADC_MISC_RES_SAMPLE |= (resN<<3);
}

/**
 * @brief This function sets ADC sample time(the number of adc clocks for each sample)
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcST enum variable of adc sample time.
 * @return none
 */
void ADC_SampleTimeSet( ADC_SampCycTypeDef adcST){

	unsigned char st;
	st = (unsigned char)adcST;

	REG_ADC_MISC_RES_SAMPLE &= (~MISC_SAMPLE);
	REG_ADC_MISC_RES_SAMPLE |= st;
}

/**
 * @brief This function sets ADC input channel
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcInCha enum variable of adc input channel.
 * @return none
 */
void ADC_AnaChSet(ADC_InputPTypeDef adcInCha){
	unsigned char cnI;

	cnI = (unsigned char)adcInCha;

	REG_ADC_MISC_INPUT &= (~MISC_INPUT_P);
	REG_ADC_MISC_INPUT |= cnI;
}

/***************************************************************************
*
*  @brief  set IO power supply for the 1/3 voltage division detection, there are two input sources of the
*      IO input battery voltage, one through the VDDH and the other through the  ANA_B<7> pin
*
*  @param[in]  IOp - input power source '1' is the VDDH; '2' is the ANA_B<7>.
*
*  @return  '1' setting success; '0' set error
*/
static unsigned char ADC_IOPowerSupplySet(unsigned char IOp){
	unsigned char vv1;
	if(IOp>2||IOp<1){

		return 0;
	}
	else{
		vv1 = ReadAnalogReg(AREG_BATT_DET_CTRL);
		vv1 &= (~BATT_DET_INPUT);
		vv1 = vv1 | (IOp<<4);
		WriteAnalogReg(AREG_BATT_DET_CTRL,vv1);
		return 1;
	}
}

/**
 * @brief This function sets ADC input channel mode - signle-end or differential mode
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   inM enum variable of ADCINPUTMODE.
 * @return none
 */
void ADC_AnaModeSet( ADC_InputMTypeDef inM){
	unsigned char cnM;

	cnM = (unsigned char)inM;
	REG_ADC_MISC_INPUT &= (~MISC_INPUT_M);
	REG_ADC_MISC_INPUT |= (cnM<<5);
}


/**********************************************************************
*	@brief	ADC initiate function, set the ADC clock details (4MHz) and start the ADC clock.
*			ADC clock relys on PLL, if the FHS isn't selected to 192M PLL (probably modified
*			by other parts codes), adc initiation function will returns error.
*
*	@return	setResult '1' set success; '0' set error
*/
unsigned char ADC_Init(void ){

	unsigned char fhsBL,fhsBH,adc_mod;
	fhsBL = (REG_FHS_SEL & FHS_SEL_HIGH);  //0x70[0]
	fhsBH = (REG_SYSCLK_SEL & FHS_SEL_LOW);//0x66[7]

	if((0 == fhsBL)&& (0 == fhsBH)){
		adc_mod = 192;
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0x7f); //power on pll
	}
	else if((0x80 == fhsBL) && (0 == fhsBH)){  //FHS select 32MHz RC
		adc_mod = 32;
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xfb); //power on 32M rc
	}
	else {  //FHS select 16MHz pad
		adc_mod = 16;
		WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xf7); //power on 16M pad
	}

	/******set adc clk as 4MHz******/
	REG_ADC_CLK_STEP = 0x04;// adc clk step as 4
	REG_ADC_CLK_MODE = adc_mod;// adc clk mode
	WriteAnalogReg(0x06, ReadAnalogReg(0x06) & 0xfe); //power on sar

	REG_ADC_MISC_PERIOD = (0xE2<<2);//set M channel period as (0xE2<<2),L channel's period is 0x06*16 defaultly.so sample frequency is sysclk/(0xE2<<2+0x06*16)
	EN_ADCCLK;//Enable adc CLK
	EN_MISC_AUTO;
	return 1;
}
/*****************************************************************
 * @brief  set input channel,set reference voltage, set resolution bits, set sample cycle
 * @param[in] chl          - enum variable ADCINPUTCH ,acd channel
 * @param[in] ref_vol      - enum variable ADCRFV
 * @param[in] resolution   - enum variable ADCRESOLUTION
 * @param[in] sample_cycle - enum variable ADCST
 * @return none
 */
void ADC_ParamSetting(ADC_InputPTypeDef chn,ADC_InputMTypeDef mode,ADC_RefVolTypeDef ref_vol,ADC_ResTypeDef resolution,ADC_SampCycTypeDef sample_cycle){
	/***1.set the analog input pin***/
	ADC_AnaChSet(chn);

	/***2.set ADC mode,signle-end or differential mode***/
	ADC_AnaModeSet(mode);///default is single-end

	/***3.set reference voltage***/
	ADC_RefVoltageSet(ref_vol);

	/***4.set resolution***/
	ADC_ResSet(resolution);

	/***5.set sample cycle**/
	ADC_SampleTimeSet(sample_cycle);
}
/**
 * @brief Initiate function for the battery check function
 * @param[in]   checkM Battery check mode, '0' for battery dircetly connected to chip,
 *              '1' for battery connected to chip via boost DCDC
 * @return none
 */
void ADC_BatteryCheckInit(BATT_ChnTypeDef checkM){
	/***1.set adc mode and input***/
#if INTER_RESIS_ONE_THIRD_EN    ///if 1,internal 1/3 voltage division open.    
	REG_ADC_MISC_INPUT &= (~MISC_INPUT_P);
	REG_ADC_MISC_INPUT |= OTVDD; //select "1/3 voltage division detection" as single-end input
	/***2.set battery check mode***/
	if(!checkM)
		ADC_IOPowerSupplySet(1);  ////VCC
	else
		ADC_IOPowerSupplySet(2);  ////B<7>
#else
	REG_ADC_MISC_INPUT &= (~MISC_INPUT_P);
	REG_ADC_MISC_INPUT |= B7;
#endif
	REG_ADC_MISC_INPUT &= (~MISC_INPUT_M);
	REG_ADC_MISC_INPUT |= (SINGLEEND<<5);      //select single_end mode
	/***3.set adc reference voltage***/
	ADC_RefVoltageSet(RV_1P428);     //Set reference voltage (V_REF)as  1.428V

	/***4.set adc resultion***/
	ADC_ResSet(RES14);               //Set adc resolution to 14 bits, bit[14] to bit bit[1]

	/***5.set adc sample time***/
	ADC_SampleTimeSet(S_3);          //set sample time
}

/**
 * @brief Initiate function for the temparture sensor
 * @param   None
 * @return none
 */
void ADC_TemSensorInit(void){
	/***1.set adc mode and input***/
	REG_ADC_MISC_INPUT &= (~MISC_INPUT_P);
	REG_ADC_MISC_INPUT |= TEMSENSORN;
	REG_ADC_MISC_INPUT &= (~MISC_INPUT_M);
	REG_ADC_MISC_INPUT |= (SINGLEEND<<5);       //select TEMSENSORN as single-end input

	/***2. set adc reference voltage***/
	ADC_RefVoltageSet(RV_AVDD);

	/***3.set adc resultion***/
	ADC_ResSet(RES14);

	/***4.set adc sample time***/
	ADC_SampleTimeSet(S_3);
}

/**
 * @brief get the temperature sensor sampled value
 * @param   None
 * @return the adc sampled value 14bits significants
 */
unsigned short ADC_TemValueGet(void){
	unsigned short sampledValue;
	while(!CHECKADCSTATUS);
	while(CHECKADCSTATUS);
	sampledValue = (unsigned short)(READOUTPUTDATA & 0x3FFF);
	while(!CHECKADCSTATUS);
	while(CHECKADCSTATUS);
	sampledValue = sampledValue - (unsigned short)(READOUTPUTDATA & 0x3FFF);
	return sampledValue;
}

/**
 * @brief get adc sampled value
 * @param[in]   adc_ch adc channel select, MISC or the LCHANNEL, enum variable
 * @param[in]   sample_mode adc sample mode, '1' manual mode; '0' auto sample mode
 * @return sampled_value, raw data
 */
unsigned short ADC_SampleValueGet(void){
	unsigned short sampledValue;

	while(!CHECKADCSTATUS);

	while(CHECKADCSTATUS);

	sampledValue = READOUTPUTDATA & 0x3FFF;

	return sampledValue;
}

