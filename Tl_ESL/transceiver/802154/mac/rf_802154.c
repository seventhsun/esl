#include "mac_include.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */
#define MAC_FCF_ACK_REQ_BIT               0x20
#define MAC_FCF_INTRA_PAN_BIT             0x40
#define MAC_FCF_DST_ADDR_BIT              0x0c
#define MAC_FCF_FRAME_TYPE                0x07        


#define RF_DROP_REASON_INVALID_CRC        0x01
#define RF_DROP_REASON_RF_BUSY            0x02
#define RF_DROP_REASON_EXPECT_ACK         0x03
#define RF_DROP_REASON_INVALIC_BEACON     0x04
#define RF_DROP_REASON_FILTER_PANID       0x05
#define RF_DROP_REASON_FILTER_DSTADDR     0x06
#define RF_DROP_REASON_FILTER_LEN         0x07
#define RF_DROP_REASON_INVALIC_FRAME_TYPE 0x08


#define RF_PACKET_LENGTH_OK(p)            (p[0] == p[12]+13)
#define RF_PACKET_CRC_OK(p)               ((p[p[0]+3] & 0x51) == 0x10)
#define RF_ACTUAL_PAYLOAD_LEN(p)          (p[12])
#define RF_GET_TIMESTAMP(p)               (p[8] | (p[9]<<8) | (p[10]<<16) | (p[11]<<24))

/**********************************************************************
 * LOCAL VARIABLES
 */


/* Variables defined in the rf driver */
u8 *rf_rxBuf;
u8 *rf_rxBackupBuf;

rf_txCB_t  rf_txCbFunc;
rf_ackCB_t rf_ackCbFunc;

u8 _attribute_aligned_(4) rf_tx_buf[136];
u8 _attribute_aligned_(4) rf_ack_buf[12];

/*
 * Parameters for 802.15.4 RF filter
 */
u16 rf_shortAddr;
u8  rf_extAddr[8];
u16 rf_panID;
u8  rf_framePending;
u8  rf_isAck;
u8  rf_isCoord;

/* Used in the upper layer stack */
u8 rx_irq_busy = 0;
u8 T_drop;

void rf_setCBFunc(rf_txCB_t txCbFunc)
{
    rf_txCbFunc = txCbFunc;
}

void rf_setAckCB(rf_ackCB_t ackCbFunc)
{
    rf_ackCbFunc = ackCbFunc;
}

void rf_tx(u8 *buf, u8 len)
{
    // Dma length
    rf_tx_buf[0] = len+1;
    rf_tx_buf[1] = 0;
    rf_tx_buf[2] = 0;
    rf_tx_buf[3] = 0;

    // RF length
    rf_tx_buf[4] = len+2;

    // Payload
    memcpy(rf_tx_buf+5, buf, len);
    
    //trigger hardware transmission
    RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
    RF_TxPkt(rf_tx_buf); 
}


 /** 
 
 * @brief   reset the parameters using in the filter and ack buf
 
 * @param   none
 
 * @return  none
 
 */  
void rf_802154_reset(void)
{
    /*reset PHY parameters*/
    RF_PowerLevelSet(RF_POWER_7dBm);
    RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));

    /* Reset the parameters using in the filter */
    rf_shortAddr = 0xFFFE;
    rf_panID = 0;
    rf_framePending = 0;
    rf_isAck = 0;
    memset(rf_extAddr, 0, 8);

    /* Reset ack buf */
    memset(rf_ack_buf, 0, 12);
    rf_ack_buf[0] = 4;
    rf_ack_buf[4] = 5;
    rf_ack_buf[5] = 0x02;
    rf_ack_buf[6] = 0x00;
}

 /** 
 * @brief   reset the parameters using in the filter and ack buf
 *
 * @return  none
 */ 
void rf_802154_init(void)
{
    rf_802154_reset();

    /* IRQ configuration*/
    IRQ_DMAIrqDisable(FLD_DMA_IRQ_RF_RX | FLD_DMA_IRQ_RF_TX); //only use RF IRQ, don't use DMA IRQ
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqDisable(FLD_RF_IRQ_ALL);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX); //only enable RF_RX and RF_TX irq
    IRQ_Enable();

    /*DMA Channel reset*/
    DMA_ChannelDisable(FLD_DMA_CHANNEL_RF_RX | FLD_DMA_CHANNEL_RF_TX);
    DMA_ChannelEnable(FLD_DMA_CHANNEL_RF_RX | FLD_DMA_CHANNEL_RF_TX);
}
 /** 
 * @brief  	Call this function to set Parameters for 802.15.4 RF filter.
 *
 * @detail  The Parameters include shortAddr externAddr panId framePending and panCoord.
 *
 * @param[in]   id - Parameters that need to be setting
 *
 * @param[in]   pValue - Parameters is the value to be set
 *
 * @param[in]   len - Parameters is the len of the value to be set
 *
 * @return  none
 */
