#include "../../common.h"
#include "../../drivers.h"

void rf_rx_irq_handler(void);
void rf_tx_irq_handler(void);

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();

    if (IrqSrc & FLD_IRQ_ZB_RT_EN) {
        if (RfIrqSrc) {
            if (RfIrqSrc & FLD_RF_IRQ_RX) {
                rf_rx_irq_handler();
            }
            
            if (RfIrqSrc & FLD_RF_IRQ_TX) {
                rf_tx_irq_handler();
            }

            IRQ_RfIrqSrcClr();
        }
    }
    
    IRQ_SrcClr();
}



