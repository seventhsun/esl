#include "../../common.h"
#include "../../drivers.h"

#define    TX_TRIG_PIN             GPIOD_GP2
#define    TRANSCEIVER_IRQ_PIN     GPIOD_GP1

extern volatile unsigned char tx_trig_flag;
extern volatile unsigned char rx_trig_flag;

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();

    if (IrqSrc & FLD_IRQ_GPIO_EN) {
        if (0 == GPIO_ReadInputBit(TX_TRIG_PIN)) {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(TX_TRIG_PIN)) {
                while(0 == GPIO_ReadInputBit(TX_TRIG_PIN));
                tx_trig_flag = 1;
            }
        }

        if (0 == GPIO_ReadInputBit(TRANSCEIVER_IRQ_PIN)) {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(TRANSCEIVER_IRQ_PIN)) {
                while(0 == GPIO_ReadInputBit(TRANSCEIVER_IRQ_PIN));
                rx_trig_flag = 1;
            }
        } 
    }
    
    IRQ_SrcClr();
}