void rf_802154_set(u8 id, u8 *pValue, u8 len)
{
    switch(id) {
        
    case RF_ID_MAC_SHORT_ADDR:
        memcpy((u8*)&rf_shortAddr, pValue, 2);
        break;

    case RF_ID_MAC_EXT_ADDR:
        memcpy(rf_extAddr, pValue, 8);
        break;

    case RF_ID_MAC_PANID:
        memcpy((u8*)&rf_panID, pValue, 2);
        break;

    case RF_ID_MAC_FRAME_PENDING:
        rf_framePending = *pValue;
        break;
        
    case RF_ID_MAC_PAN_COORD:
        rf_isCoord = *pValue;
        break;
    }
}

/** 
 * @brief   Rx Interrupt handler for RF module
 *          TODO: Handle ACK request with highest priority
 *                Do the filter
 *                No wonder need to send ACK. Using the backup buffer now
 *                post the message to MAC layer
 *
 * @param   none
 *
 * @return  none
 */  


_attribute_ram_code_ __attribute__((optimize("-Os"))) void rf_rx_irq_handler(void)
{
    u8 *p = rf_rxBuf;
    int len = 0;
    int fNeedAck = 0;
    int fAck = 0;
    int fDrop = 0;
    u8 fcf1, fcf2;
    u8 rf_lastRssi;
    u16 dstPanid;
    u16 dstAddr;

    if ((!RF_PACKET_CRC_OK(p)) || (!RF_PACKET_LENGTH_OK(p))) {
        // Garbage packet
        *((u32 *)rf_rxBuf) = 0;
        T_drop = RF_DROP_REASON_INVALID_CRC;
        return;
    }

    /* Disable the dma during RX isr, because the memory may be changed by DMA*/
    DMA_ChannelDisable(FLD_DMA_CHANNEL_RF_RX);

    /* Parse necessary field to be used later */
    len = (int)RF_ACTUAL_PAYLOAD_LEN(p);
    fcf1 = *(p+13);    // frame control byte 1
    fcf2 = *(p+14);    // frame control byte 2

    /*----------------------------------------------------------
     *  Handle ACK request with highest priority
     */
    if (fcf1 & MAC_FCF_ACK_REQ_BIT) {
        fNeedAck = 1;
    }

    /*----------------------------------------------------------
     *  Do the filter
     */
    do {
        /*-------------------------------------------------------
         *  Check whether we are in the waiting ACK state
         *  TODO: need remove place
         */
        if (IS_MAC_WAITING_ACK()) {
            if ((fcf1 & MAC_FCF_FRAME_TYPE) != 0x02) {
                fDrop = 1;
                T_drop = RF_DROP_REASON_EXPECT_ACK;
            } 
            else {
                /* That's the ACK frame, length should be 5 */
                if (len != 5) {
                     fDrop = 1;
                     T_drop = RF_DROP_REASON_FILTER_LEN;
                } 
                else {
                    fAck = 1;
                }
            }
            break;
        }
        else if(rx_irq_busy == 1) {
            fDrop = 1;
            T_drop = RF_DROP_REASON_RF_BUSY;
            break;
        }
        if ((fcf1 & MAC_FCF_FRAME_TYPE) > 3) {
            fDrop = 1; // False frame_type
            T_drop = RF_DROP_REASON_INVALIC_FRAME_TYPE;
            break;
        }
        /* For Beacon packet, no filter should be applied */
        if ((fcf1 & MAC_FCF_FRAME_TYPE) == MAC_FRAME_TYPE_BEACON) {
            if ((fcf1 != 0x00) || ((fcf2 != 0x80) && (fcf2 != 0xC0))) {
                fDrop = 1; // False beacon packet
                T_drop = RF_DROP_REASON_INVALIC_BEACON;
            }
            break;
        }

        // panid
        dstPanid = *((u16*)(p+16));
        if (dstPanid != rf_panID && dstPanid != 0xffff) {
            fDrop = 1;
            T_drop = RF_DROP_REASON_FILTER_PANID;
            break;
        }

        // dst addr
        if ((fcf2 & MAC_FCF_DST_ADDR_BIT) == 0x08) {
            dstAddr = *((u16*)(p+18));
            if (dstAddr != 0xffff && dstAddr != rf_shortAddr ) {
                fDrop = 1;
                T_drop = RF_DROP_REASON_FILTER_DSTADDR;
                break;
            }
        } 
        else if ((fcf2 & MAC_FCF_DST_ADDR_BIT) == 0x0c) {
            if ( *((u16*)(p+18)) != *((u16*)(rf_extAddr)) ||
                 *((u16*)(p+20)) != *((u16*)(rf_extAddr+2)) ||
                 *((u16*)(p+22)) != *((u16*)(rf_extAddr+4)) ||
                 *((u16*)(p+24)) != *((u16*)(rf_extAddr+6)) ) {
                fDrop = 1;
                T_drop = RF_DROP_REASON_FILTER_DSTADDR;
                break;
            }
        } 
        else if ((fcf2 & MAC_FCF_DST_ADDR_BIT) == 0) {
            if ((((fcf1) & MAC_FCF_FRAME_TYPE) != 0) && (rf_isCoord == 0)) {
                fDrop = 1;
                T_drop = RF_DROP_REASON_FILTER_DSTADDR;
                break;
            }
        }
    } while (0);

    if (fDrop) {
        /* Drop the packet and recover the DMA */
        rf_rxBuf[12] = 0;
        *((u32*)rf_rxBuf) = 0;
        DMA_ChannelEnable(FLD_DMA_CHANNEL_RF_RX);
        return;
    }

    /*---------------------------------------------------------
     *  Handle ACK Frame
     */
    if (fAck) {
        if (rf_ackCbFunc) {
            rf_ackCbFunc(fcf1 & 0x10, *(p+15));
        }

        /* still use the rf_rxBuf to receive */
        rf_rxBuf[12] = 0;
        *((u32*)rf_rxBuf) = 0;
        DMA_ChannelEnable(FLD_DMA_CHANNEL_RF_RX);
        return;
    }

    /*----------------------------------------------------------
     *  Send ACK
     */
    if (fNeedAck) {
        DMA_ChannelEnable(FLD_DMA_CHANNEL_RF_TX);
        rf_ack_buf[7] = *(p+15);
        rf_ack_buf[5] |= (rf_framePending << 4);
        rf_isAck = 1;
        
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        RF_TxPkt(rf_ack_buf);
    }

    /*----------------------------------------------------------
     *  No wonder need to send ACK. Using the backup buffer now.
     */
    /* Get RSSI of the packet */
    rf_lastRssi = p[4];

    /* Use the backup buffer to receive next packet */
    rf_rxBuf = rf_rxBackupBuf;
    *((u32*)rf_rxBuf) = 0;
    rf_rxBuf[12] = 0;
    RF_RxBufferSet(rf_rxBuf, RF_RXBUF_MAX_SIZE, 0);
    DMA_ChannelEnable(FLD_DMA_CHANNEL_RF_RX);

    /*-------------------------------------------------------------------------------
     *  post the message to MAC layer
     */
    rx_buf_t* pRxEvt  = (rx_buf_t*)ev_buf_getTail(p, sizeof(rx_buf_t)); 
    rx_irq_busy = 1;
    // Save necessary informations
    pRxEvt->rxBuf     = p;
    pRxEvt->rssi      = rf_lastRssi;
    pRxEvt->fPending  = rf_framePending;
    pRxEvt->timestamp = RF_GET_TIMESTAMP(p);
    pRxEvt->len = len;
    buf_message_post((u8 *)p, BUF_ITEM_STATE_PHY2MAC);
}

/**
 * @brief   Tx Interrupt handler for RF module
 *          this function will change to RX mode first
 *
 * @param   none
 *
 * @return  none
 */  
_attribute_ram_code_ __attribute__((optimize("-Os"))) void rf_tx_irq_handler(void)
{
    /* Must change to RX mode first, otherwise the next ACK may miss */
    RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));

    //if the tx irq is triggered by the transmission of MAC Ack frame
    if (rf_isAck) {
        rf_isAck = 0;
        rf_ack_buf[5] = 0x02; //reset frame control byte0

    } 
    else {
        if (rf_txCbFunc) {
            rf_txCbFunc((void *)SUCCESS);
        }
    }
}
