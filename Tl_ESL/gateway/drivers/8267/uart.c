/********************************************************************************************************
 * @file     uart.c
 *
 * @brief    This file provides set of functions to manage the UART interface
 *
 * @author   qiuwei.chen@telink-semi.com; junjun.chen@telink-semi.com;
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
#include "uart.h" 
#include "irq.h"
#define USE_GPIO_SIMULATE_RTS  1

//UART realted registers definitions
#define REG_UART_SC_RST        REG_ADDR8(0x61)
enum {
    FLD_UART_SC_RST_RS232 = BIT(3),
};    

#define REG_UART_CLOCK_DIV     REG_ADDR16(0x94)
enum {
    FLD_UART_CLK_DIV = BIT_RNG(0,14),
    FLD_UART_CLK_DIV_EN = BIT(15),
};

#define REG_UART_CTRL0         REG_ADDR8(0x96)
enum {
    FLD_UART_CTRL0_BWPC = BIT_RNG(0,3),
    FLD_UART_CTRL0_RX_DMA_EN = BIT(4),
    FLD_UART_CTRL0_TX_DMA_EN = BIT(5),
	FLD_UART_CTRL0_RX_IRQ_EN = BIT(6),
	FLD_UART_CTRL0_TX_IRQ_EN = BIT(7),
};

#define REG_UART_CTRL1         REG_ADDR8(0x97)
enum {
    FLD_UART_CTRL1_CTS_SELECT = BIT(0),
    FLD_UART_CTRL1_CTS_EN = BIT(1),
    FLD_UART_CTRL1_PARITY_EN = BIT(2),
    FLD_UART_CTRL1_PARITY = BIT(3),
    FLD_UART_CTRL1_STOP_BIT = BIT_RNG(4,5),
    FLD_UART_CTRL1_TTL = BIT(6),
    FLD_UART_CTRL1_LOOPBACK = BIT(7),
};

#define REG_UART_CTRL2         REG_ADDR8(0x98)
enum {
    FLD_UART_CTRL2_RTS_TRIG_LVL = BIT_RNG(0,3),
    FLD_UART_CTRL2_RTS_PARITY = BIT(4),
    FLD_UART_CTRL2_RTS_MANUAL_VAL = BIT(5),
    FLD_UART_CTRL2_RTS_MANUAL_EN = BIT(6),
    FLD_UART_CTRL2_RTS_EN = BIT(7),
};

#define REG_UART_CTRL3        REG_ADDR8(0x99)
enum{
	RX_IRQ_TRIG_LEVEL = BIT_RNG(0,3),
	TX_IRQ_TRIG_LEVEL = BIT_RNG(4,7),
};

#define REG_UART_RX_TIMEOUT0    REG_ADDR8(0x9a)
#define REG_UART_RX_TIMEOUT1    REG_ADDR8(0x9b)
#define REG_UART_STATUS         REG_ADDR8(0x9d)
enum {
    FLD_UART_STATUS_RBCNT = BIT_RNG(0,2),
    FLD_UART_STATUS_IRQ = BIT(3),
    FLD_UART_STATUS_WBCNT = BIT_RNG(4,6),
    FLD_UART_STATUS_RX_ERR = BIT(7),
};

#define REG_UART_IRQ_STATUS     REG_ADDR8(0x9e)
enum {
    FLD_UART_IRQ_STATUS_TXDONE = BIT(0),
    FLD_UART_IRQ_STATUS_TXBUF_IRQ = BIT(1),
    FLD_UART_IRQ_STATUS_RXDONE = BIT(2),
    FLD_UART_IRQ_STATUS_RXBUF_IRQ = BIT(3),
};

#define REG_UART_DMA_RX_ADDR0     REG_ADDR16(0x500)
#define REG_UART_DMA_RX_SIZE0     REG_ADDR8(0x502)
#define REG_UART_DMA_TX_ADDR0     REG_ADDR16(0x504)
#define REG_UART_DMA_MODE0        REG_ADDR8(0x503)
#define REG_UART_DMA_MODE1        REG_ADDR8(0x507)
enum {
    FLD_UART_DMA_MODE_WRITE = BIT(0),
};

#define REG_UART_DMA_READY0    REG_ADDR8(0x524)
enum {
    FLD_UART_DMA_READY0_CH0 = BIT(0),
    FLD_UART_DMA_READY0_CH1 = BIT(1),
    FLD_UART_DMA_READY0_CH2 = BIT(2),
    FLD_UART_DMA_READY0_CH3 = BIT(3),
    FLD_UART_DMA_READY0_CH4 = BIT(4),
    FLD_UART_DMA_READY0_CH5 = BIT(5),
    FLD_UART_DMA_READY0_CH6 = BIT(6),
    FLD_UART_DMA_READY0_CH7 = BIT(7),
};

#define REG_UART_GPIO_DUPLEX    REG_ADDR8(0x5b2)

/**
 * @brief This function resets the UART module.
 * @return none
 */
