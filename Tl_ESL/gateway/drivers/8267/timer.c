/********************************************************************************************************
 * @file     timer.c
 *
 * @brief    This file provides set of functions to manage the spi interface
 *
 * @author   qiuwei.chen@telink-semi.com
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

#include "timer.h"

#define   REG_IRQ_MASK0     READ_REG8(0x640)
enum{
	TIMER0_IRQ_MASK  = BIT(0),
	TIMER1_IRQ_MASK  = BIT(1),
	TIMER2_IRQ_MASK  = BIT(2),
};

#define   REG_IRQ_SRC0      READ_REG8(0x648)
enum{
	TIMER0_IRQ_SRC   = BIT(0),
	TIMER1_IRQ_SRC   = BIT(1),
	TIMER2_IRQ_SRC   = BIT(2),
};
/*******************************************************************
 * @brief  the function config the mode of timer
 * @param[in]  the timer need to config. enum variable
 * @param[in]  the mode want to config. enum varialbe
 * @return  1:parameter is ok 0:parameter is out of range. ie not timer0/timer1/timer2
 */
unsigned char TMR_InitMode(TMR_NumTypeDef timer_n,TMR_ModeTypeDef mode)
{
	if(timer_n == Timer0){
		REG_TMR_CTRL8 &= (~FLD_TMR0_MODE);
		REG_TMR_CTRL8 |= (mode<<1);
	}else if(timer_n == Timer1){
		REG_TMR_CTRL8 &= (~FLD_TMR1_MODE);
		REG_TMR_CTRL8 |= (mode<<4);
	}else if(timer_n == Timer2){
		REG_TMR_CTRL16 &= (~FLD_TMR2_MODE);
		REG_TMR_CTRL16 |= (mode<<7);
	}
	else{
		return 0;
	}
	return 1;
}

/**************************************************************************************
 * @brief  the mode1 and mode2 of timer are related to gpio. so need to config the gpio
 *         this function config polarity and mode of pin.(mode1:gpio count. mode2:gpio pulse)
 * @param[in] select the timer to config.enum varialbe(timer0,timer1, timer2)
 * @param[in] select the pin to capture.
 * @param[in] config the polarity of pin.
 * @return none.
 */
void TMR_GpioDetInit(TMR_NumTypeDef timer_n,GPIO_PinTypeDef pin, GPIO_BitActionTypeDef falling)
{
	unsigned char	bit = pin & 0xff;
	switch(timer_n){
	case Timer0:
		REG_TMR0_TICK = 0x00; //reset tick
		GPIO_SetInterruptPolarity(pin,falling);
		BM_SET(REG_GPIO_2RISC0(pin), bit);  //open the switch of timer0
		break;
	case Timer1:
		REG_TMR1_TICK = 0x00; //reset tick
		GPIO_SetInterruptPolarity(pin,falling);
		BM_SET(REG_GPIO_2RISC1(pin), bit);  //open the switch of timer1
		break;
	case Timer2:
		REG_TMR2_TICK = 0x00; //reset tick
		GPIO_SetInterruptPolarity(pin,falling);
		BM_SET(REG_GPIO_2RISC2(pin), bit);  //open the switch of timer2
		break;
	default:
		break;
	}
}

/****************************************
 * @brief  set the tick register of timer.
 * @param[in] select the timer to config.
 * @param[in] the value to set.
 * @return  1:parameter is ok 0:parameter is out of range. ie not timer0/timer1/timer2
 */
unsigned char TMR_SetTick(TMR_NumTypeDef timer_n, unsigned int tmpTick)
{
	if(timer_n == Timer0){
		REG_TMR0_TICK = tmpTick; //reset tick
	}
	else if(timer_n == Timer1){
		REG_TMR1_TICK = tmpTick; //reset tick
	}
	else if(timer_n == Timer2){
		REG_TMR2_TICK = tmpTick; //reset tick
	}
	else{
		return 0;
	}
	return 1;
}

/*******************************************
 * @brief set the capture register of timer.
 * @param[in] select the timer to set.
 * @param[in] the value to set.
 * @return  1:parameter is ok 0:parameter is out of range. ie not timer0/timer1/timer2
 *
 */
unsigned char TMR_SetCapt(TMR_NumTypeDef timer_n,unsigned int interval)
{
	if(timer_n == Timer0){
		REG_TMR0_CAPT = interval;
	}
	else if(timer_n == Timer1){
		REG_TMR1_CAPT = interval; //set interval by tick
	}
	else if(timer_n == Timer2){
		REG_TMR2_CAPT = interval;
	}
	else{
		return 0;
	}
	return 1;
}

