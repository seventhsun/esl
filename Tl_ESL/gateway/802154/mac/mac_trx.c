#include "mac_trx.h"
#define USE_TELINK_MAC          1

#define MAC_CSMA_BACK_OFF() macPib.maxCsmaBackoffs
#define MAC_RX_ON_WHEN_IDLE() macPib.rxOnWhenIdle

/**********************************************************************
 * LOCAL CONSTANTS
 */
#define MAC_TRX_TX_EVT_MASK     0x80
#define MAC_TRX_RX_EVT_MASK     0x40


/**********************************************************************
 * TYPEDEFS
 */
typedef struct {
    u8 *                 retryBuf;
    mac_genericFrame_t*  pCurData;
    //for CSMA-CA algorithm
    u8                   backoffCnt;
    u8                   be;
    //for retry transmission
    u8                   retryCnt;
    u8                   txSeq;
    mac_txState_t        txState;
} mac_trxCtrl_t;
mac_trxCtrl_t mac_trx_vars;

mac_txInfo_t txStatiInfo = {0, 0, 0, 0, 0, 0, 0};

static ev_time_event_t *softAckCheckTimer;
static ev_time_event_t *rf_txIrqCheckTimer = NULL;
static ev_time_event_t *mac_ackWaitTimer = NULL;
static ev_time_event_t *mac_csmaDlyTimer = NULL;


/** @brief  MAC TRX state machine */
typedef struct
{
    mac_txState_t curState;          /*! The MAC State in which the event handler can be used */
    u8 txEvt;                        /*! The event to decide which TX operation be triggered */
    mac_txState_t nxtState;          /*! The MAC State in which the event handler can be used */
    mac_evHandler_t evHandlerFunc;   /*! The corresponding event handler */
} mac_txStateMachine_t;

void mac_rxInit(void);
void mac_rxCb(u8 *pData, u8 len, u32 timestamp, u8 lqi);
void mac_txCb(void* status);
void mac_doTx(u8 *pData);
void mac_sendTxCnf(u8 *pData);
void mac_txBusyHandler(u8 *pData);
void mac_ackHandler(u8 *pData);
void mac_csmaTx(u8 *pData);
void mac_ackStartTimer(u8* pData);
int mac_csmaDelayCb(void* arg);

/**********************************************************************
 * STATE MACHINE DEFINES
 */
static const mac_txStateMachine_t mac_txStateMachine[] = {
    /*  current state,      event,                     next state,              action    */
    { MAC_TX_IDLE,      MAC_TX_EV_NEW_DATA,          MAC_TX_CSMA,             mac_csmaTx},
    { MAC_TX_CSMA,      MAC_TX_EV_CSMA_IDLE,         MAC_TX_UNDERWAY,         mac_doTx}, // In macDoTx, it will change the data to MAX_TX_WAITING_ACK or MAC_TX_UNDERWAY
    { MAC_TX_CSMA,      MAC_TX_EV_CSMA_BUSY,         MAC_TX_IDLE,             mac_sendTxCnf},
    { MAC_TX_CSMA,      MAC_TX_EV_NEW_DATA,          MAC_TX_CSMA,             mac_txBusyHandler},

    { MAC_TX_UNDERWAY,  MAC_TX_EV_SEND_SUCC,         MAC_TX_IDLE,             mac_sendTxCnf},
    { MAC_TX_UNDERWAY,  MAC_TX_EV_SEND_FAIL,         MAC_TX_IDLE,             mac_sendTxCnf},
    { MAC_TX_UNDERWAY,  MAC_TX_EV_NEW_DATA,          MAC_TX_UNDERWAY,         mac_txBusyHandler},

    { MAC_TX_RETRY,     MAC_TX_EV_ACK_RETRY,         MAC_TX_CSMA,             mac_csmaTx},

    { MAC_TX_WAIT_ACK,  MAC_TX_EV_ACK_RECV,          MAC_TX_IDLE,             mac_ackHandler},
    { MAC_TX_WAIT_ACK,  MAC_TX_EV_NO_ACK,            MAC_TX_IDLE,             mac_sendTxCnf},

    { MAC_TX_TIME_OUT,  MAC_TX_EV_SEND_FAIL,         MAC_TX_IDLE,             mac_sendTxCnf},

    { MAC_TX_WAIT_ACK,  MAC_TX_EV_NEW_DATA,          MAC_TX_WAIT_ACK,         mac_txBusyHandler},
    { MAC_TX_WAIT_ACK,  MAC_TX_EV_SEND_SUCC,         MAC_TX_WAIT_ACK,         mac_ackStartTimer},
};


 /** 
 * @brief      Call this function to reset the mac_trx_vars
 *
 * @param   none
 *
 * @return  none
 */