void UART_Reset(void)
{
    REG_UART_SC_RST |= FLD_UART_SC_RST_RS232;
    REG_UART_SC_RST &= (~FLD_UART_SC_RST_RS232);
}

static unsigned char IsPrime(unsigned int n)
{
	unsigned int i = 5;
	if(n <= 3){
		return 1; //althought n is prime, but the bwpc must be larger than 2.
	}
	else if((n %2 == 0) || (n % 3 == 0)){
		return 0;
	}
	else{
		for(i=5;i*i<n;i+=6){
			if((n % i == 0)||(n %(i+2))==0){
				return 0;
			}
		}
		return 1;
	}
}
/***********************************************************
 * @brief  calculate the best bwpc(bit width) .i.e reg0x96
 * @algorithm: BaudRate*(div+1)*(bwpc+1)=system clock
 *    simplify the expression: div*bwpc =  constant(z)
 *    bwpc range from 3 to 15.so loop and
 *    get the minimum one decimal point
 * @return the position of getting the minimum value
 */
static unsigned char g_bwpc = 0;
static unsigned int  g_uart_div = 0;
static void GetBetterBwpc(unsigned int baut_rate)
{
	unsigned char i = 0, j= 0;
	unsigned int primeInt = 0;
	unsigned char primeDec = 0;
	unsigned int D_intdec[13],D_int[13];
	unsigned char D_dec[13];

	unsigned int tmp_sysclk = TickPerUs*1000*1000;
	primeInt = tmp_sysclk/baut_rate;
	primeDec = 10*tmp_sysclk/baut_rate - 10*primeInt;
	/************************************************************
	 * calculate the primeInt and check whether primeInt is prime.
	 */
	if(IsPrime(primeInt)){ // primeInt is prime
		primeInt += 1;  //+1 must be not prime. and primeInt must be larger than 2.
	}
	else{
		if(primeDec > 5){ // >5
			primeInt += 1;
			if(IsPrime(primeInt)){
				primeInt -= 1;
			}
		}
	}
	/*******************************************
	 * get the best division value and bit width
	 */
	for(i=3;i<=15;i++){
		D_intdec[i-3] = (10*primeInt)/(i+1);////get the LSB
		D_dec[i-3] = D_intdec[i-3] - 10*(D_intdec[i-3]/10);///get the decimal section
		D_int[i-3] = D_intdec[i-3]/10;///get the integer section
	}

	//find the max and min one decimation point
	unsigned char position_min = 0,position_max = 0;
	unsigned int min = 0xffffffff,max = 0x00;
	for(j=0;j<13;j++){
		if((D_dec[j] <= min)&&(D_int[j] != 0x01)){
			min = D_dec[j];
			position_min = j;
		}
		if(D_dec[j]>=max){
			max = D_dec[j];
			position_max = j;
		}
	}

	if((D_dec[position_min]<5) && (D_dec[position_max]>=5)){
		if(D_dec[position_min]<(10-D_dec[position_max])){
			g_bwpc = position_min + 3;
			g_uart_div = D_int[position_min]-1;
		}
		else{
			g_bwpc = position_max + 3;
			g_uart_div = D_int[position_max];
		}
	}
	else if((D_dec[position_min]<5) && (D_dec[position_max]<5)){
		g_bwpc = position_min + 3;
		g_uart_div = D_int[position_min] - 1;
	}
	else{
		g_bwpc = position_max + 3;
		g_uart_div = D_int[position_max];
	}
}

