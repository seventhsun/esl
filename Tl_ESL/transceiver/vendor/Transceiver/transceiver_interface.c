#include "../../drivers.h"
#include "../../common.h"
#include "transceiver_interface.h"

#define    GW_NOTIFY_PIN   GPIOD_GP1

#define    IO_BASE_ADDR    0x800000

#define    GW_PARA_BUF_SIZE   128
#define    GW_PARA_BUF_CNT    4
#define    GW_RESU_BUF_SIZE   128
#define    GW_RESU_BUF_CNT    4

unsigned char gw_para_buf[GW_PARA_BUF_SIZE][GW_PARA_BUF_CNT];
unsigned char gw_resu_buf[GW_RESU_BUF_SIZE][GW_RESU_BUF_CNT];

void ParaBuf_Init(void)
{
    WRITE_REG16(PARA_BUF_ADDR, gw_para_buf);
    WRITE_REG8(PARA_BUF_SIZE, GW_PARA_BUF_SIZE);
    WRITE_REG8(PARA_BUF_CNT, GW_PARA_BUF_CNT);
    WRITE_REG8(PARA_BUF_WPTR, 0);
    WRITE_REG8(PARA_BUF_RPTR, 0);
}

unsigned char *ParaBuf_Read(void)
{
    unsigned char wptr = READ_REG8(PARA_BUF_WPTR);
    unsigned char rptr = READ_REG8(PARA_BUF_RPTR);
    unsigned char *ret = NULL;

    if (wptr == rptr) {  //if buf is empty, return 0
        return NULL;
    }
    else {
        //adjust rptr
        ret = (unsigned char *)gw_para_buf;
        ret += GW_PARA_BUF_SIZE*rptr;
        WRITE_REG8(PARA_BUF_RPTR, (rptr+1)%GW_PARA_BUF_CNT);

        return ret;
    }
}

void ResuBuf_Init(void)
{
    WRITE_REG16(RESU_BUF_ADDR, gw_resu_buf);
    WRITE_REG8(RESU_BUF_SIZE, GW_RESU_BUF_SIZE);
    WRITE_REG8(RESU_BUF_CNT, GW_RESU_BUF_CNT);
    WRITE_REG8(RESU_BUF_WPTR, 0);
    WRITE_REG8(RESU_BUF_RPTR, 0);
}

unsigned int ResuBuf_Write(unsigned char *pSrcBuf, unsigned char len)
{
    unsigned char wptr = READ_REG8(RESU_BUF_WPTR);
    unsigned char rptr = READ_REG8(RESU_BUF_RPTR);

    if (((wptr+1)%GW_RESU_BUF_CNT) == rptr) { //if ResuBuf is full, do nothing and return 0
        return 0;
    }
    else {
        unsigned char *p = (unsigned char *)gw_resu_buf;
        p += GW_RESU_BUF_SIZE*wptr;
        memcpy(p, pSrcBuf, len);

        //adjust wptr
        WRITE_REG8(RESU_BUF_WPTR, (wptr+1)%GW_RESU_BUF_CNT);  
        
        return len;   
    }
}

void Interface_Init(void)
{
    //config SPI slave
    SPI_SlaveInit(0x0f, SPI_MODE0);
    SPI_SlavePinSelect(SPI_PIN_GPIOA);

    //config gateway notify pin
    GPIO_SetGPIOEnable(GW_NOTIFY_PIN, Bit_SET);    //set as gpio
    GPIO_SetOutputEnable(GW_NOTIFY_PIN, Bit_RESET); //disable output
    GPIO_ResetBit(GW_NOTIFY_PIN); //set output low
}

void GW_IrqTrig(void)
{
    GPIO_SetOutputEnable(GW_NOTIFY_PIN, Bit_SET); //enable output
    WaitUs(200);
    GPIO_SetOutputEnable(GW_NOTIFY_PIN, Bit_RESET); //disable output
}
