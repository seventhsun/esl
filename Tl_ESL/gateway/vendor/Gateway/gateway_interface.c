#include "../../drivers.h"
#include "../../common.h"
#include "gateway_interface.h"
#include "drivers.h"

/*********************************************************************
0x808004    0x808005    0x808006    0x808007    0x808008    0x808009
ParaAddr_L  ParaAddr_H  fifo_size   fifo_cnt    wptr        rptr
0x80800a    0x80800b    0x80800c    0x80800d    0x80800e    0x80800f
ResAddr_L   ResAddr_H   fifo_size   fifo_cnt    wptr        rptr
**********************************************************************/
#define    PARA_BUF_ADDR_PTR       0x8004
#define    PARA_BUF_SIZE_PTR       0x8006
#define    PARA_BUF_CNT_PTR        0x8007
#define    PARA_BUF_WPTR_PTR       0x8008
#define    PARA_BUF_RPTR_PTR       0x8009

#define    RESU_BUF_ADDR_PTR       0x800a
#define    RESU_BUF_SIZE_PTR       0x800c
#define    RESU_BUF_CNT_PTR        0x800d
#define    RESU_BUF_WPTR_PTR       0x800e
#define    RESU_BUF_RPTR_PTR       0x800f

#define    TRANSCEIVER_CMD_READ    0x80
#define    TRANSCEIVER_CMD_WRITE   0x00

#define    TRANSCEIVER_CS_PIN      GPIOA_GP5
#define    TRANSCEIVER_IRQ_PIN     GPIOD_GP1

static void Transceiver_Read(unsigned short addr, unsigned char *pDst, int len)
{
    unsigned char tmp[3];

    tmp[0] = addr >> 8;
    tmp[1] = addr & 0xff;
    tmp[2] = TRANSCEIVER_CMD_READ;

    SPI_Read(tmp, 3, pDst, len, TRANSCEIVER_CS_PIN);
}

static void Transceiver_Write(unsigned short addr, unsigned char *pDst, int len)
{
    unsigned char tmp[3];

    tmp[0] = addr >> 8;
    tmp[1] = addr & 0xff;
    tmp[2] = TRANSCEIVER_CMD_WRITE;

    SPI_Write(tmp, 3, pDst, len, TRANSCEIVER_CS_PIN);
}

void Interface_Init(void)
{
    //initialize the SPI interface
    SPI_MasterPinSelect(SPI_PIN_GPIOA);
    SPI_MasterCSPinSelect(TRANSCEIVER_CS_PIN);
    SPI_MasterInit(0x0f, SPI_MODE0);

    //config the GPIO irq pin
    GPIO_SetGPIOEnable(TRANSCEIVER_IRQ_PIN, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(TRANSCEIVER_IRQ_PIN, Bit_SET);   //enable input
    GPIO_PullSet(TRANSCEIVER_IRQ_PIN, PULL_UP_1M);       //pull up
    GPIO_SetInterrupt(TRANSCEIVER_IRQ_PIN, Bit_SET);     //falling edge trig irq
    IRQ_EnableType(FLD_IRQ_GPIO_EN);
}
static unsigned char tmpBuf1[6];
unsigned int ResBuf_Read(unsigned char *pDestBuf, unsigned char len)
{
    unsigned char i,tmpBuf[6];

    Transceiver_Read(RESU_BUF_ADDR_PTR, tmpBuf, sizeof(tmpBuf));
    unsigned short BufAddr = tmpBuf[0] + (tmpBuf[1]<<8);
    unsigned char BufSize = tmpBuf[2];
    unsigned char BufCnt = tmpBuf[3];
    unsigned char wptr = tmpBuf[4];
    unsigned char rptr = tmpBuf[5];


    if (wptr == rptr) { //if ResuBuf is empty, do nothing and return 0
        return 0;
    }
    else {
        unsigned short ReadAddr = BufAddr + BufSize*rptr;
        //for(i=0;i<6;i++)
        tmpBuf1[0]=ReadAddr>>8;
        tmpBuf1[1]=ReadAddr&0x00ff;
        Transceiver_Read(ReadAddr, pDestBuf, len);
        /*update rptr*/
        rptr = (rptr+1) % BufCnt;
        Transceiver_Write(RESU_BUF_RPTR_PTR, &rptr, sizeof(rptr));
        return len;
    }
}

unsigned int ParaBuf_Write(unsigned char *pSrcBuf, unsigned char len)
{
    unsigned char i,tmpBuf[6];

    Transceiver_Read(PARA_BUF_ADDR_PTR, tmpBuf, sizeof(tmpBuf));
    unsigned short BufAddr = tmpBuf[0] + (tmpBuf[1]<<8);
    unsigned char BufSize = tmpBuf[2];
    unsigned char BufCnt = tmpBuf[3];
    unsigned char wptr = tmpBuf[4];
    unsigned char rptr = tmpBuf[5];

    if (((wptr+1)%BufCnt) == rptr) { //if ParaBuf is full, do nothing and return 0

        return 0;
    }
    else {
        unsigned short WriteAddr = BufAddr + BufSize*wptr;
        Transceiver_Write(WriteAddr, pSrcBuf, len);
        /*update wptr*/
        wptr = (wptr+1) % BufCnt;
        Transceiver_Write(PARA_BUF_WPTR_PTR, &wptr, sizeof(wptr));
        return len;
    }
}