/**
 * @brief This function initializes the UART module.
 * @param[in]   BaudRate the selected baudrate for UART interface
 * @param[in]   Parity selected parity type for UART interface
 * @param[in]   StopBit selected length of stop bit for UART interface
 * @return none
 */
void UART_Init(unsigned int BaudRate, UART_ParityTypeDef Parity, UART_StopBitTypeDef StopBit)
{
    GetBetterBwpc(BaudRate); //get the best bwpc and uart_div

    REG_UART_CTRL0 &= (~FLD_UART_CTRL0_BWPC);
    REG_UART_CTRL0 |= g_bwpc; //set bwpc
    REG_UART_CLOCK_DIV = (g_uart_div | FLD_UART_CLK_DIV_EN); //set div_clock
    REG_UART_RX_TIMEOUT0 = (g_bwpc+1) * 12; //one byte includes 12 bits at most
    REG_UART_RX_TIMEOUT1 = 1; //if over 2*(tmp_bwpc+1),one transaction end.

    //parity config
    if (Parity) {
        REG_UART_CTRL1 |= FLD_UART_CTRL1_PARITY_EN; //enable parity function
        if (PARITY_EVEN == Parity) {
            REG_UART_CTRL1 &= (~FLD_UART_CTRL1_PARITY); //enable even parity 
        }
        else if (PARITY_ODD == Parity) {
            REG_UART_CTRL1 |= FLD_UART_CTRL1_PARITY; //enable odd parity
        }
        else {
            while(1);
        }
    }
    else {
        REG_UART_CTRL1 &= (~FLD_UART_CTRL1_PARITY_EN); //disable parity function
    }

    //stop bit config
    REG_UART_CTRL1 &= (~FLD_UART_CTRL1_STOP_BIT);
    REG_UART_CTRL1 |= StopBit;
}
/************************
 * @brief enable uart DMA
 * @return none
 */
void UART_DmaInit(void)
{
	//enable DMA function of tx and rx
	REG_UART_CTRL0 |= (FLD_UART_CTRL0_RX_DMA_EN | FLD_UART_CTRL0_TX_DMA_EN);
    //receive DMA and buffer details
    REG_UART_DMA_MODE0 = FLD_UART_DMA_MODE_WRITE; //set DMA 0 mode to 0x01 for receive
    REG_UART_DMA_MODE1 = 0; //DMA1 mode to send
}

/*********************************************
 * @brief config the irq of uart tx and rx
 * @param[in] 1:enable rx irq. 0:disable rx irq
 * @param[in] 1:enable tx irq. 0:disable tx irq
 * @return none
 */
void UART_NDmaIrqInit(unsigned char rx_irq_en,unsigned char tx_irq_en)
{
	if(rx_irq_en){
		REG_UART_CTRL0 |= FLD_UART_CTRL0_RX_IRQ_EN;
	}else{
		REG_UART_CTRL0 &= (~FLD_UART_CTRL0_RX_IRQ_EN);
	}

	if(tx_irq_en){
		REG_UART_CTRL0 |= FLD_UART_CTRL0_TX_IRQ_EN;
	}else{
		REG_UART_CTRL0 &= (~FLD_UART_CTRL0_TX_IRQ_EN);
	}
	REG_IRQ_MASK |= FLD_IRQ_UART_EN;
}

/************************************************************************
 * @brief  config the number level setting the irq bit of status register 0x9d
 *         ie 0x9d[3].
 *         If the cnt register value(0x9c[0,3]) larger or equal than the value of 0x99[0,3]
 *         or the cnt register value(0x9c[4,7]) less or equal than the value of 0x99[4,7],
 *         it will set the irq bit of status register 0x9d, ie 0x9d[3]
 * @param[in] rx_level - receive level value. ie 0x99[0,3]
 * @param[in] tx_level - transmit level value. ie 0x99[4,7]
 * @return none
 */