void mac_trxReset(void)
{
    mac_trx_vars.txState = MAC_TX_IDLE;

    if (mac_trx_vars.retryBuf) {
        if (SUCCESS != ev_buf_free(mac_trx_vars.retryBuf)) {
            while(1);
        }
        mac_trx_vars.retryBuf = NULL;
    }
}

 /** 
 * @brief      Call this function to initialize the mac_trx_var
 *
 * @param   none
 *
 * @return  none
 */
void mac_trxInit(void)
{
    memset(&mac_trx_vars, 0, (sizeof(mac_trxCtrl_t)));
    mac_trxReset();

    rf_setCBFunc(mac_txCb);
    rf_setAckCB(mac_ackCb);
    mac_rxInit();
    rf_802154_init();
}


void mac_tx_stateMachine(mac_txEvt_t evt, mac_genericFrame_t* pData)
{
    int size, i;

    size = sizeof(mac_txStateMachine) / sizeof(mac_txStateMachine_t);

    /* Search an appropriate event handler */
    for (i = 0; i < size; i++) {
        if (mac_txStateMachine[i].curState == mac_trx_vars.txState &&
            mac_txStateMachine[i].txEvt == evt) {
            /* set tx state to next state */
            mac_trx_vars.txState = mac_txStateMachine[i].nxtState;

            /* call event handler */
            if (mac_txStateMachine[i].evHandlerFunc) {
                mac_txStateMachine[i].evHandlerFunc((u8*)pData);
            }
            return;
        }
    }
}

void mac_rx_handler(mac_genericFrame_t* pData)
{
    u8 cmdId;
    mac_beaconNotifyInd_t *pBeaconInd;
    u8 seqNo;
    u8 lqi;
    u32 timeStamp;

    /* Command frame handler */
    switch (pData->data.rx.frameType & MAC_FCF_FRAME_TYPE_MASK) {
    case MAC_FRAME_TYPE_BEACON:
        /* Beacon Frame Handler */
        pBeaconInd = (mac_beaconNotifyInd_t*)pData;
        pBeaconInd->primitive = MAC_MLME_BEACON_NOTIFY_IND;
        break;

    case MAC_FRAME_TYPE_DATA:
        seqNo = pData->data.rx.dsn;
        lqi = pData->data.rx.lqi;
        timeStamp = pData->data.rx.timestamp;
        pData->primitive = MAC_MCPS_DATA_IND;
        *((u8*)(pData)+1) = seqNo;
        mac_dataInd_t *macData = (mac_dataInd_t *)pData;
        macData->lqi = lqi;
        macData->timestamp = timeStamp;
        break;

    case MAC_FRAME_TYPE_COMMAND:
        /* Get command id */
        cmdId = *pData->msdu;
        switch (cmdId) {
        case MAC_BEACON_REQ_FRAME:
            pData->primitive = MAC_MLME_BEACON_REQ_CMD;
            break;

        case MAC_ASSOC_REQ_FRAME:
            pData->primitive = MAC_MLME_ASSOCIATE_IND;
            break;

        case MAC_ASSOC_RSP_FRAME:
            pData->primitive = MAC_MLME_ASSOCIATE_RES;
            break;

        case MAC_DATA_REQ_FRAME:
            pData->primitive = MAC_MLME_DATA_REQ_CMD;
            break;

        case MAC_DISASSOC_NOTIF_FRAME:
            pData->primitive = MAC_MLME_DISASSOCIATE_IND;
            break;

        default:
            break;
        }
        break;

    default:
        ev_buf_free((u8*)pData);
        return;
    }

    /* Send Frame to upper layer */
    mac_runStateMachine((mac_generalReq_t *)pData, PHY_2_MAC_TYPE);
}

 /** 
 * @brief   This function is called by upper layer when there is data to send.
 *          Firstly, it will check whether the transcevier is idle to send
 *          a new packet. If not, currently it will return as MAC_TX_ACTIVE
 *          state. If idle, send a NEW DATA to the transceiver task.
 *          TODO: Using the queue to buffer the data in Transceiver layer.
 *
 * @param   pData - New data to be sent from upper layer
 *
 * @return  Status of the operation
 */ 
