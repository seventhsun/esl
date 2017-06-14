#include "mac_data.h"

#if (INDIR_DATA)
typedef struct {
    /* used for poll primitive */
    mac_pollCnf_t *pPollCnf;
    /* used for indirect data request */
    ev_queue_t indirQ;
} mac_dataCtrl_t;
mac_dataCtrl_t mac_data_vars;

mac_indirItem_t  mac_indirTable[MAC_MAX_INDIR_DATA_NUM];
ev_time_event_t *mac_dataPollWaitTimer;
#endif

#if (INDIR_DATA)
static void mac_sendPollConfirm(u8 status);
#endif

static int mac_pollWaitTimerCb(void* arg);

void mac_dataReset(void)
{
#if (INDIR_DATA)
    u8 i, value;
    mac_indirItem_t* pItem;
    while (mac_data_vars.indirQ.curNum > 0) {
        pItem = (mac_indirItem_t*)ev_queue_rawPop(&mac_data_vars.indirQ);
        if (pItem->pData != NULL) {
            ev_buf_free(pItem->pData);
        }
    }

    /* Reset Indirect Table */
    for (i = 0; i < MAC_MAX_INDIR_DATA_NUM; i++) {
        mac_indirTable[i].msduHandle = 0xff;
        mac_indirTable[i].next = NULL;
        mac_indirTable[i].pData = NULL;
        memset((u8*)&mac_indirTable[i].devAddr, MAC_RESET_VALUE, sizeof(addr_t));
    }
    /* Reset the control variables */
    mac_data_vars.pPollCnf = NULL;

    value = 0;
    rf_802154_set(RF_ID_MAC_FRAME_PENDING, &value, 1);
#endif
}

void mac_dataInit(void)
{
#if (INDIR_DATA)
    ev_queue_init(&mac_data_vars.indirQ, NULL);
    memset(&mac_data_vars, 0, sizeof(mac_dataCtrl_t));
#endif
    mac_dataReset();
}

void mac_mcpsDataReq(mac_dataReq_t* pDataReq)
{
    buf_message_post((u8 *)pDataReq,BUF_ITEM_STATE_NWK2MAC);
}

void mac_mlmePollReq(mac_pollReq_t* pPollReq)
{
    buf_message_post((u8 *)pPollReq,BUF_ITEM_STATE_NWK2MAC);
}

void mac_mcpsPurgeReq(mac_purgeReq_t* pPurgeReq)
{
    buf_message_post((u8 *)pPurgeReq,BUF_ITEM_STATE_NWK2MAC);
}

////////////////////////////////////////////////////
//Data confirm
////////////////////////////////////////////////////
static void mac_sendDataConfirm(u8 *pData, u8 status, u8 msduHandle)
{
    mac_dataCnf_t* pCnf = (mac_dataCnf_t*)pData;


    pCnf->primitive = MAC_MCPS_DATA_CNF;
    pCnf->status = status;
    pCnf->msduHandle = msduHandle;

    mac_sendConfirm((u8*)pCnf);
}

void mac_dataCnfHandler(u8 *pData)
{
    mac_genericFrame_t* pCnf = (mac_genericFrame_t*)pData;
    mac_sendDataConfirm((u8*)pCnf, pCnf->data.tx.cnfStatus, pCnf->data.tx.msduHandle);
}

//////////////////////////////////////////////////
//Direct Data Request
/////////////////////////////////////////////////
void mac_doSendData(u8 *pData)
{
    mac_sts_t status = MAC_SUCCESS;
    mac_dataReq_t* pReq = (mac_dataReq_t*)pData;
    mac_genericFrame_t *pMacFrame = (mac_genericFrame_t *)pData;
    u8 msduHandle = pReq->msduHandle;

    /* Check parameters */
    if (pReq->msduLen > (macPib.securityEnabled ? (MAC_MAX_FRAME_SIZE - MAC_MAX_AUX_SEC_HDR_SIZE) : MAC_MAX_FRAME_SIZE)) {
        status = MAC_INVALID_PARAMETER;
    }

    if (pReq->srcAddrMode == ADDR_MODE_NONE &&
        pReq->dstAddr.addrMode == ADDR_MODE_NONE) {
        status = MAC_INVALID_ADDRESS;
    }

    if (MAC_SUCCESS == status) {
        /* Build Data frame and send */
        status = mac_sendDataFrame(pMacFrame);
    }

    if (MAC_SUCCESS != status) {
        /* Send confirm back to upper layer */
        mac_sendDataConfirm(pData, status, msduHandle);
    }
}

