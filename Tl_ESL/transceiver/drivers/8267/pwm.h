/********************************************************************************************************
 * @file     pwm.h
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

#ifndef _PWM_H
#define _PWM_H
#include "bsp.h"
#include "gpio.h"

/*************************************************************
*	@brief		enum variable, the number of PWM channels supported
*
*/
typedef enum{
	PWM0 = 0,
	PWM1,
	PWM2,
	PWM3,
	PWM4,
	PWM5,
}PWM_NTypeDef;


/*************************************************************
*	@brief		enum variable used for PWM work mode setting
*
*/
typedef enum{
	NORMAL=0x00,
	COUNT,
	IR = 0x03,
}PWM_ModeTypeDef;

/********************************************************
*
*	pwm interrupt source
*/
typedef enum{
	NOIRQ = 0xff,
	PWM0PNUM = 0,
	PWM1PNUM,
	PWM0CYCLEDONE,
	PWM1CYCLEDONE,
	PWM2CYCLEDONE,
	PWM3CYCLEDONE,
	PWM4CYCLEDONE,
	PWM5CYCLEDONE,
	PWMIRQSOURCENUM,
}PWM_IrqSourceTypeDef;

/********************************************************
 *
 */
#define    PAD_FUNC_MUX_PA     READ_REG8(0x5b0)
enum{
	PA0_FUNC_PWM0_DIS  = BIT(0),
	PA2_FUNC_PWM0N_DIS = BIT(1),
	PA3_FUNC_PWM1_DIS  = BIT(2),
	PA4_FUNC_PWM1N_DIS = BIT(4),
	PA5_FUNC_PWN2N_DIS = BIT(5),
};
#define    PAD_FUNC_MUX_PB     READ_REG8(0x5b1)
enum{
	PB0_FUNC_PWM2_EN   = BIT(0),
	PB1_FUNC_PWM2N_DIS = BIT(1),
	PB2_FUNC_PWM3_DIS  = BIT(2),
	PB3_FUNC_PWM3N_DIS = BIT(3),
	PB4_FUNC_PWM4_DIS  = BIT(4),
	PB5_FUNC_PWM4N_DIS = BIT(5),
	PB6_FUNC_PWM5_DIS  = BIT(6),
	PB7_FUNC_PWM5N_DIS = BIT(7),
};
#define    PAD_FUNC_MUX_PC     READ_REG8(0x5b2)
enum{
	PC0_FUNC_PWM0_DIS  = BIT(0),
	PC1_FUNC_PWM1_DIS  = BIT(1),
	PC2_FUNC_PWM2_DIS  = BIT(2),
	PC3_FUNC_PWM3_DIS  = BIT(3),
	PC4_FUNC_PWM4_DIS  = BIT(4),
	PC5_FUNC_PWM5_DIS  = BIT(5),
};
#define    PAD_FUNC_MUX_PE     READ_REG8(0x5b4)
enum{
	PE0_FUNC_PWM0_EN   = BIT(0),
	PE1_FUNC_PWM1_EN   = BIT(1),
};

//pwm0 gpio config
#define    PWM0_CFG_GPIO_A0()	do{\
									GPIO_SetGPIOEnable(GPIOA_GP0,Bit_RESET);\
									PAD_FUNC_MUX_PA &= (~PA0_FUNC_PWM0_DIS);\
								}while(0)
#define    PWM0_CFG_GPIO_C0()    do{\
									GPIO_SetGPIOEnable(GPIOC_GP0,Bit_RESET);\
									PAD_FUNC_MUX_PC &= (~PC0_FUNC_PWM0_DIS);\
								 }while(0)
#define    PWM0_CFG_GPIO_D5()    do{\
									GPIO_SetGPIOEnable(GPIOD_GP5,Bit_RESET);\
								 }while(0)
