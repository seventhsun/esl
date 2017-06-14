#include "../../drivers.h"
#include "../../common.h"
#include "gateway_interface.h"
#include "test.h"

unsigned long firmwareVersion;

#define    TX_TRIG_PIN    GPIOD_GP2
#define    TX_LED_PIN     GPIOD_GP5
#define    RX_LED_PIN     GPIOD_GP7
#define    INDI_LED_PIN   GPIOB_GP4

volatile unsigned char tx_trig_flag = 0;
volatile unsigned char rx_trig_flag = 0;


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
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(3000);
}

static void UserInit(void)
{
    //config the LED pins
    GPIO_SetGPIOEnable(TX_LED_PIN | RX_LED_PIN, Bit_SET);    //set as gpio
    GPIO_ResetBit(TX_LED_PIN);
    GPIO_ResetBit(RX_LED_PIN);
    GPIO_SetOutputEnable(TX_LED_PIN | RX_LED_PIN, Bit_SET);   //enable input

    //config Indicating LED pins
    GPIO_SetGPIOEnable(INDI_LED_PIN, Bit_SET);    //set as gpio
    GPIO_ResetBit(INDI_LED_PIN);
    GPIO_SetOutputEnable(INDI_LED_PIN, Bit_SET);   //enable input

    //config the Tx trig pin
    GPIO_SetGPIOEnable(TX_TRIG_PIN, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(TX_TRIG_PIN, Bit_SET);   //enable input
    GPIO_PullSet(TX_TRIG_PIN, PULL_UP_1M);       //pull up
    GPIO_SetInterrupt(TX_TRIG_PIN, Bit_SET);     //falling edge trig irq
    IRQ_EnableType(FLD_IRQ_GPIO_EN);
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();
    UserInit();
    Interface_Init();
    Test_Init();
    IRQ_Enable();

    Blink_LED(INDI_LED_PIN, 3);

    while (1) {
        if (tx_trig_flag) {
            tx_trig_flag = 0;
            Test_SendData();
        }

        if (rx_trig_flag) {
            rx_trig_flag = 0;
            Test_ProcessData();
        }
    }
}