void UART_NDmaIrqTrigLevel(unsigned char rx_level, unsigned char tx_level)
{
	REG_UART_CTRL3 = rx_level | (tx_level<<4);
}

/*********************************
 * @brief  get the status of uart irq.
 * @return  if 0, not uart irq
 *          if not 0, indicate tx or rx irq
 */
unsigned char UART_NDmaIrqGet(void)
{
	return  (REG_UART_STATUS&FLD_UART_STATUS_IRQ);
}

/**********************************************
 * @brief  uart send data function with not DMA method.
 * @param[in]  the data to be send.
 * @uart_TxIndex  it will cycle the four registers 0x90 0x91 0x92 0x93 for the design of SOC.
 *                so we need variable to remember the index.
 * @return none
 */
void UART_NDmaSendByte(unsigned char uartData)
{
	static unsigned char uart_TxIndex = 0;
	WRITE_REG8(0x90+uart_TxIndex,uartData);
	uart_TxIndex++;
	uart_TxIndex &= 0x03;// cycle the four register 0x90 0x91 0x92 0x93.
}

/**
 * @brief uart send data function, this  function tell the DMA to get data from the RAM and start
 *        the DMA transmission
 * @param[in]   addr pointer to the buffer containing data need to send
 * @return '1' send success; '0' DMA busy
 */
volatile unsigned char UART_Send(unsigned char* Addr)
{
    if (REG_UART_IRQ_STATUS & FLD_UART_IRQ_STATUS_TXDONE) {
        REG_UART_DMA_TX_ADDR0 = (unsigned short)Addr; //packet data, start address is sendBuff+1
        REG_UART_DMA_READY0 = FLD_UART_DMA_READY0_CH1;
        return 1;
    }

    return 0;
}

/**
 * @brief data receive buffer initiate function. DMA would move received uart data to the address space,
 *        uart packet length needs to be no larger than (RecvBufLen - 4).
 * @param[in]   RecvAddr pointer to the receiving buffer
 * @param[in]   RecvBufLen length in byte of the receiving buffer
 * @return none
 */
void UART_RecBuffInit(unsigned char *RecvAddr, unsigned short RecvBufLen)
{
    unsigned char bufLen;
    bufLen = RecvBufLen / 16;
    REG_UART_DMA_RX_ADDR0 = (unsigned short)(RecvAddr); //set receive buffer address
    REG_UART_DMA_RX_SIZE0 = bufLen; //set receive buffer size
}

/**
 * @brief This function determines whether parity error occurs once a packet arrives.
 * @return 1: parity error 0: no parity error
 */
unsigned char UART_IsParityError(void)
{
    return (REG_UART_STATUS & FLD_UART_STATUS_RX_ERR);
}

/**
 * @brief This function clears parity error status once when it occurs.
 * @return none
 */
void UART_ClearParityError(void)
{
    REG_UART_STATUS |= FLD_UART_STATUS_RX_ERR; //write 1 to clear
}

/**
 * @brief UART hardware flow control configuration. Configure RTS pin.
 * @param[in]   enable: enable or disable RTS function.
 * @param[in]   mode: set the mode of RTS(auto or manual).
 * @param[in]   thrsh: threshold of trig RTS pin's level toggle(only for auto mode), 
 *                     it means the number of bytes that has arrived in Rx buf.
 * @param[in]   invert: whether invert the output of RTS pin(only for auto mode)
 * @return none
 */