mac_sts_t mac_tx(mac_genericFrame_t *pData)
{
    if (MAC_TX_IDLE != mac_trx_vars.txState) {
        return MAC_TX_ACTIVE;
    }

    if (!buf_message_post((u8 *)pData, BUF_ITEM_STATE_MAC2CSMA)) {
        return MAC_SUCCESS;
    }
    else {
        return MAC_INVALID_PARAMETER;
    }
}

void process_Mac2CSMA(void)
{
    u8 *pData = NULL;
    pData = buf_message_poll(BUF_ITEM_STATE_MAC2CSMA);
    
    if (pData) {
        mac_tx_stateMachine(MAC_TX_EV_NEW_DATA, (mac_genericFrame_t*)pData);
    }
}

static u8 rf_performCCA(void)
{
    u32 t1 = ClockTime();
    s8 rssi_peak = -110;
    s8 rssi_cur = -110;

    while(!ClockTimeExceed(t1, 60)) {
        rssi_cur = RF_GetRssi();
        if (rssi_cur > rssi_peak) {
            rssi_peak = rssi_cur;
        }
    }
    if (rssi_peak > CCA_THRESHOLD) {
        return 0x00;
    } 
    else {
        return 0x04;
    }
}

/**
 * @brief   This function is the timer callback function of CSMA.
 *          It estimate the channel state and decide to do the CSMA again
 *          or directly send the data.
 * @param   arg - The argument of the callback function.
 *
 * @return  -1. The timer only run once after called.
 */
int mac_csmaDelayCb(void *arg)
{
    phy_ccaSts_t ccaStatus;
    u32 newTimeout;

    if (mac_csmaDlyTimer) {
        mac_csmaDlyTimer = NULL;
    }

    /* Perform CCA */
    ccaStatus = (phy_ccaSts_t)rf_performCCA();
    if (ccaStatus == PHY_CCA_IDLE) {
        /* channel idle, Store the data pointer for further confirm reuse */
        if (mac_trx_vars.pCurData) {
            ev_buf_free((u8*)mac_trx_vars.pCurData);
        }
        mac_trx_vars.pCurData = (mac_genericFrame_t*)arg;
        mac_trx_vars.pCurData->data.tx.cnfStatus = MAC_SUCCESS;
        mac_tx_stateMachine(MAC_TX_EV_CSMA_IDLE, mac_trx_vars.pCurData);
    } 
    else {
        if(MAC_CSMA_BACK_OFF() != mac_trx_vars.backoffCnt++) {
            /* Do the delay again */
            mac_trx_vars.be = (mac_trx_vars.be + 1 > macPib.maxBe) ? macPib.maxBe : mac_trx_vars.be+1;
            newTimeout = ((Rand()) & ((1<<mac_trx_vars.be)-1)) * MAC_SPEC_USECS_PER_BACKOFF;
            if (mac_csmaDlyTimer) {
                ev_unon_timer(&mac_csmaDlyTimer);
            }
            mac_csmaDlyTimer = ev_on_timer(mac_csmaDelayCb, arg, newTimeout);
        } 
        else {
            /* CSMA Failure */
            mac_trx_vars.pCurData = (mac_genericFrame_t*)arg;
            mac_trx_vars.pCurData->data.tx.cnfStatus = MAC_CHANNEL_ACCESS_FAILURE;
            mac_tx_stateMachine(MAC_TX_EV_CSMA_BUSY, mac_trx_vars.pCurData);
            mac_trx_vars.pCurData = NULL;
        }
    }

    return -1;
}

/**
 * @brief   This function send the new data through CSMA method.
 *
 * @param   pData - The argument of the callback function.
 *
 * @return  -1. The timer only run once after called.
 */
void mac_csmaTx(u8 *pData)
{
    u32 timeout;

    if (rf_txIrqCheckTimer) {
        ev_unon_timer(&rf_txIrqCheckTimer);
    }
    rf_txIrqCheckTimer = NULL;

    if (softAckCheckTimer) {
        ev_unon_timer(&softAckCheckTimer);
    }
    softAckCheckTimer = NULL;

    /* Init backoff count number and be */
    mac_trx_vars.backoffCnt = 0;
    mac_trx_vars.be = macPib.minBe;

    /* Delay for random 2^BE-1 unit backoff periods */
    timeout = ((Rand()) & ((1<<mac_trx_vars.be)-1)) * MAC_SPEC_USECS_PER_BACKOFF;
    if (mac_csmaDlyTimer) {
        ev_unon_timer(&mac_csmaDlyTimer);
    }
    mac_csmaDlyTimer = ev_on_timer(mac_csmaDelayCb, (void*)pData, timeout);
}