#define    PWM0_CFG_GPIO_E0()    do{\
									GPIO_SetGPIOEnable(GPIOE_GP0,Bit_RESET);\
									PAD_FUNC_MUX_PE |= PE0_FUNC_PWM0_EN;\
								 }while(0)

//pwm1 gpio config
#define    PWM1_CFG_GPIO_A3()    do{\
									GPIO_SetGPIOEnable(GPIOA_GP3,Bit_RESET);\
									PAD_FUNC_MUX_PA &= (~PA3_FUNC_PWM1_DIS);\
								 }while(0)
#define    PWM1_CFG_GPIO_C1()    do{\
									GPIO_SetGPIOEnable(GPIOC_GP1,Bit_RESET);\
									PAD_FUNC_MUX_PC &= (~PC1_FUNC_PWM1_DIS);\
								 }while(0)
#define    PWM1_CFG_GPIO_D6()    do{\
									GPIO_SetGPIOEnable(GPIOC_GP6,Bit_RESET);\
								 }while(0)
#define    PWM1_CFG_GPIO_E1()    do{\
									GPIO_SetGPIOEnable(GPIOE_GP1,Bit_RESET);\
									PAD_FUNC_MUX_PE |= PE1_FUNC_PWM1_EN;\
								 }while(0)

//pwm2 gpio config
#define    PWM2_CFG_GPIO_B0()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP0,Bit_RESET);\
									PAD_FUNC_MUX_PB |= PB0_FUNC_PWM2_EN;\
								 }while(0)
#define    PWM2_CFG_GPIO_C2()    do{\
									GPIO_SetGPIOEnable(GPIOC_GP2,Bit_RESET);\
									PAD_FUNC_MUX_PC &= (~PC2_FUNC_PWM2_DIS);\
								 }while(0)
#define    PWM2_CFG_GPIO_D7()    do{\
									GPIO_SetGPIOEnable(GPIOD_GP7,Bit_RESET);\
								 }while(0)

//pwm3 gpio config
#define    PWM3_CFG_GPIO_B2()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP2,Bit_RESET);\
									PAD_FUNC_MUX_PB &= (~PB2_FUNC_PWM3_DIS);\
								 }while(0)
#define    PWM3_CFG_GPIO_C3()    do{\
									GPIO_SetGPIOEnable(GPIOC_GP3,Bit_RESET);\
									PAD_FUNC_MUX_PC &= (~PC3_FUNC_PWM3_DIS);\
								 }while(0)

//pwm4 gpio config
#define    PWM4_CFG_GPIO_B4()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP4,Bit_RESET);\
									PAD_FUNC_MUX_PB &= (~PB4_FUNC_PWM4_DIS);\
								 }while(0)
#define    PWM4_CFG_GPIO_C4()    do{\
									GPIO_SetGPIOEnable(GPIOC_GP4,Bit_RESET);\
									PAD_FUNC_MUX_PC &= (~PC4_FUNC_PWM4_DIS);\
								 }while(0)

//pwm5 gpio config
#define    PWM5_CFG_GPIO_B6()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP6,Bit_RESET);\
									PAD_FUNC_MUX_PB &= (~PB6_FUNC_PWM5_DIS);\
								 }while(0)
#define    PWM5_CFG_GPIO_C5()    do{\
									GPIO_SetGPIOEnable(GPIOC_GP5,Bit_RESET);\
									PAD_FUNC_MUX_PC &= (~PC5_FUNC_PWM5_DIS);\
								 }while(0)

//pwm0_n gpio config
#define    PWM0N_CFG_GPIO_A2()   do{\
									GPIO_SetGPIOEnable(GPIOA_GP2,Bit_RESET);\
									PAD_FUNC_MUX_PA &= (~PA2_FUNC_PWM0N_DIS);\
								 }while(0)

//pwm1_n gpio config
#define    PWM1N_CFG_GPIO_A4()   do{\
									GPIO_SetGPIOEnable(GPIOA_GP4,Bit_RESET);\
									PAD_FUNC_MUX_PA &= (~PA4_FUNC_PWM1N_DIS);\
								 }while(0)

