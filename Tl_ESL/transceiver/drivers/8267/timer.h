/********************************************************************************************************
 * @file     timer.h
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

#ifndef _TIMER_H
#define _TIMER_H
#include "bsp.h"
#include "gpio.h"
/*********************************************
 * timer mode:
 * mode 1: reg_tick increase, up to reg_capt,and irq
 * mode 2: gpio count
 * mode 3: gpio pulse width, based on system tick.
 * mode 4: reg_tick increase from 0 to 0xffffffff then restart from 0.no irq.
 */


/****************************************************
 timer regs struct: begin  0x620
 *****************************************************/
typedef enum {  
	Timer0,
	Timer1,
	Timer2,
}TMR_NumTypeDef; 

typedef enum {  
	M0_SYS_CLK,
	M1_GPIO_TRIGGER,
	M2_GPIO_PULSEWIDTH,
	M3_TICK,
}TMR_ModeTypeDef;


#define  REG_TMR_CTRL           READ_REG32(0x620)
enum{
	FLD_TMR0_EN =				BIT(0),
	FLD_TMR0_MODE =				BIT_RNG(1,2),
	FLD_TMR1_EN = 				BIT(3),
	FLD_TMR1_MODE =				BIT_RNG(4,5),
	FLD_TMR2_EN =				BIT(6),
	FLD_TMR2_MODE = 			BIT_RNG(7,8),
	FLD_TMR_WD_CAPT = 			BIT_RNG(9,22),
	FLD_TMR_WD_EN =				BIT(23),
	FLD_TMR0_STA =				BIT(24),
	FLD_TMR1_STA =				BIT(25),
	FLD_TMR2_STA =				BIT(26),
	FLD_CLR_WD =				BIT(27),
};


#define REG_TMR_STA             READ_REG8(0x623)
enum{
	FLD_TMR_STA_TMR0 =			BIT(0),
	FLD_TMR_STA_TMR1 =			BIT(1),
	FLD_TMR_STA_TMR2 =			BIT(2),
	FLD_TMR_STA_WD =			BIT(3),
};

#define REG_TMR_CTRL16          READ_REG16(0x620)		
#define REG_TMR_CTRL8           READ_REG8(0x620)

#define WATCHDOG_TIMEOUT_COEFF	18		//  check register definiton, 0x622

#define WATCHDOG_DISABLE	( REG_TMR_CTRL &= (~FLD_TMR_WD_EN) )



#define REG_TMR_CAPT_BASE       0x624
#define REG_TMR0_CAPT           READ_REG32(0x624)
#define REG_TMR1_CAPT           READ_REG32(0x628)
#define REG_TMR2_CAPT           READ_REG32(0x62c)
#define REG_TMR_CAPT(v)         READ_REG32(REG_TMR_CAPT_BASE + ((v)<<2))

#define REG_TMR_TICK_BASE       0x630
#define REG_TMR0_TICK           READ_REG32(0x630)
#define REG_TMR1_TICK           READ_REG32(0x634)
#define REG_TMR2_TICK           READ_REG32(0x638)
#define REG_TMR_TICK(v)         READ_REG32(REG_TMR_TICK_BASE + ((v)<<2))

/*******************************************************************
 * @brief  the function config the mode of timer
 * @param[in]  the timer need to config. enum variable
 * @param[in]  the mode want to config. enum varialbe
 * @return  1:parameter is ok 0:parameter is out of range. ie not timer0/timer1/timer2
 */
extern unsigned char TMR_InitMode(TMR_NumTypeDef timer_n,TMR_ModeTypeDef mode);
/**************************************************************************************
 * @brief  the mode1 and mode2 of timer are related to gpio. so need to config the gpio
 *         this function config polarity and mode of pin.(mode1:gpio count. mode2:gpio pulse)
 * @param[in] select the timer to config.enum varialbe(timer0,timer1, timer2)
 * @param[in] select the pin to capture.
 * @param[in] config the polarity of pin.
 * @return none.
 */
extern void TMR_GpioDetInit(TMR_NumTypeDef timer_n,GPIO_PinTypeDef pin, GPIO_BitActionTypeDef falling);
/****************************************
 * @brief  set the tick register of timer.
 * @param[in] select the timer to config.
 * @param[in] the value to set.
 * @return  1:parameter is ok 0:parameter is out of range. ie not timer0/timer1/timer2
 */
extern unsigned char TMR_SetTick(TMR_NumTypeDef timer_n, unsigned int tmpTick);
/*******************************************
 * @brief set the capture register of timer.
 * @param[in] select the timer to set.
 * @param[in] the value to set.
 * @return  1:parameter is ok 0:parameter is out of range. ie not timer0/timer1/timer2
 *
 */
extern unsigned char TMR_SetCapt(TMR_NumTypeDef timer_n,unsigned int interval);

/**************************************************
 * @brief  enable timer, ie timer will start to work.
 * @param[in]  select the timer to enable.
 * @return none.
 */
extern void TMR_Start(TMR_NumTypeDef timer_n);

/**************************************************
 * @brief  disable timer. ie the timer stop working.
 * @param[in]  select the timer to disable.
 * @return none.
 */
extern void TMR_Stop(TMR_NumTypeDef timer_n);

/**************************************************
 * @brief enable irq of timer.
 * @param[in]  select the timer to enable irq, enum variable
 * @return none.
 */
extern void TMR_IrqEn(TMR_NumTypeDef timer_n);
/*************************************
 * @brief  clear irq of tiemr.
 * @param[in]  select the timer to clear.
 * return  none.
 */
extern void TMR_IrqClr(TMR_NumTypeDef timer_n);

/************************************
 * @brief  the function set the period watchdog reset chip when chip run over.
 * @param[in]  set the period. the period is(s) : (param*2^18/system_clk) .
 * @return none
 */
extern void WD_SetIntervalTick(unsigned short interval);

/*********************************************
 * @brief the function set the period.It is likely with WD_SetInterval.
 *        just this function calculate the value to set register with param.
 *        the maximum tolerance(system tick): 2^18 system ticks.
 * @param[in] the period to set.It is in seconds.
 * @return none
 */
extern void WD_SetIntervalSec(unsigned int period_s);

/*******************************************
 * @brief feed watchdog. ie restart watchdog.
 * @return none
 */
extern void WD_ReloadCnt(void);

/************************************
 * @brief start watchdog. ie enable watchdog
 * @return none
 */
extern void WD_Start(void);

/****************************
 * @brief stop watchdog. ie disable watchdog
 * @return none
 */
extern void WD_Stop(void);




#endif