/**
 * @brief   This function is the timer callback function when NO ACK
 *          received. Here it will decide to retry transmit or report
 *          transimit failure according to the retry counter.
 *          After retry success, if the RX_ON_WHEN_IDLE field is set to
 *          0, we will close the RF to save power.
 *
 * @param   arg - The argument of the timer callback function.
 *
 * @return  -1. The timer only run once after called.
 */
int mac_ackWaitingTimerCb(void* arg)
{
    u8 r = IRQ_Disable();

    if (MAC_RX_ON_WHEN_IDLE() == 0) {
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
    }

    if (mac_trx_vars.retryCnt--) {
        mac_trx_vars.txState = MAC_TX_RETRY;
        mac_tx_stateMachine(MAC_TX_EV_ACK_RETRY, mac_trx_vars.pCurData);
    } 
    else {
        /* failure, send confirm as MAC_NO_ACK */
        if (softAckCheckTimer) {
            ev_unon_timer(&softAckCheckTimer);
        }
        mac_trx_vars.retryBuf = NULL;
        mac_trx_vars.pCurData->data.tx.cnfStatus = MAC_NO_ACK;
        mac_tx_stateMachine(MAC_TX_EV_NO_ACK, mac_trx_vars.pCurData);
        mac_trx_vars.pCurData = NULL;
    }

    mac_ackWaitTimer = NULL;
    IRQ_Restore(r);
    return -1;
}


/**
 * @brief   This function is the timer callback function when TX Irq not
 *          come. Here it will decide to retry transmit or report
 *          transimit failure according to the retry counter.
 *          After retry success, if the RX_ON_WHEN_IDLE field is set to
 *          0, we will close the RF to save power.
 *
 * @param   arg - The argument of the timer callback function.
 *
 * @return  -1. The timer only run once after called.
 */
int mac_waitTxIrqCb(void* arg)
{
    u8 r = IRQ_Disable();
    /* set rf the rx mode */
    RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
    mac_trx_vars.retryBuf = NULL;
    mac_trx_vars.pCurData->data.tx.cnfStatus = MAC_TX_ABORTED;
    
    if (MAC_TX_WAIT_ACK == mac_trx_vars.txState) {
        mac_trx_vars.txState = MAC_TX_TIME_OUT;
    }
    mac_tx_stateMachine(MAC_TX_EV_SEND_FAIL, mac_trx_vars.pCurData);
    mac_trx_vars.pCurData = NULL;

    rf_txIrqCheckTimer = NULL;
    IRQ_Restore(r);
    return -1;
}

int softAckTimerCb(void* arg)
{
    if (macPib.rxOnWhenIdle == 0) {
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
    }
    mac_trx_vars.retryBuf = NULL;
    mac_trx_vars.pCurData->data.tx.cnfStatus = MAC_NO_ACK;
    mac_tx_stateMachine(MAC_TX_EV_NO_ACK, mac_trx_vars.pCurData);
    mac_trx_vars.pCurData = NULL;
    softAckCheckTimer = NULL;
    return -1;
}

/**
 * @brief   Finally the transimittion be performed in this function.
 *          It will check whether the packet need a MAC ACK. If so, the
 *          Mac layer retry will be performed. A retry timer will be started.
 *          If not, directly send data out through RF driver.
 *
 * @param   pData - The data to be sent.
 *
 * @return  None
 */
