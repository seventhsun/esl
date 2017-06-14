/********************************************************************************************************
 * @file     pwm.c
 *
 * @brief    This file provides set of functions for PWM module
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

#include "pwm.h"

#define     CLOCK_EN_REG1       READ_REG8(0x64)
enum{
	PWM_CLK_EN = BIT(4),
};
#define     RESET_CTL_REG0      READ_REG8(0x61)
enum{
	PWM_RST_EN  = BIT(4),
};

#define		PHASE_TIME_BASE		0x788
#define		CSC_VALUE_BASE		0x794
#define		CYC_VALUE_BASE		0x796
#define		SFN_VALUE_BASE		0x7ac

/********************************************************
 *
 */
#define     PWM_EN_REG          READ_REG8(0x780)
#define     EN_PWM(v)           PWM_EN_REG |= BIT(v)
#define     DIS_PWM(v)          PWM_EN_REG &= ~BIT(v)

#define     PWM_CLK_DIV_REG     READ_REG8(0x781)
#define     SET_PWMCLK(v)       PWM_CLK_DIV_REG = v

#define     PWM_CC0             READ_REG8(0x783)
#define     INVERT_PWM(v)       PWM_CC0 |= BIT(v)

//PWM-INV output invert
#define     PWM_CC1             READ_REG8(0x784)
#define     INVERT_PWMINV(v)    PWM_CC1 |= BIT(v)

#define     PWM_CC2             READ_REG8(0x785)
#define     EN_PWMPOL(v)        PWM_CC2 |= BIT(v)
#define     DIS_PWMPOL(v)       PWM_CC2 &= ~BIT(v)

#define     PWMN_IRQ_MASK       READ_REG8(0x7b0)
#define     EN_PWMINT(v)        PWMN_IRQ_MASK |= BIT(v)
#define     DIS_PWMINT(v)       PWMN_IRQ_MASK &= ~BIT(v)
 
/********************************************************
 *
 */
#define     IRQ_MASK        READ_REG8(0x641)
enum{
	PWM_IRQ_MASK = BIT(6),
};
#define     EN_PWMINTM          IRQ_MASK |= PWM_IRQ_MASK
#define     DIS_PWMINTM         IRQ_MASK &= ~PWM_IRQ_MASK


#define     PWM_IRQ_STA         READ_REG8(0x7b1)
#define     CLR_PWMIRQS(v)      PWM_IRQ_STA |= BIT(v)

//Set pwm mode, v can only be pwm0 and pwm1, n is the mode
#define     PWM_MODE_REG        READ_REG8(0x782)
#define		SET_PWMMODE(v,n)	do{\
									PWM_MODE_REG &= ~(n<<(v*2));\
									PWM_MODE_REG |= (n<<(v*2));\
								}while(0)

//Set Phase time, v is the PWM number, n is the desired value
#define		SET_PWMPT(v,n)		WRITE_REG16((PHASE_TIME_BASE + v*2),n)

//PWM count status cycle value set function, v is the PWM numer, n is the desired value
#define		SET_PWMCSCV(v,n)	WRITE_REG16((CSC_VALUE_BASE + v*4),n)

//PWM cycle value set function, v is the PWM numer, n is the desired value
#define		SET_PWMCYCV(v,n)	WRITE_REG16((CYC_VALUE_BASE + v*4),n)

//Set number of signal frames, v can only be pwm0 and pwm1, n is the setting value
#define		SET_PWMSFN(v,n)		WRITE_REG16((SFN_VALUE_BASE + v*2),n)

////end of reg

/************************************************************
*	@brief	Initiate the PWM function include seting and enabling the PWM clock.
*			For test pin number 22 - 27 is the PWM output pin.
*
*	@return	None
*/
void PWM_Init(unsigned char pwmCLKdiv){
	SET_PWMCLK(pwmCLKdiv);//PWMCLK = SYSCLK/4
	CLOCK_EN_REG1 |= PWM_CLK_EN;//PWM CLK Enable
    RESET_CTL_REG0 &= (~PWM_RST_EN);
}
/******************************************************************************
 * 	@brief    set the pwm clock,the function is similar to the pwm_Init.
 * 	          set the pwm clock.eg:pwm clock = 140 000.pwm_Init_clk(140000);
 * 	          limite: system_clock/pwm_clk <= 0xff. system_clock = CLOCK_SYS_CLOCK_HZ
 * 	@param[in]  pwm clock
 * 	@return    it will return error 0, not return success 1.
 * 	           because the div range from 0 to 0xff,so if the div if out of range
 */
unsigned char PWM_InitClk(unsigned int pwm_clk){
	unsigned int tmp_div = (unsigned int)((TickPerUs*1000*1000+(pwm_clk>>1))/pwm_clk);
	if(tmp_div <= 0xff){
		SET_PWMCLK((unsigned char)(tmp_div-1));
		CLOCK_EN_REG1 |= PWM_CLK_EN;//PWM CLK Enable
		RESET_CTL_REG0 &= (~PWM_RST_EN);// disable PWM reset, wakeup
	}else{
		return 0;
	}
	return 1;
}

