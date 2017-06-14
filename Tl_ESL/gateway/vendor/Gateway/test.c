#include "../../drivers.h"
#include "../../common.h"
#include "test.h"
#include "gateway_frame.h"

#define    TX_TRIG_PIN    GPIOD_GP2
#define    TX_LED_PIN     GPIOD_GP5
#define    RX_LED_PIN     GPIOD_GP7
#define    INDI_LED_PIN   GPIOB_GP4

void Blink_LED(unsigned short pin, unsigned char times)
{
    while (times--) {
        GPIO_SetBit(pin);
        WaitMs(10);
        GPIO_ResetBit(pin);
        WaitMs(40);
    }
}

static int Test_ReadRssiDoneHandler(unsigned char *data)
{
    LogMsg("ReadRssiDone", &data[2], 1);

    return 0;
}

static int Test_SetMacDoneHandler(unsigned char *data)
{
    data = data;

    LogMsg("SetMacDone", NULL, 0);

    return 0;
}

static int Test_SetPanIDDoneHandler(unsigned char *data)
{
    data = data;

    LogMsg("SetPanIDDone", NULL, 0);

    return 0;
}

static int Test_SetChannelDoneHandler(unsigned char *data)
{
    data = data;

    LogMsg("SetChannelDone", NULL, 0);

    return 0;
}

static int Test_SetPowerDoneHandler(unsigned char *data)
{
    data = data;

    LogMsg("SetPowerDone", NULL, 0);

    return 0;
}

static int Test_TxPacketDoneHandler(unsigned char *data)
{
    data = data;

    LogMsg("TxPacketDone", NULL, 0);

    return 0;
}

static int Test_RxPacketHandler(unsigned char *data)
{
    unsigned char len = data[1];

    LogMsg("data:", &data[2], len);

    return 0;
}

void Test_Init(void)
{
    Gateway_SetHandlers(Test_ReadRssiDoneHandler,
                        Test_SetMacDoneHandler,
                        Test_SetPanIDDoneHandler,
                        Test_SetChannelDoneHandler,
                        Test_SetPowerDoneHandler,
                        Test_TxPacketDoneHandler,
                        Test_RxPacketHandler);
}

static unsigned short dst_addr = 0x0001;
static unsigned short dst_pan_id = 0xaabb;
static unsigned int payload_data = 0;
void Test_SendData(void)
{
    payload_data++;
    Blink_LED(TX_LED_PIN, 1);
    Gateway_TxPacket(dst_addr, dst_pan_id, &payload_data, sizeof(payload_data));
//    Gateway_SetPower(20);
//    Gateway_SetPanID(0x1234);

}

void Test_ProcessData(void)
{
    Blink_LED(RX_LED_PIN, 1);
    Gateway_IrqHandler();
}