void mac_doTx(u8 *pData)
{
    mac_genericFrame_t*p = (mac_genericFrame_t*)pData;

    /* Check whether the data need MAC acknowledge */
    if (p->flags.bf.fAck) {
        mac_trx_vars.txState = MAC_TX_WAIT_ACK;

        /* Store the buffer for retry */
        if (mac_trx_vars.retryBuf != pData) {
            mac_trx_vars.retryBuf = (u8*)pData;
            mac_trx_vars.retryCnt = macPib.maxFrameRetries;
        }

        /* Store the sequence number */
        mac_trx_vars.txSeq = *(u8*)(((u32)p) + p->data.tx.psduOffset + 2 + TELINK_RF_TX_HDR_LEN);

        /* if it's a new packet, start soft timer to check ack */
        if (mac_trx_vars.retryCnt == macPib.maxFrameRetries) {
            if (softAckCheckTimer) {
                ev_unon_timer(&softAckCheckTimer);
            }
            softAckCheckTimer = ev_on_timer(softAckTimerCb, NULL, 1800*1000);
        }
    }

    /* Start the timer to make sure TX IRQ always come */
    if (rf_txIrqCheckTimer) {
        ev_unon_timer(&rf_txIrqCheckTimer);
    }
    rf_txIrqCheckTimer = ev_on_timer(mac_waitTxIrqCb, NULL, 5000);

    /* Transmit data */
    rf_tx((u8*)(p->data.tx.psduOffset + (u32)p + TELINK_RF_TX_HDR_LEN), p->data.tx.psduLen);
}

/**
 * @brief   Callback function from TX Interrupt.
 *          Entering this function means the data has been send out
 *          successfully. If the packet need MAC ACK, start ack listening
 *          timer here. If not, transmition succeed.
 *
 * @param   arg - The status of TX.
 *
 * @return  None
 */
void mac_txCb(void* arg)
{
    u8 status = (u8)arg;

    /* Firstly, Cancel the TX IRQ waiting timer. */
    if (rf_txIrqCheckTimer) {
        ev_unon_timer(&rf_txIrqCheckTimer);
    }
    rf_txIrqCheckTimer = NULL;

    if (MAC_TX_IDLE == mac_trx_vars.txState) {
        /* Ideally, this path should not enter. And even if it enter,
           it should no impact on other part.*/
        return;
    }

    /* According to the TX status to decide send success or retry */
    if (status == SUCCESS) {
        if (mac_trx_vars.txState == MAC_TX_WAIT_ACK) {
            //start the AckwaitTimer to wait for the Ack
            // u32 timeout = macPib.ackWaitDuration * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
            u32 timeout = macPib.ackWaitDuration * 3 * MAC_SPEC_USECS_PER_SYMBOL;
            if (mac_ackWaitTimer) {
                ev_unon_timer(&mac_ackWaitTimer);
            }
            mac_ackWaitTimer = ev_on_timer(mac_ackWaitingTimerCb, NULL, timeout);
        } 
        else if (mac_trx_vars.txState == MAC_TX_UNDERWAY) {
            mac_tx_stateMachine(MAC_TX_EV_SEND_SUCC, mac_trx_vars.pCurData);
            mac_trx_vars.pCurData = NULL;
        } 
        else {
            /* Ideally, this path should not be hit */
            if ( mac_trx_vars.pCurData ) {
                ev_buf_free((u8*)mac_trx_vars.pCurData);
                mac_trx_vars.pCurData = NULL;
            }
        }
    }
}

/**
 * @brief   This function send the TX confirmation to upper layer.
 *          For Telink MAC, generate the MAC layer primitive and send
 *          back to mac_task.
 *
 * @param   arg - The buffer to hold the TX confirm and TX data.
 *
 * @return  None
 */
void mac_sendTxCnf(u8 *pData)
{
    mac_genericFrame_t *p = (mac_genericFrame_t*)pData;

    /* if not in busy state */
    if (p->data.tx.cnfStatus != MAC_TX_ACTIVE) {
        if (softAckCheckTimer) {
            ev_unon_timer(&softAckCheckTimer);
        }

        if (MAC_RX_ON_WHEN_IDLE() == 0) {
            RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        }
    }

    /* generate confirm */
    u8 cmdID = p->flags.bf.cmdID;
    switch (cmdID ) {
    case MAC_BEACON_REQ_FRAME:
        /* No need to generate confirm */
        if (SUCCESS != ev_buf_free(pData)) {
            while(1);
        }
        break;

    case MAC_BEACON_FRAME:
        /* No need to generate confirm */
        if (SUCCESS != ev_buf_free(pData)) {
            while(1);
        }
        break;

    case MAC_DATA_FRAME:
        p->primitive = MAC_MCPS_DATA_CNF;
        buf_message_post((u8 *)pData, BUF_ITEM_STATE_CSMA2MAC);
        break;

    case MAC_ASSOC_RSP_FRAME:
        p->primitive = MAC_MLME_ASSO_RSP_CNF;
        buf_message_post((u8 *)pData, BUF_ITEM_STATE_CSMA2MAC);
        break;

    case MAC_ASSOC_REQ_FRAME:
        p->primitive = MAC_MLME_ASSOCIATE_CNF;
        buf_message_post((u8 *)pData, BUF_ITEM_STATE_CSMA2MAC);
        break;

    case MAC_DATA_REQ_FRAME:
        p->primitive = MAC_MLME_POLL_CNF;
        buf_message_post((u8 *)pData, BUF_ITEM_STATE_CSMA2MAC);
        break;

    case MAC_DISASSOC_NOTIF_FRAME:
        p->primitive = MAC_MLME_DISASSOCIATE_CNF;
        buf_message_post((u8 *)pData, BUF_ITEM_STATE_CSMA2MAC);
        break;

    default:
        break;
    }

    return;
}