////////////////////////////////////////////////////////
//Data Indication
////////////////////////////////////////////////////////
void mac_dataIndHandler(u8 *pData)
{
    mac_dataInd_t* pInd = (mac_dataInd_t*)pData;

    /* Send data indication to upper layer first */
    mac_sendIndication((u8*)pInd);

    /* Check whether it is indirect data, if so, send polling confirm */
    if (MAC_STATE_POLLING == mac_getState()) {
#if (INDIR_DATA)
        /* Cancel polling timer */
        if (mac_dataPollWaitTimer && !is_timer_expired(mac_dataPollWaitTimer)) {
            ev_unon_timer(&mac_dataPollWaitTimer);
        }

        /* Send confirm back */
        mac_sendPollConfirm(MAC_SUCCESS);
#endif
    }
}

/////////////////////////////////////////////////////////
//Poll confirm
/////////////////////////////////////////////////////////
#if (INDIR_DATA)
static void mac_sendPollConfirm(u8 status)
{
    mac_data_vars.pPollCnf->primitive = MAC_MLME_POLL_CNF;
    mac_data_vars.pPollCnf->status = status;

    mac_restoreState();

    mac_sendConfirm((u8*)mac_data_vars.pPollCnf);

    mac_data_vars.pPollCnf = NULL;
}

void mac_pollCnfHandler(u8 *pData)
{
#if (INDIR_DATA)
    u32 timeout;
    mac_genericFrame_t *p = (mac_genericFrame_t*)pData;

    if (p->data.tx.cnfStatus == MAC_SUCCESS) {
        /* If there is data pending, Keep receiver ON, start a timer
        to wait data */
        if (p->flags.bf.fFramePending) {
            timeout = macPib.responseWaitTime * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
            mac_dataPollWaitTimer = ev_on_timer(mac_pollWaitTimerCb, NULL, timeout);
        } 
        else { /* No pending data */
            mac_sendPollConfirm(MAC_NO_DATA);
        }

    } 
    else { /* Other PHY error */
        mac_sendPollConfirm(p->data.tx.cnfStatus);
    }
#endif
}

/////////////////////////////////////////////////////////
//Poll request
/////////////////////////////////////////////////////////
static int mac_pollWaitTimerCb(void* arg)
{
    /* Return MAC_NO_DATA as poll confirm */
    mac_sendPollConfirm(MAC_NO_DATA);
    mac_dataPollWaitTimer = NULL;
    return -1;
}
#endif

static mac_sts_t mac_sendDataRequest(mac_genericFrame_t* pData)
{
    mac_pollReq_t* pReq = (mac_pollReq_t*)pData;
    return mac_buildCommonReq((u8*)pData, MAC_DATA_REQ_FRAME, mac_getSelfAddrMode(), &pReq->coordAddress, pReq->coordPanId, MAC_TXOPTION_ACK, NULL);
}

void mac_doPoll(u8 *pData)
{
#if (INDIR_DATA)
    mac_sts_t status;

    /* Store the buffer for confirm use */
    mac_data_vars.pPollCnf = (mac_pollCnf_t *)pData;
    status = mac_sendDataRequest((mac_genericFrame_t*)pData);
    if (MAC_SUCCESS != status) {
        mac_sendPollConfirm(status);
    }
#endif
}

/////////////////////////////////////////////////////////////
//Indirect Data Request
/////////////////////////////////////////////////////////////
static int mac_indirTimerExpire(void* arg)
{
#if (INDIR_DATA)
    mac_indirItem_t* pItem;
    mac_genericFrame_t* p;
    u8 value;

    /* Get the expired item */
    pItem = (mac_indirItem_t*)ev_queue_rawPop(&mac_data_vars.indirQ);

    /* Clear the item */
    p = (mac_genericFrame_t*)pItem->pData;
    pItem->next = NULL;
    pItem->pData = NULL;
    memset((u8*)&pItem->devAddr, MAC_RESET_VALUE, sizeof(addr_t));
    if (mac_data_vars.indirQ.curNum == 0) {
        value = 0;
        rf_802154_set(RF_ID_MAC_FRAME_PENDING, &value, 1);
    }

    if (p->primitive == MAC_MLME_ASSOCIATE_RES) {
        p->primitive = MAC_MLME_ASSO_RSP_CNF;
        p->data.tx.cnfStatus = MAC_TRANSACTION_EXPIRED;
        mac_assoRspCnfHandler((u8*)p);
    } 
    else {
        /* Send confirm of Failure */
        mac_sendDataConfirm((u8*)p, MAC_TRANSACTION_EXPIRED, pItem->msduHandle);
    }
    pItem->timer = NULL;
#endif
    return -1;
}

