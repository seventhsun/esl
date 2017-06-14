#include "../../drivers.h"
#include "../../common.h"
#include "../../802154/mac/mac_include.h"
#include "transceiver_handler.h"

unsigned long firmwareVersion;

#define    CMD_LED_PIN     GPIOD_GP7
#define    TRIG_LED_PIN    GPIOB_GP4

static void SYS_Init(void)
{
    BSP_SysCtlTypeDef SysCtrl;
    SysCtrl.rst0 = (~FLD_RST0_ALL);
    SysCtrl.rst1 = (~FLD_RST1_ALL);
    SysCtrl.rst2 = (~FLD_RST2_ALL);
    SysCtrl.clk0 = FLD_CLK0_EN_ALL;
    SysCtrl.clk1 = FLD_CLK1_EN_ALL;
    SysCtrl.clk2 = FLD_CLK2_EN_ALL;
    SysInit(&SysCtrl);
    SysClockInit(SYS_CLK_HS_DIV, 6);
    RF_Init(RF_OSC_12M, RF_MODE_ZIGBEE_250K);
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(3000);
}

static void UserInit(void)
{
    //config the LED pins
    GPIO_SetGPIOEnable(CMD_LED_PIN, Bit_SET);    //set as gpio
    GPIO_ResetBit(CMD_LED_PIN);
    GPIO_SetOutputEnable(CMD_LED_PIN, Bit_SET);

    //config the LED pins
    GPIO_SetGPIOEnable(TRIG_LED_PIN, Bit_SET);    //set as gpio
    GPIO_ResetBit(TRIG_LED_PIN);
    GPIO_SetOutputEnable(TRIG_LED_PIN, Bit_SET);
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();
    UserInit();
    Interface_Init();

    ParaBuf_Init();
    ResuBuf_Init();

    //802.15.4 stack Init
    ev_buf_init();
    mac_init();
    Transceiver_MACInit();
    UpperLayerCallbackSet(CALLBACK_DATA_INDICATION, Transceiver_DataIndCb);

    // SendTestTimerStart();
    while (1) {
        stack_mainloop();
        UpperLayerProcess();
        Transceiver_Mainloop();
    }
}