/**
 * @brief   This function handles the MAC Acknowledge packet.
 *          TODO: to check if this function is necessary.
 *
 * @param   pData - The current sending data.
 *
 * @return  None
 */
void mac_ackHandler(u8 *pData)
{
    /* Send TX confirm */
    mac_trx_vars.retryBuf = NULL;
    mac_trx_vars.pCurData->data.tx.cnfStatus = MAC_SUCCESS;
    mac_sendTxCnf((u8*)mac_trx_vars.pCurData);

    mac_trx_vars.pCurData = NULL;
}

/**
 * @brief   Start the MAC ACK waiting timer.
 *          TODO: Need to review if this function is necessary.
 *
 * @param   pData - The current sending data.
 *
 * @return  None
 */
void mac_ackStartTimer(u8 *pData)
{
    // u32 timeout = macPib.ackWaitDuration * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
    u32 timeout = macPib.ackWaitDuration * 3 * MAC_SPEC_USECS_PER_SYMBOL;

    if (mac_ackWaitTimer) {
        ev_unon_timer(&mac_ackWaitTimer);
    }
    mac_ackWaitTimer = ev_on_timer(mac_ackWaitingTimerCb, NULL, timeout);
}
 /* @brief   Trigger the TX BUSY event.
 *          TODO: Need to review if this function is necessary.
 *
 * @param   pData - The current sending data.
 *
 * @return  None
 */
void mac_txBusyHandler(u8 *pData)
{
    mac_genericFrame_t *p = (mac_genericFrame_t*)pData;
    p->data.tx.cnfStatus = MAC_TX_ACTIVE;
    mac_sendTxCnf(pData);
}

/**
 * @brief   This function initialize the RX related setting.
 *          (1) Init RX buffer and backup buffer.
 *          (2) Turn the RF to RX state.
 *
 * @param   None
 *
 * @return  None
 */
void mac_rxInit(void)
{
    u8 *pBackupBuf;
    pBackupBuf = ev_buf_allocate(LARGE_BUFFER);
    if ( NULL == pBackupBuf ) {
        while(1);
    }
    rf_rxBuf = pBackupBuf;
    rf_rxBuf[12] = 1;
    RF_RxBufferSet(rf_rxBuf, RF_RXBUF_MAX_SIZE, 0);

    pBackupBuf = ev_buf_allocate(LARGE_BUFFER);
    if ( NULL == pBackupBuf ) {
        while(1);
    }
    rf_rxBackupBuf = pBackupBuf;
}


