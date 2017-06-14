#include "irq.h"



unsigned char IRQ_Enable(void)
{
    unsigned char r = REG_IRQ_EN;      // don't worry,  the compiler will optimize the return value if not used
    REG_IRQ_EN = 1;
    return r;
}

unsigned char IRQ_Disable(void)
{
    unsigned char r = REG_IRQ_EN;      // don't worry,  the compiler will optimize the return value if not used
    REG_IRQ_EN = 0;
    return r;
}

void IRQ_Restore(unsigned char Enable)
{
    REG_IRQ_EN = Enable;
}

unsigned int IRQ_MaskGet(void)
{
    return REG_IRQ_MASK;
}

void IRQ_MaskSet(unsigned int Mask)
{
    SET_BIT_FLD(REG_IRQ_MASK, Mask);
}

void IRQ_MaskClr(unsigned int Mask)
{
    CLR_BIT_FLD(REG_IRQ_MASK, Mask);
}

unsigned int IRQ_SrcGet(void)
{
    return REG_IRQ_SRC;
}

void IRQ_SrcClr(void)
{
    REG_IRQ_SRC = 0xffffffff;
}

void IRQ_EnableType(unsigned int TypeMask)
{
    IRQ_MaskSet(TypeMask);
}

void IRQ_DisableType(unsigned int TypeMask)
{
    IRQ_MaskClr(TypeMask);
}

//RF module irq
#define REG_RF_IRQ_MASK         REG_ADDR16(0xf1c)
#define REG_RF_IRQ_STATUS       REG_ADDR16(0xf20)

void IRQ_RfIrqEnable(unsigned int RfIrqMask)
{
    REG_RF_IRQ_MASK |= RfIrqMask;
}

void IRQ_RfIrqDisable(unsigned int RfIrqMask)
{
    REG_RF_IRQ_MASK &= (~RfIrqMask);
}

unsigned short IRQ_RfIrqSrcGet(void)
{
    return REG_RF_IRQ_STATUS;
}

void IRQ_RfIrqSrcClr()
{
    REG_RF_IRQ_STATUS = 0xffff;
}

//SPI module irq
#define REG_SPI_IRQ_STATUS       REG_ADDR8(0x21)
#define REG_SPI_IRQ_CLR_STATUS   REG_ADDR8(0x22)

void IRQ_SpiIrqEn(void)
{
	REG_IRQ_MASK |= FLD_IRQ_HOST_CMD_SRC;
}

void IRQ_SpiIrqDis(void)
{
	REG_IRQ_MASK &= (~FLD_IRQ_HOST_CMD_SRC);
}

unsigned char IRQ_SpiIrqSrcGet(void)
{
    return REG_SPI_IRQ_STATUS;
}

void IRQ_SpiIrqSrcClr(void)
{
    REG_SPI_IRQ_CLR_STATUS = FLD_SPI_IRQ_HOST_CMD | FLD_SPI_IRQ_HOST_RD_TAG;
}

//UART module irq
#define REG_UART_IRQ_MASK         REG_ADDR8(0x521)
#define REG_UART_IRQ_STATUS       REG_ADDR8(0x526)
#define REG_UART_IRQ_CLR_STATUS   REG_ADDR8(0x526)

void IRQ_UartDmaIrqEnable(unsigned int UARTIrqMask)
{
    if (UARTIrqMask) {
        if (UARTIrqMask & FLD_UART_IRQ_RX) {
            REG_UART_IRQ_MASK |= FLD_UART_IRQ_RX; //open dma1 interrupt mask
        }

        if (UARTIrqMask & FLD_UART_IRQ_TX) {
            REG_UART_IRQ_MASK |= FLD_UART_IRQ_TX; //open dma1 interrupt mask
        }
    }
    REG_IRQ_MASK |= FLD_IRQ_DMA_SRC;
}

void IRQ_UartDmaIrqDisable(unsigned int UARTIrqMask)
{
    if (UARTIrqMask & FLD_UART_IRQ_RX) {
        REG_UART_IRQ_MASK &= ~FLD_UART_IRQ_RX; //close dma1 interrupt mask
    }

    if (UARTIrqMask & FLD_UART_IRQ_TX) {
        REG_UART_IRQ_MASK &= ~FLD_UART_IRQ_TX; //close dma1 interrupt mask
    }
    if((UARTIrqMask & (FLD_UART_IRQ_RX|FLD_UART_IRQ_TX)) == (FLD_UART_IRQ_RX|FLD_UART_IRQ_TX)){
    	REG_IRQ_MASK &= (~FLD_IRQ_DMA_SRC);
    }
}

unsigned char IRQ_UartDmaIrqSrcGet(void)
{
    return REG_UART_IRQ_STATUS;
}

void IRQ_UartDmaIrqSrcClr(void)
{
    REG_UART_IRQ_CLR_STATUS |= (FLD_UART_IRQ_RX | FLD_UART_IRQ_TX);  // set to clear
}

///I2C module irq
#define  REG_I2C_IRQ_STATUS       REG_ADDR8(0x21)
#define  REG_I2C_IRQ_CLR_STATUS   REG_ADDR8(0x22)

void IRQ_I2CIrqEnable(void)
{
	REG_IRQ_MASK |= FLD_IRQ_HOST_CMD_EN;
}
void IRQ_I2CIrqDisable(void)
{
	REG_IRQ_MASK &= (~FLD_IRQ_HOST_CMD_EN);
}

I2C_I2CIrqSrcTypeDef I2C_SlaveIrqGet(void){
	unsigned char hostStatus = REG_I2C_IRQ_STATUS;
	if(hostStatus&FLD_I2C_IRQ_HOST_RD_TAG){
		return I2C_IRQ_HOST_READ_ONLY;
	}
	else if(hostStatus&FLD_I2C_IRQ_HOST_CMD){
		////the bit actually indicate read and write,but because the "return read_only"is before "read_write",
		////so if return"read_write" indicate write only
		return I2C_IRQ_HOST_WRITE_ONLY;
	}
	else{
		return I2C_IRQ_NONE;
	}
}

void I2C_SlaveIrqClr(I2C_I2CIrqSrcTypeDef src){
	if(src==I2C_IRQ_HOST_READ_ONLY){
		REG_I2C_IRQ_CLR_STATUS |= (FLD_I2C_IRQ_HOST_CMD|FLD_I2C_IRQ_HOST_RD_TAG);
	}
	else if(src==I2C_IRQ_HOST_WRITE_ONLY){
		REG_I2C_IRQ_CLR_STATUS |= FLD_I2C_IRQ_HOST_CMD;
	}
	else{
	}
}

#define REG_DMA_IRQ_MASK    REG_ADDR8(0x521)

void IRQ_DMAIrqEnable(unsigned int DMAIrqMask)
{
    REG_DMA_IRQ_MASK |= DMAIrqMask;
}

void IRQ_DMAIrqDisable(unsigned int DMAIrqMask)
{
    REG_DMA_IRQ_MASK &= ~DMAIrqMask;
}