/**************************************************
 * @brief  enable timer, ie timer will start to work.
 * @param[in]  select the timer to enable.
 * @return none.
 */
void TMR_Start(TMR_NumTypeDef timer_n)
{
	//	assert(timer_n);
	switch(timer_n){
	case Timer0:
		REG_TMR_CTRL8 |= FLD_TMR0_EN;
		break;
	case Timer1:
		REG_TMR_CTRL8 |= FLD_TMR1_EN;
		break;
	case Timer2:
		REG_TMR_CTRL8 |= FLD_TMR2_EN;
		break;
	default:
		break;
	}
}

/**************************************************
 * @brief  disable timer. ie the timer stop working.
 * @param[in]  select the timer to disable.
 * @return none.
 */
void TMR_Stop(TMR_NumTypeDef timer_n)
{
	//	assert(timer_n);
	switch(timer_n){
	case Timer0:
		REG_TMR_CTRL8 &= (~FLD_TMR0_EN);
		break;
	case Timer1:
		REG_TMR_CTRL8 &= (~FLD_TMR1_EN);
		break;
	case Timer2:
		REG_TMR_CTRL8 &= (~FLD_TMR2_EN);
		break;
	default:
		break;
	}
}

/**************************************************
 * @brief enable irq of timer.
 * @param[in]  select the timer to enable irq, enum variable
 * @return none.
 */
void TMR_IrqEn(TMR_NumTypeDef timer_n)
{
	switch(timer_n){
	case Timer0:
		REG_IRQ_MASK0 |= TIMER0_IRQ_MASK;
		break;
	case Timer1:
		REG_IRQ_MASK0 |= TIMER1_IRQ_MASK;
		break;
	case Timer2:
		REG_IRQ_MASK0 |= TIMER2_IRQ_MASK;
		break;
	default:
		break;
	}
}

/*************************************
 * @brief  clear irq of tiemr.
 * @param[in]  select the timer to clear.
 * return  none.
 */
void TMR_IrqClr(TMR_NumTypeDef timer_n)
{
	switch(timer_n){
	case Timer0:
		REG_TMR_STA  |= FLD_TMR_STA_TMR0;
		REG_IRQ_SRC0 |= TIMER0_IRQ_SRC;
		break;
	case Timer1:
		REG_TMR_STA |= FLD_TMR_STA_TMR1;
		REG_IRQ_SRC0 |= TIMER1_IRQ_SRC;
		break;
	case Timer2:
		REG_TMR_STA |= FLD_TMR_STA_TMR2;
		REG_IRQ_SRC0 |= TIMER2_IRQ_SRC;
		break;
	default:
		break;
	}
}
/************************************
 * @brief  the function set the period watchdog reset chip when chip run over.
 * @param[in]  set the period. the period is(s) : (param*2^18/system_clk) .
 * @return none
 */
void WD_SetIntervalTick(unsigned short interval)
{
	REG_TMR2_TICK = 0x00000000;    //reset tick register
	REG_TMR_CTRL &= (~FLD_TMR_WD_CAPT);
	REG_TMR_CTRL |= (interval<<9); //set the capture register
}

/*********************************************
 * @brief the function set the period.It is likely with WD_SetInterval.
 *        just this function calculate the value to set register with param.
 *        the maximum tolerance(system tick): 2^18 system ticks.
 * @param[in] the period to set.It is in seconds.
 * @return none
 */
void WD_SetIntervalSec(unsigned int period_s)
{
	static unsigned short tmp_period_us = 0;
	tmp_period_us = (period_s*1000*1000*TickPerUs>>18);
	REG_TMR2_TICK = 0x00000000;    //reset tick register
	REG_TMR_CTRL &= (~FLD_TMR_WD_CAPT);
	REG_TMR_CTRL |= (tmp_period_us<<9); //set the capture register
}
/*******************************************
 * @brief feed watchdog. ie restart watchdog.
 * @return none
 */
void WD_ReloadCnt(void)
{
	REG_TMR2_TICK = 0x00000000;    //reset tick register
}
/************************************
 * @brief start watchdog. ie enable watchdog
 * @return none
 */
void WD_Start(void)
{
	REG_TMR_CTRL |= FLD_TMR2_EN;   //enable timer2
	REG_TMR_CTRL |= FLD_TMR_WD_EN; //enable watch dog
}
/****************************
 * @brief stop watchdog. ie disable watchdog
 * @return none
 */
void WD_Stop(void)
{
	REG_TMR_CTRL &= (~FLD_TMR_WD_EN);
}