mac_sts_t mac_indirectTx(u8 *pData, addr_t *pDstAddr)
{
#if (INDIR_DATA)
    mac_indirItem_t* pItem = NULL;
    u8 i;
    u32 timeout;
    u8 value;

    /* Check whether these is capacity for more indirect request */
    if (mac_data_vars.indirQ.curNum >= MAC_MAX_INDIR_DATA_NUM) {
        return MAC_TRANSACTION_OVERFLOW;
    }

    /* Search an empty entry */
    for (i = 0; i < MAC_MAX_INDIR_DATA_NUM; i++) {
        if (mac_indirTable[i].devAddr.addrMode == MAC_RESET_VALUE) {
            pItem = &mac_indirTable[i];
            break;
        }
    }
    if (NULL == pItem) {
        while(1);
    } 
    else {
        /* Found, Store the indirect state */
        timeout = macPib.transactionPersistenceTime * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
        pItem->msduHandle = ((mac_genericFrame_t*)pData)->data.tx.msduHandle;
        pItem->txOptions = ((mac_dataReq_t*)pData)->txOptions;
        addrCpy(&pItem->devAddr, pDstAddr);
        pItem->pData = (u8*)pData;

        /* add to indirect queue */
        ev_queue_rawPush(&mac_data_vars.indirQ, (queue_item_t*)pItem);
        value = 1;
        rf_802154_set(RF_ID_MAC_FRAME_PENDING, &value, 1);

        /* Start a persistent timer */
        pItem->timer = ev_on_timer(mac_indirTimerExpire, NULL, timeout);
    }
    return MAC_SUCCESS;
#endif
}

///////////////////////////////////////////////////////////
//Data request command frame handler
///////////////////////////////////////////////////////////
void mac_dataReqHandler(u8 *pData)
{
#if (INDIR_DATA)
    mac_genericFrame_t* pMacFrame = (mac_genericFrame_t*)pData;
    mac_indirItem_t* pItem;
    u8 *p;
    u8 fFound = 0, value;
    mac_sts_t status = MAC_SUCCESS;

    pItem = (mac_indirItem_t*)mac_data_vars.indirQ.head;
    while (pItem) {
        if (addrCmp(&pMacFrame->srcAddr, &pItem->devAddr)) {
            /* Found */
            fFound = 1;
            break;
        } 
        else { /* Not found */
            pItem = pItem->next;
        }
    }

    if (fFound) {
        /* Cancel the persistent timer */
        if (pItem->timer){
            ev_unon_timer(&pItem->timer);
        }

        /* Remove the item from Indirect Queue */
        if (SUCCESS != ev_queue_rawDelete(&mac_data_vars.indirQ, (queue_item_t*)pItem)) {
            while(1);
        }

        /* Clear the item */
        p = pItem->pData;
        pItem->next = NULL;
        pItem->pData = NULL;
        memset((u8*)&pItem->devAddr, MAC_RESET_VALUE, sizeof(addr_t));

        if (mac_data_vars.indirQ.curNum == 0) {
            value = 0;
            rf_802154_set(RF_ID_MAC_FRAME_PENDING, &value, 1);
        }

        /* Send it out through TRX module */
        status = mac_tx((mac_genericFrame_t*)p);
    }

    /* Free the buffer of Data Request Frame */
    if (SUCCESS != ev_buf_free(pData)) {
        while(1);
    }
    if (MAC_SUCCESS != status) {
        /* Tx Failure, send confirm to upper layer */
        mac_sendDataConfirm(p, status, ((mac_genericFrame_t*)p)->data.tx.msduHandle);
    }
#endif
    return;
}


void mac_doPurge(u8 *pData)
{
#if (INDIR_DATA)
    mac_sts_t status = MAC_SUCCESS;
    mac_purgeReq_t* pReq = (mac_purgeReq_t*)pData;
    mac_purgeCnf_t* pCnf = (mac_purgeCnf_t*)pData;
    mac_indirItem_t* pItem;
    u8 *p;
    u8 fFound = 0;
    u8 value;

    pItem = (mac_indirItem_t*)mac_data_vars.indirQ.head;
    while (pItem) {
        if (pItem->msduHandle == pReq->msduHandle) {
            /* Found, remove from Queue */
            ev_queue_rawDelete(&mac_data_vars.indirQ, (queue_item_t*)pItem);
            if (mac_data_vars.indirQ.curNum == 0) {
                value = 0;
                rf_802154_set(RF_ID_MAC_FRAME_PENDING, &value, 1);
            }
            fFound = 1;
            break;
        } 
        else { /* Not found */
            pItem = pItem->next;
        }
    }

    if (fFound) {
        /* Cancel the timer */
        ev_unon_timer(&pItem->timer);

        /* Clear the item */
        p = pItem->pData;
        pItem->next = NULL;
        pItem->pData = NULL;
        memset((u8*)&pItem->devAddr, MAC_RESET_VALUE, sizeof(addr_t));

        /* Send confirm of Failure */
        mac_sendDataConfirm(p, MAC_TRANSACTION_EXPIRED, pReq->msduHandle);
    } 
    else {
        status = MAC_INVALID_HANDLE;
    }

    pCnf->primitive = MAC_MCPS_PURGE_CNF;
    pCnf->status = status;

    mac_sendConfirm((u8*)pCnf);
#endif
}