//pwm2_n gpio config
#define    PWM2N_CFG_GPIO_A5()   do{\
									GPIO_SetGPIOEnable(GPIOA_GP5,Bit_RESET);\
									PAD_FUNC_MUX_PA &= (~PA5_FUNC_PWN2N_DIS);\
								 }while(0)
#define    PWM2N_CFG_GPIO_B1()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP1,Bit_RESET);\
									PAD_FUNC_MUX_PB &= (~PB1_FUNC_PWM2N_DIS);\
								 }while(0)

//pwm3_n gpio config
#define    PWM3N_CFG_GPIO_B3()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP3,Bit_RESET);\
									PAD_FUNC_MUX_PB &= (~PB3_FUNC_PWM3N_DIS);\
								 }while(0)

//pwm4_n gpio config
#define    PWM4N_CFG_GPIO_B5()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP5,Bit_RESET);\
									PAD_FUNC_MUX_PB &= (~PB5_FUNC_PWM4N_DIS);\
								 }while(0)

//pwm5_n gpio config
#define    PWM5N_CFG_GPIO_B7()    do{\
									GPIO_SetGPIOEnable(GPIOB_GP7,Bit_RESET);\
									PAD_FUNC_MUX_PB &= (~PB7_FUNC_PWM5N_DIS);\
								 }while(0)

/*******************************************************************************
 *
 */
#define    PWM_INT_STATUS_REG             READ_REG8(0x7b1)
enum{
	PWM0_PNUM_INT_CLR  = BIT(0),
	PWM1_PNUM_INT_CLR  = BIT(1),
	PWM0_CYCLE_DONE_INT= BIT(2),
	PWM1_CYCLE_DONE_INT= BIT(3),
	PWM2_CYCLE_DONE_INT= BIT(4),
	PWM3_CYCLE_DONE_INT= BIT(5),
	PWM4_CYCLE_DONE_INT= BIT(6),
	PWM5_CYCLE_DONE_INT= BIT(7),
};
#define    PWM0_FRAME_INT_STATUS_CLEAR    PWM_INT_STATUS_REG |= PWM0_PNUM_INT_CLR;
#define    PWM1_FRAME_INT_STATUS_CLEAR    PWM_INT_STATUS_REG |= PWM1_PNUM_INT_CLR;

#define    PWM0_FRAME_DONE_INT_STATUS     PWM_INT_STATUS_REG & PWM0_PNUM_INT_CLR;
#define    PWM1_FRAME_DONE_INT_STATUS     PWM_INT_STATUS_REG & PWM1_PNUM_INT_CLR;

/************************************************************************************************************
 *
 */
#define    PWM_PNUM_REG_BASE              0x7ac
#define    PWM_SET_FRAME_NUM(v,n)   	  ( *(volatile unsigned short *)(PWM_PNUM_REG_BASE + (v << 1)) = (n) )

#define    PWM0_PNUM_REG                  READ_REG16(0x7ac)
#define    PWM0_SET_FRAME(n)              PWM0_PNUM_REG = (n)

#define    PWM1_PNUM_REG                  READ_REG16(0x7ae)
#define	   PWM1_SET_FRAME(n)              PWM1_PNUM_REG = (n)

/*************************************************************************************************************
 *
 */
#define    PWM_CTRL_REG                   READ_REG8(0x780)
enum{
	PWM0_EN   = BIT(0),
	PWM1_EN   = BIT(1),
	PWM2_EN   = BIT(2),
	PWM3_EN   = BIT(3),
	PWM4_EN   = BIT(4),
	PWM5_EN   = BIT(5),
};
#define    PWM_START(v)                   PWM_CTRL_REG |= BIT(v);
#define    PWM0_START                     PWM_CTRL_REG |= PWM0_EN;
#define    PWM1_START					  PWM_CTRL_REG |= PWM1_EN;