/********************************************************************************
*	@brief	open a pwm channel and setting the parameters
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*	@param[in]	pwmWorkM - enum variable of pwm work mode
*	@param[in]	phaseTime - delay time of the pwm channel before enter into the count status
*	@param[in]	cscValue - count status value
*	@param[in]	cycVlaue - cycle value
*	@param[in]	sfValue - signal frame value for the COUNT and IR mode
*
*	@return	'1' set success; '0' set error, if you set pwm2 - pwm5 to work on the other not normal mode, the function will return error.
*/
unsigned char PWM_Open(PWM_NTypeDef pwmNumber,PWM_ModeTypeDef pwmWorkM,unsigned short phaseTime,unsigned short cscValue,unsigned short cycValue,unsigned short sfValue){
	unsigned long setValue = 0;
	unsigned char pwmN,pwmM;
	pwmN = (unsigned char)pwmNumber;
	pwmM = (unsigned char)pwmWorkM;
	if(pwmN>1 && pwmM!=0){
		return 0;
	}
	SET_PWMPT(pwmN,phaseTime);
	setValue = (unsigned long)cscValue;
	setValue = setValue + ((unsigned long)cycValue<<16);
	WRITE_REG32((CSC_VALUE_BASE + pwmN*4),setValue);
	/***set pwm work mode(only for pwm0/pwm1)***/
	if (pwmN<2) {
		SET_PWMMODE(pwmN,pwmM);
	}

	if(pwmWorkM != NORMAL)
		SET_PWMSFN(pwmN,sfValue);

	EN_PWM(pwmN);//Should be here, donot enable the PWM unitil all the settings done
	return 1;
}
/*******************************************************************************
*
*	@brief	close a pwm channel and reset the settings to default
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*	@return	None
*/
void PWM_Close(PWM_NTypeDef pwmNumber){
	unsigned char pwmN;
	pwmN = (unsigned char)pwmNumber;
	SET_PWMPT(pwmN,0);
	WRITE_REG32((CSC_VALUE_BASE + pwmN*4),0);
	SET_PWMMODE(pwmN,0);
	if(pwmN<2){
		SET_PWMSFN(pwmN,0);
	}
	DIS_PWM(pwmN);
}

/*******************************************************************************
*
*	@brief	start a pwm channel and enable the output
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*	@return	None
*/
void PWM_Start(PWM_NTypeDef pwmNumber)
{
	unsigned char pwmN = (unsigned char)pwmNumber;
    EN_PWM(pwmN);
}

/*******************************************************************************
*
*	@brief	stop a pwm channel and disable the output
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*	@return	None
*/
void PWM_Stop(PWM_NTypeDef pwmNumber)
{
	unsigned char pwmN = (unsigned char)pwmNumber;
    DIS_PWM(pwmN);
}

/*******************************************************************************
*
*	@brief	set the duty cycle of a pwm channel
*
*	@param[in] 	pwmNumber - enum variable of pwm channel number
*	@param[in]	cscValue - count status value
*	@param[in]	cycVlaue - cycle value

*	@return	'1' set success; '0' set error, if csValue>cycValue
*/
unsigned char PWM_DutyCycleSet(PWM_NTypeDef pwmNumber,unsigned short csValue,unsigned short cycValue){
	unsigned char pwmN;
	pwmN = (unsigned char)pwmNumber;
	if(cycValue >csValue){
	WRITE_REG16((CSC_VALUE_BASE + pwmN*4),csValue);
	WRITE_REG16((CYC_VALUE_BASE + pwmN*4),cycValue);
	return 1;
	}
	return 0;
}

/********************************************************
*	@brief	enable pwm interrupt
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*
*	@return	None
*/
unsigned char PWM_InterruptEnable(PWM_IrqSourceTypeDef pwmIrqSrc){
	EN_PWMINT(pwmIrqSrc);
	EN_PWMINTM;
	return 1;
}
/********************************************************
*	@brief	disable pwm interrupt
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*
*	@return
*/
unsigned char PWM_InterruptDisable(PWM_IrqSourceTypeDef pwmIrqSrc){
	DIS_PWMINT(pwmIrqSrc);
	return 1;
}
/**********************************************************************
*
*	@brief	get interrupt source
*
*	@return	irq_source - interrupt source, enum variable
*/
PWM_IrqSourceTypeDef PWM_InterruptSourceGet(void ){
	unsigned char IrqStatus = PWM_IRQ_STA;
	int i = 0;

	if (IrqStatus == 0) {
        return NOIRQ;
	}
	else {
        for (i=PWM0PNUM; i<PWMIRQSOURCENUM; i++) {
        	if ((1<<i) & IrqStatus)
        		break;
        }
        return (PWM_IrqSourceTypeDef)i;
	}
}

/************************
 * @brief  judge whether the interrupt is parameter interrupt.
 * @param[in]  pwm_x  - enum variable of pwm channel number
 * @retrun irq_source - interrupt source, enum variable
 */
PWM_IrqSourceTypeDef PWM_ISpwmxInterrupt(PWM_IrqSourceTypeDef pwm_x){
	unsigned char IrqStatus = PWM_IRQ_STA;

	if (IrqStatus == 0) {
        return NOIRQ;
	}
	else {
		IrqStatus &= BIT(pwm_x);
		if(IrqStatus){
			return pwm_x;
		}
        return 0;
	}
}

/********************************************************
*	@brief	clear interrupt status
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*
*	@return	None
*/
void PWM_InterruptStatusClr(PWM_IrqSourceTypeDef irq_src){
	unsigned char irqS;
	irqS = (unsigned char)irq_src;
	CLR_PWMIRQS(irqS);
}

/********************************************************************
*
*	@brief	Invert PWM-INV output (would generate the same output waveform as PWMN)
*
*	@param[in]	pwmNumber - enum variable of pwm number
*
*	@return	None
*/
void PWM_INVInvert(PWM_NTypeDef pwmNumber){
	INVERT_PWMINV((unsigned char)pwmNumber);
}