void UART_RTSCfg(unsigned char Enable, UART_RTSModeTypeDef Mode, unsigned char Thresh, unsigned char Invert)
{
#if USE_GPIO_SIMULATE_RTS
    //This case only supports manual mode
    if (Mode != UART_RTS_MODE_MANUAL) {
        while(1);
    }

    if (Enable) {
        GPIO_SetGPIOEnable(GPIOC_GP4, Bit_SET); //enable GPIOC_GP4 Pin's GPIO function
        GPIO_SetOutputEnable(GPIOC_GP4, Bit_SET); //enable GPIOC_GP4 output function
    }
    else {
        GPIO_SetGPIOEnable(GPIOC_GP4, Bit_RESET); //disable GPIOC_GP4 Pin's GPIO function
        GPIO_SetOutputEnable(GPIOC_GP4, Bit_RESET); //disable GPIOC_GP4 output function
    }
#else
    if (Enable) {
        GPIO_SetGPIOEnable(GPIOC_GP4, Bit_RESET); //disable GPIOC_GP4 Pin's GPIO function
        REG_UART_GPIO_DUPLEX |= BIT(4); //enable GPIOC_GP4 Pin as RTS Pin
        REG_UART_CTRL2 |= FLD_UART_CTRL2_RTS_EN; //enable RTS function
    }
    else {
        GPIO_SetGPIOEnable(GPIOC_GP4, Bit_SET); //enable GPIOC_GP4 Pin's GPIO function
        REG_UART_GPIO_DUPLEX &= (~BIT(4)); //disable GPIOC_GP4 Pin as RTS Pin
        REG_UART_CTRL2 &= (~FLD_UART_CTRL2_RTS_EN); //disable RTS function
    }

    if (Mode) {
        REG_UART_CTRL2 |= FLD_UART_CTRL2_RTS_MANUAL_EN;
    }
    else {
        REG_UART_CTRL2 &= (~FLD_UART_CTRL2_RTS_MANUAL_EN);
    }

    if (Invert) {
        REG_UART_CTRL2 |= FLD_UART_CTRL2_RTS_PARITY;
    }
    else {
        REG_UART_CTRL2 &= (~FLD_UART_CTRL2_RTS_PARITY);
    }

    //set threshold
    REG_UART_CTRL2 &= (~FLD_UART_CTRL2_RTS_TRIG_LVL);
    REG_UART_CTRL2 |= (Thresh & FLD_UART_CTRL2_RTS_TRIG_LVL);
#endif /*USE_GPIO_SIMULATE_RTS*/
}

/**
 * @brief This function sets the RTS pin's level manually
 * @param[in]   polarity: set the output of RTS pin(only for manual mode)
 * @return none
 */
void UART_RTSLvlSet(unsigned char Polarity)
{
#if USE_GPIO_SIMULATE_RTS
    if (Polarity) {
        GPIO_SetBit(GPIOC_GP4);
    }
    else {
        GPIO_ResetBit(GPIOC_GP4);
    }
#else
    if (Polarity) {
        REG_UART_CTRL2 |= FLD_UART_CTRL2_RTS_MANUAL_VAL;
    }
    else {
        REG_UART_CTRL2 &= (~FLD_UART_CTRL2_RTS_MANUAL_VAL);
    }
#endif
}

/**
 * @brief UART hardware flow control configuration. Configure CTS pin.
 * @param[in]   enable: enable or disable CTS function.
 * @param[in]   select: when CTS's input equals to select, tx will be stopped
 * @return none
 */
void UART_CTSCfg(unsigned char Enable, unsigned char Select)
{
    if (Enable) {
        GPIO_SetGPIOEnable(GPIOC_GP5, Bit_RESET); //disable GPIOC_GP5 Pin's GPIO function
        REG_UART_GPIO_DUPLEX |= BIT(5); //enable GPIOC_GP5 Pin as CTS Pin
        REG_UART_CTRL1 |= FLD_UART_CTRL1_CTS_EN; //enable CTS function
    }
    else {
        GPIO_SetGPIOEnable(GPIOC_GP5, Bit_SET); //enable GPIOC_GP5 Pin's GPIO function
        REG_UART_GPIO_DUPLEX &= (~BIT(5)); //disable GPIOC_GP5 Pin as CTS Pin
        REG_UART_CTRL1 &= (~FLD_UART_CTRL1_CTS_EN); //disable CTS function
    }

    if (Select) {
        REG_UART_CTRL1 |= FLD_UART_CTRL1_CTS_SELECT;
    }
    else {
        REG_UART_CTRL1 &= (~FLD_UART_CTRL1_CTS_SELECT);
    }
}