#define	   PWM_STOP(v)					  PWM_CTRL_REG &= (~BIT(v));
#define    PWM0_STOP					  PWM_CTRL_REG &= (~PWM0_EN);
#define    PWM1_STOP					  PWM_CTRL_REG &= (~PWM1_EN);


/************************************************************
*	@brief	Initiate the PWM function include seting and enabling the PWM clock.
*			For test pin number 22 - 27 is the PWM output pin.
*
*	@return	None
*/
extern void PWM_Init(unsigned char pwmCLKdiv);

/******************************************************************************
 * 	@brief    set the pwm clock,the function is similar to the pwm_Init.
 * 	          set the pwm clock.eg:pwm clock = 140 000.pwm_Init_clk(140000);
 * 	          limite: system_clock/pwm_clk <= 0xff. system_clock = CLOCK_SYS_CLOCK_HZ
 * 	@param[in]  pwm clock
 * 	@return    it will return error 0, not return success 1.
 * 	           because the div range from 0 to 0xff,so if the div if out of range
 */
extern unsigned char PWM_InitClk(unsigned int pwm_clk);

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
extern unsigned char PWM_Open(PWM_NTypeDef pwmNumber,PWM_ModeTypeDef pwmWorkM,unsigned short phaseTime,unsigned short cscValue,unsigned short cycValue,unsigned short sfValue);
/*******************************************************************************
*
*	@brief	close a pwm channel and reset the settings to default
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*	@return	None
*/
extern void PWM_Close(PWM_NTypeDef pwmNumber);

/*******************************************************************************
*
*	@brief	start a pwm channel and enable the output
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*	@return	None
*/
extern void PWM_Start(PWM_NTypeDef pwmNumber);

/*******************************************************************************
*
*	@brief	stop a pwm channel and disable the output
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*	@return	None
*/
extern void PWM_Stop(PWM_NTypeDef pwmNumber);

/*******************************************************************************
*
*	@brief	set the duty cycle of a pwm channel
*
*	@param[in] 	pwmNumber - enum variable of pwm channel number
*	@param[in]	cscValue - count status value
*	@param[in]	cycVlaue - cycle value

*	@return	'1' set success; '0' set error, if csValue>cycValue
*/
extern unsigned char PWM_DutyCycleSet(PWM_NTypeDef pwmNumber, unsigned short csValue,unsigned short cycValue);

/********************************************************
*	@brief	enable pwm interrupt
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*
*	@return	None
*/
extern unsigned char PWM_InterruptEnable(PWM_IrqSourceTypeDef pwmIrqSrc);

/********************************************************
*	@brief	disable pwm interrupt
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*
*	@return	None
*/
extern unsigned char PWM_InterruptDisable(PWM_IrqSourceTypeDef pwmIrqSrc);
/**********************************************************************
*
*	@brief	get interrupt source
*
*	@return	irq_source - interrupt source, enum variable
*/
extern PWM_IrqSourceTypeDef PWM_InterruptSourceGet(void );
/************************
 * @brief  judge whether the interrupt is parameter interrupt.
 * @param[in]  pwm_x  - enum variable of pwm channel number
 * @retrun irq_source - interrupt source, enum variable
 */
extern PWM_IrqSourceTypeDef PWM_ISpwmxInterrupt(PWM_IrqSourceTypeDef pwm_x);

/********************************************************
*	@brief	clear interrupt status
*
*	@param[in]	pwmNumber - enum variable of pwm channel number
*
*	@return	None
*/
extern void PWM_InterruptStatusClr(PWM_IrqSourceTypeDef irq_src);

/********************************************************************
*
*	@brief	Invert PWM-INV output (would generate the same output waveform as PWMN)
*
*	@param[in]	pwmNumber - enum variable of pwm number
*
*	@return	None
*/
extern void PWM_INVInvert(PWM_NTypeDef pwmNumber);


#endif /* PWM_8267_H_ */
