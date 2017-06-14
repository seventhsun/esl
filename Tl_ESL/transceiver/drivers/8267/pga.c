/********************************************************************************************************
 * @file     pga.c
 *
 * @brief    This file provides set of functions to manage the PGA module
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

void PGA_SetChannel(unsigned char chM){
	//set ANA_C<5> and ANA_C<4> as positive and minus input of the PGA
	if(chM){
		REG_PGA_INPUT_SEL1 |= PGA_INPUT_C4C5_EN;
		REG_PGA_INPUT_SEL2 |= PGA_INPUT_C4C5_EN;
	}
	//set ANA_C<3> and ANA_C<2> as positive and minus input of the PGA
	else{
		REG_PGA_INPUT_SEL1 &= (~PGA_INPUT_A2A3_DIS);
		REG_PGA_INPUT_SEL2 &= (~PGA_INPUT_A2A3_DIS);
	}
}

/**
 * @brief pga output enable or disable function, call this function to enable or disable the PGA output
 * @param[in]   enDis '1' enable output/mute off audio; '0' disable output/mute  on audio
 * @return none
 */
void PGA_SetOutput(unsigned char enDis){

	unsigned char tem;
	tem = ReadAnalogReg(AREG_PGA_MUTE);
	if(enDis)
		tem &= (~PGA_MUTE);//output enable
	else
		tem |= PGA_MUTE;   //output dis able
	WriteAnalogReg(AREG_PGA_MUTE,tem);
}

/**
 * @brief adjust pre-amplifier gain value
 * @param[in]   preGV enum var of PREAMPValue, 0DB or 20DB
 * @return '1' adjusted; '0' adjust error
 */
unsigned char PGA_PreGainAdjust(PGA_PreAmpValTypeDef preGV){
	unsigned char tem0,tem1;
	tem0 = (unsigned char)preGV;
	if(tem0>1)
		return 0;
	tem1 = ReadAnalogReg(0x87);
	tem1 = tem1 & 0xFE;
	tem1 = tem1 |tem0;
	WriteAnalogReg(0x87,tem1);
	return 1;
}

/**
 * @brief adjust post-amplifier gain value
 * @param[in]   posGV enum var of POSTAMPValue, 0,3,6 or 9dB
 * @return '1' adjusted; '0' adjust error
 */
unsigned char PGA_PostGainAdjust(PGA_PostAmpValTypeDef posGV){
	unsigned char tem0,tem1;
	tem0 = (unsigned char)posGV;
	if(tem0>3)
		return 0;
	tem1 = ReadAnalogReg(0x87);
	tem1 = tem1 & 0xF9;
	tem1 = tem1 |(tem0<<1);
	WriteAnalogReg(0x87,tem1);
	return 1;
}

/**
 * @brief pga initiate function, call this function to enable the PGA module
 *        the input channel deafult set to ANA_C<3> and ANA_C<2>
 * @param   none
 * @return  none
 */
void PGA_Init(void){
	unsigned char tem;
	//Power on pga
	tem = ReadAnalogReg(0x87);
	tem = tem & 0xF7;
	WriteAnalogReg(0x87,tem);
	//set PGA input channel
	PGA_SetChannel(1);
	PGA_SetOutput(1);
}