/**
* @brief       Check the recieved frame, if the frame is no problem ,Send frame to upper layer
*
* @param[in]   pRxBuf - recieved data buffer.
*
* @param[in]   len -   length of recieved data
*
* @param[in]   timestamp -   The time, in symbols, at which the data were received
*
* @param[in]   lqi -   Link Quality value of recieved data
*
* @return        none
*/
void mac_rxCb(u8 *pRxBuf, u8 len, u32 timestamp, u8 lqi)
{
    mac_genericFrame_t *pMacFrame;
    u16 fcf;
    u16 dstPanID = 0xffff;
    u16 srcPanID = 0xffff;
    u8 hdrLen = MAC_MIN_HDR_LEN;

    u8 *pData = pRxBuf + TELINK_RF_RX_HDR_LEN;

    u8 *p = pData;
    u8 fIntraPan = 0;
    u8 seqNo;
    addr_t srcAddr, dstAddr;

    fcf = ( (*(p+1)) << 8) | (*p);  // (pData[1] <<8) | pData[0];
    p += 2;

    pMacFrame = (mac_genericFrame_t*)pRxBuf;

    /* Add Sequence Number Byte */
    seqNo = *p++;

    /* Check MAC header and do the filter  */
    if (MAC_FRAME_TYPE_BEACON == (*pData & (u8)MAC_FCF_FRAME_TYPE_MASK)) {
        /* No filtering for the BEACON frame */
        srcPanID = ((*(p+1)) << 8) | (*p);
        p += 2;

        if (ADDR_MODE_SHORT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
            hdrLen = 7;
            srcAddr.addrMode = ADDR_MODE_SHORT;
            srcAddr.addr.shortAddr = ((*(p+1)) << 8) | (*p);
        } else if (ADDR_MODE_EXT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
            hdrLen = 13;
            srcAddr.addrMode = ADDR_MODE_EXT;
            memcpy(srcAddr.addr.extAddr, p, EXT_ADDR_LEN);
        }
    } 
    else {
        /* Check PAN id */
        if (!(fcf & MAC_FCF_INTRA_PAN_MASK)) {
            fIntraPan = 0;
        } 
        else { /* only one PAN id is present */
            fIntraPan = 1;
            dstPanID = ( (*(p+1)) << 8) | (*p); //(pData[4] <<8) | pData[3];
            p += 2;
            if (dstPanID != macPib.panId && dstPanID != MAC_PAN_ID_BROADCAST) {
                /* Invalid frame, drop */
                if ( SUCCESS != ev_buf_free(pRxBuf) ) {
                    while(1);
                }

                return;
            }
            hdrLen += 2;
        }

        /* Check Destination address */
        if (ADDR_MODE_NONE != (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
            if (fIntraPan == 0) {
                hdrLen += 2;
                dstPanID = ((*(p+1)) << 8) | (*p);
                p += 2;

                if (dstPanID != MAC_SHORT_ADDR_BROADCAST && dstPanID != macPib.panId) {
                    /* Invalid frame, drop */
                    if ( SUCCESS != ev_buf_free(pRxBuf) ) {
                        while(1);
                    }

                    return;
                }
            }

            if (ADDR_MODE_SHORT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
                hdrLen += 2;
                dstAddr.addrMode = ADDR_MODE_SHORT;
                dstAddr.addr.shortAddr = ( (*(p+1)) << 8) | (*p); //(pData[6] <<8) | pData[5];
                p += 2;
                if (dstAddr.addr.shortAddr != macPib.shortAddress && dstAddr.addr.shortAddr != MAC_SHORT_ADDR_BROADCAST) {
                    /* Invalid frame, drop */
                    if ( SUCCESS != ev_buf_free(pRxBuf) ) {
                        while(1);
                    }

                    return;
                }
            } 
            else if (ADDR_MODE_EXT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
                hdrLen += 8;
                if (0 != memcmp(p, macPib.extendedAddress, EXT_ADDR_LEN)) {
                    /* Invalid frame, drop */
                    if ( SUCCESS != ev_buf_free(pRxBuf) ) {
                        while(1);
                    }

                    return;
                }
                dstAddr.addrMode = ADDR_MODE_EXT;
                memcpy(dstAddr.addr.extAddr, p, EXT_ADDR_LEN);
                p += 8;
            }

        } 
        else {
            if (!mac_getPanCoord()) {
                /* Invalid frame, drop */
                if ( SUCCESS != ev_buf_free(pRxBuf) ) {
                    while(1);
                }

                return;
            }

            dstAddr.addrMode = ADDR_MODE_NONE;
        }

        /* Check Source address */
        if (ADDR_MODE_NONE != (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
            if (fIntraPan == 0) {
                hdrLen += 2;
                srcPanID = ((*(p+1)) << 8) | (*p);
                p += 2;
            }

            if (ADDR_MODE_SHORT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
                srcAddr.addrMode = ADDR_MODE_SHORT;
                srcAddr.addr.shortAddr = ( (*(p+1)) << 8) | (*p);
                p += 2;
                hdrLen += 2;
            }
            else if (ADDR_MODE_EXT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
                srcAddr.addrMode = ADDR_MODE_EXT;
                memcpy(srcAddr.addr.extAddr, p, EXT_ADDR_LEN);
                p += EXT_ADDR_LEN;
                hdrLen += 8;
            }
        }
    }


    /* Simulate a virtual LQI now */
    u8 payloadLen = len - hdrLen - MAC_FCS_FIELD_LEN;
    if (payloadLen > 128) {
        ev_buf_free(pRxBuf);
        return;
    }
    memcpy(pMacFrame->msdu, pData+hdrLen, len - hdrLen - MAC_FCS_FIELD_LEN);
    pMacFrame->data.rx.msduLen = len - hdrLen - MAC_FCS_FIELD_LEN;
    pMacFrame->data.rx.dsn = seqNo;
    pMacFrame->data.rx.lqi = lqi;
    pMacFrame->data.rx.timestamp = timestamp;
    pMacFrame->dstPanId = dstPanID;
    pMacFrame->data.rx.frameType = fcf & MAC_FCF_FRAME_TYPE_MASK;
    addrCpy(&pMacFrame->data.rx.dstAddr, &dstAddr);
    addrCpy(&pMacFrame->srcAddr, &srcAddr);
    if (fIntraPan) {
        pMacFrame->data.rx.srcPanId = dstPanID;
    } 
    else {
        pMacFrame->data.rx.srcPanId = srcPanID;
    }

    /* Send frame to upper layer */
    mac_rx_handler(pMacFrame);
}

void mac_ackCb(u8 fPendingFrame, u8 seq)
{
    if (mac_trx_vars.txState == MAC_TX_WAIT_ACK && mac_trx_vars.txSeq == seq) {
        /* Disable interrupt to make sure ACK timer won't expired during handle ack. */
        u8 r = IRQ_Disable();
        /* ACK received */
        if (mac_ackWaitTimer) {
            ev_unon_timer(&mac_ackWaitTimer);
            mac_ackWaitTimer = NULL;
        }

        if (softAckCheckTimer) {
            ev_unon_timer(&softAckCheckTimer);
        }

        if (fPendingFrame) {
            mac_trx_vars.pCurData->flags.bf.fFramePending = 1;
        }
        else if (macPib.rxOnWhenIdle == 0) {
            RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        }
        mac_tx_stateMachine(MAC_TX_EV_ACK_RECV, mac_trx_vars.pCurData);
        /* It's ok the ack timer happen now for the flag has been set. */
        IRQ_Restore(r);
    }
    return;
}

/**
* @brief       RX Interrupt callback function.
*
* @param[in]   pData - The current recieved data.
*
* @return      none
*/
extern u8 rx_irq_busy;
void mac_rxCb_1(void *pData)
{
    u8 *tmpBuf;

    /* Parse RF received Event and getting the information for
    Upper layer handler */
    rx_buf_t *pRxEvt = (rx_buf_t *)ev_buf_getTail(pData, sizeof(rx_buf_t));
    u8 *pBuf = pRxEvt->rxBuf;
    u8 len = pRxEvt->len;
    u32 timestamp = pRxEvt->timestamp;
    u8 lqi = pRxEvt->rssi;
    u8 *pFrameCtrl = pBuf + TELINK_RF_RX_HDR_LEN;

    tmpBuf = ev_buf_allocate(LARGE_BUFFER);
    if ( NULL == tmpBuf ) {
        /*if there is no buffer, use the current buffer as backup buffer. */
        tmpBuf = pBuf;
    }

    rf_rxBackupBuf = tmpBuf;
    rx_irq_busy = 0;

    if(tmpBuf == pBuf) {
        return;
    }

    if ((*pFrameCtrl) & (u8)MAC_FCF_SEC_ENABLED_MASK) {
        mac_sts_t unsecure_stat = mac_unsecureIncomingFrame(pBuf);
        if (MAC_SUCCESS != unsecure_stat) {
            if (SUCCESS != ev_buf_free(pBuf)) {
                while(1);
            }

            return unsecure_stat;
        }
        len = pRxEvt->len;
    }

    mac_rxCb(pBuf, len, timestamp, lqi);
}

void process_Phy2Mac(void)
{
    u8 *pData = NULL;
    pData = buf_message_poll(BUF_ITEM_STATE_PHY2MAC);
    
    if (pData) {
        mac_rxCb_1((void *)pData);
    }
}
u8 mac_getTrxState(void)
{
    return mac_trx_vars.txState;
}

#if (PHY_TEST_802_15_4)
u8 mac_setTrxState(mac_txState_t State)
{
   mac_trx_vars.txState = State;
}
#endif



