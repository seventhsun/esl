#include "mac_associate.h"

#define MAC_NO_ASSOCIATE_ADDR    0xffff

typedef struct {
    mac_associateCnf_t *pAssoCnf;
    ev_time_event_t *waitRspTimer;

    mac_disassociateCnf_t *pDisassoCnf;
    u16 disassoPanID;
    addr_t disassoAddr;
} mac_assoCtrl_t;
mac_assoCtrl_t mac_assoCtrl_v;
mac_assoCtrl_t *mac_asso_v = NULL;

void mac_mlmeAssociateReq(mac_associateReq_t* pAssoReq)
{
    buf_message_post((u8 *)pAssoReq,BUF_ITEM_STATE_NWK2MAC);
}

void mac_mlmeAssociateRsp(mac_associateRsp_t* pAssoRsp)
{
    buf_message_post((u8 *)pAssoRsp,BUF_ITEM_STATE_NWK2MAC);
}

void mac_mlmeDissociateReq(mac_disassociateReq_t* pDissoReq)
{
    buf_message_post((u8 *)pDissoReq,BUF_ITEM_STATE_NWK2MAC);
}

/////////////////////////////////////////////
//associate related functions for end device
/////////////////////////////////////////////
static mac_sts_t mac_sendAssociateReq(mac_genericFrame_t* pData)
{
    mac_associateReq_t* pReq = (mac_associateReq_t*)pData;
    addr_t dstAddr;
    u8 capInfo;

    memcpy((u8*)&dstAddr, (u8*)&pReq->coordAddress, sizeof(addr_t));
    capInfo = pReq->capabilityInfo;

    return mac_buildCommonReq((u8*)pData, pData->flags.bf.cmdID, ADDR_MODE_EXT | MAC_SRC_PAN_ID_BROADCAST, &dstAddr, pReq->coordPanId, MAC_TXOPTION_ACK, &capInfo);
}

static void mac_sendAssociateConfirm(u8 status, u16 assoAddr)
{
    mac_asso_v->pAssoCnf->primitive = MAC_MLME_ASSOCIATE_CNF;
    mac_asso_v->pAssoCnf->status = status;
    mac_asso_v->pAssoCnf->assocShortAddress = assoAddr;

    if (status == MAC_SUCCESS) {
        mac_setState(MAC_STATE_DEV);
    } 
    else {
        mac_restoreState();
    }

    mac_sendConfirm((u8*)mac_asso_v->pAssoCnf);

    mac_asso_v->pAssoCnf = NULL;
    
    mac_asso_v = NULL;
}

//1. As the associating device, it firstly sends out the associate request
void mac_doAssociate(u8* pData)
{
    mac_sts_t status = MAC_SUCCESS;
    mac_associateReq_t* pAssoReq = (mac_associateReq_t*)pData;
    mac_genericFrame_t* pMacFrame;

    if (mac_asso_v == NULL) {
        mac_asso_v = &mac_assoCtrl_v;
    }
    /* Store the confirm buffer, reuse the request buffer */
    mac_asso_v->pAssoCnf = (mac_associateCnf_t*)pData;

    /* Verify parameters */
    if (pAssoReq->logicalChannel < MAC_CHAN_11 ||
        pAssoReq->logicalChannel > MAC_CHAN_26 ||
        pAssoReq->coordAddress.addrMode < ADDR_MODE_SHORT || 
        pAssoReq->coordAddress.addrMode > ADDR_MODE_EXT) {
        status = MAC_INVALID_PARAMETER;
    }

    if (MAC_SUCCESS == status) {
        /* set channel in radio */
        mac_mlmeSetReq(MAC_LOGICAL_CHANNEL, 0, &pAssoReq->logicalChannel);

        /* set PAN ID, coordinator address in PIB */
        mac_mlmeSetReq(MAC_PAN_ID, 0, &pAssoReq->coordPanId);
        if (pAssoReq->coordAddress.addrMode == ADDR_MODE_SHORT) {
            macPib.coordShortAddress = pAssoReq->coordAddress.addr.shortAddr;
        }
        else {
            macPib.coordShortAddress = MAC_ADDR_USE_EXT;
            memcpy(macPib.coordExtendedAddress.addr.extAddr, pAssoReq->coordAddress.addr.extAddr, 8);
        }

        /* build and send associate request */
        pMacFrame = (mac_genericFrame_t *)pAssoReq;
        pMacFrame->flags.bf.cmdID = MAC_ASSOC_REQ_FRAME;
        pMacFrame->flags.bf.fAck = 1;
        status = mac_sendAssociateReq(pMacFrame);
    }

    if (MAC_SUCCESS != status) {
        /* Send confirm back */
        mac_sendAssociateConfirm(status, MAC_NO_ASSOCIATE_ADDR);
    }
}

//2. After sending out the associate request successfully, it starts the macResponseWaitTimer immediately.
//   otherwise it will respond to the next higher lay with the Associate Confirm primitive indicating 
//   failure immediately.
static int mac_assoWaitPrepareTimerCb(void* arg);

static void mac_startAssoResponseWaitTimer(void)
{
    /* Need to change from symbol to us */
    u32 timeout = (u32)macPib.responseWaitTime * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
    mac_asso_v->waitRspTimer = ev_on_timer(mac_assoWaitPrepareTimerCb, NULL, timeout);
}

void mac_associateCnfHandler(u8* pData)
{
    mac_genericFrame_t* p = (mac_genericFrame_t*)pData;

    /* ignore the success confirm, it still need to wait associate response */
    if (p->data.tx.cnfStatus != MAC_SUCCESS) {
        mac_sendAssociateConfirm(p->data.tx.cnfStatus, MAC_NO_ASSOCIATE_ADDR);
    } 
    else {
        /* Start a wait associate response timer */
        mac_startAssoResponseWaitTimer();
    }  
}

//3. once the macResponseWaitTimer expires, the associating device
//   then sends out the data request command frame for acquiring 
//   potential associate response 
static int mac_assoWaitPrepareTimerCb(void* arg)
{
    /* Poll the associate response */
    mac_pollReq_t* pData = (mac_pollReq_t*)ev_buf_allocate(MIDDLE_BUFFER);
    if (NULL == pData) {
        while(1);
    }
    memset(pData, 0, BUFFER_SIZE);

    pData->primitive = MAC_MLME_POLL_REQ;
    mac_getCoordAddr(&pData->coordAddress);
    mac_buildCommonReq((u8*)pData, MAC_DATA_REQ_FRAME, ADDR_MODE_EXT, &pData->coordAddress, macPib.panId, MAC_TXOPTION_ACK, NULL);
    mac_asso_v->waitRspTimer = NULL;
    return -1;
}

//4. After sending out the data request command frame successfully, the associating device then starts a Timer to wait 
//   for the associate response. Otherwise it will respond to the next higher lay with the Associate Confirm primitive 
//   indicating failure.
static int mac_assoWaitRspTimerCb(void* arg)
{
    /* The association response not back, send confirm of Failure */
    mac_sendAssociateConfirm(MAC_NO_DATA, MAC_NO_ASSOCIATE_ADDR);
    mac_asso_v->waitRspTimer = NULL;
    return -1;
}

void mac_associatePollCnfHandler(u8* pData)
{
    u32 timeout;
    mac_genericFrame_t* p = (mac_genericFrame_t*)pData;

    if (p->data.tx.cnfStatus == MAC_SUCCESS && p->flags.bf.fFramePending) {
        /* If there is data pending, Keep receiver ON, start a timer
        to wait data */
        if (SUCCESS != ev_buf_free(pData)) {
            while(1);
        }

        timeout = (u32) macPib.responseWaitTime * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
        mac_asso_v->waitRspTimer = ev_on_timer(mac_assoWaitRspTimerCb, NULL, timeout);
    } 
    else { 
        /* Free the buffer, send associate failure confirm */
        if (SUCCESS != ev_buf_free(pData)) {
            while(1);
        }

        mac_sendAssociateConfirm(MAC_NO_DATA, MAC_NO_ASSOCIATE_ADDR);
    }
}

//5. If the device receives the expected associate response from coordinator, it will send the associate confirm 
//   primitive containing assigned short address to the next higher layer.
void mac_associateRspHandler(u8* pData)
{
    u16 shortAddr;
    u8 status;
    mac_genericFrame_t *pMacFrame = (mac_genericFrame_t*)pData;

    /* turn off tranceiver */

    /* stop timers */
    ev_unon_timer(&mac_asso_v->waitRspTimer);

    /* parse frame parameters */
    shortAddr = (u16)(*(pMacFrame->msdu+1)) | (u16)(*(pMacFrame->msdu+2)<<8);
    status = *(pMacFrame->msdu + 3);

    if (status == MAC_ASSOC_SUCCESS) {
        /* set short addr and coord ext addr in PIB */
        mac_mlmeSetReq(MAC_SHORT_ADDRESS, 0, &shortAddr);
        mac_mlmeSetReq(MAC_COORD_EXTENDED_ADDRESS, 0, pMacFrame->srcAddr.addr.extAddr);

        /* TODO: check security */
    }

    /* free the response buffer because the association confirm buffer has
       already be stored. */
    if (SUCCESS != ev_buf_free(pData)) {
        while(1);
    }
    /* call callback for associate */
    mac_sendAssociateConfirm(status, shortAddr);
}

/////////////////////////////////////////////
//associate related functions for coordinator
/////////////////////////////////////////////

//1. If the MAC of coordinator receives an associate request from an associating device,
//   it will report this by passing a MLME-ASSOCIATE.indication primitive to the next
//   higher layer. 
void mac_associateReqHandler(u8* pData)
{
    mac_associateInd_t *pAssoInd = (mac_associateInd_t*)pData;
    mac_genericFrame_t *pMacFrame = (mac_genericFrame_t*)pData;

    /* Build to associate indication format */
    if (macPib.associationPermit) {
        memcpy(&pAssoInd->deviceAddress, pMacFrame->srcAddr.addr.extAddr, EXT_ADDR_LEN);
        pAssoInd->capabilityInfo = *(pMacFrame->msdu+1);
    }

    pAssoInd->sec.securityLevel = 0;

    /* Send the indication to upper layer */
    mac_sendIndication((u8*)pAssoInd);
}

//2. If the MAC of coordinator receives a MLME-ASSOCIATE.response including successful status,
//   it will send out an associate response command frame in indirect transmission method.
static mac_sts_t mac_sendAssociateRsp(mac_genericFrame_t *pData)
{
    addr_t dstAddr;
    mac_associateRsp_t* pRsp = (mac_associateRsp_t*)pData;

    u8 payload[MAC_ASSOC_RSP_PAYLOAD];

    payload[0] = MAC_ASSOC_RSP_FRAME;
    payload[1] = pRsp->assocShortAddr & 0xff; //low byte of the assigned short address
    payload[2] = (pRsp->assocShortAddr >> 8) & 0xff; //high byte of the assigned short address
    payload[3] = pRsp->status;

    dstAddr.addrMode = ADDR_MODE_EXT;
    memcpy(dstAddr.addr.extAddr, pRsp->deviceAddress, EXT_ADDR_LEN);
    pData->flags.bf.cmdID = MAC_ASSOC_RSP_FRAME;
    return mac_buildCommonReq((u8*)pData, pData->flags.bf.cmdID, ADDR_MODE_EXT, &dstAddr, macPib.panId, MAC_TXOPTION_ACK|MAC_TXOPTION_INDIRECT, payload);
}

void mac_doAssociateRspSend(u8* pData)
{
    mac_sts_t status;
    mac_genericFrame_t *p;
    status = mac_sendAssociateRsp((mac_genericFrame_t*)pData);

    if (MAC_SUCCESS != status) {
        p = (mac_genericFrame_t*)pData;
        p->primitive = MAC_MLME_ASSO_RSP_CNF;
        p->data.tx.cnfStatus = status;
        mac_assoRspCnfHandler((u8*)p);
    }
}

//3. After sending out the associate response command frame successfully, the MAC of coordinator will
//   report the result by passing a MLME-COMM-STATUS.indication primitive to the next higher layer.
void mac_assoRspCnfHandler(u8* pData)
{
    mac_genericFrame_t *p = (mac_genericFrame_t*)pData;
    addr_t srcAddr;
    addr_t dstAddr;

    srcAddr.addrMode = ADDR_MODE_EXT;
    memcpy(srcAddr.addr.extAddr, macPib.extendedAddress, EXT_ADDR_LEN);

    dstAddr.addrMode = ADDR_MODE_EXT;
    memcpy(dstAddr.addr.extAddr, (u8*)(((u32)p) + p->data.tx.psduOffset + 5 + TELINK_RF_TX_HDR_LEN), EXT_ADDR_LEN);

    if (mac_asso_v != NULL) {
        mac_asso_v = NULL;
    }
    /* Generate comm-status indication */
    mac_genCommStatusInd(pData, p->data.tx.cnfStatus, &srcAddr, &dstAddr);
}

void mac_sendDisassociateConfirm(u8 status)
{
    mac_asso_v->pDisassoCnf->primitive = MAC_MLME_DISASSOCIATE_CNF;
    mac_asso_v->pDisassoCnf->status = status;
    mac_asso_v->pDisassoCnf->panId = mac_asso_v->disassoPanID;
    memcpy((u8*)&mac_asso_v->pDisassoCnf->devAddr, (u8*)&mac_asso_v->disassoAddr, sizeof(addr_t));

    if (status == MAC_SUCCESS) {
        mac_setState(MAC_STATE_DEV);
    } else {
        mac_restoreState();
    }

    mac_sendConfirm((u8*)mac_asso_v->pAssoCnf);

    mac_asso_v->pAssoCnf = NULL;
	
	mac_asso_v = NULL;
}

static mac_sts_t mac_sendDisassociateNotify(mac_genericFrame_t* pData)
{
    mac_disassociateReq_t *pReq = (mac_disassociateReq_t*)pData;
    u8 txoption = pReq->txIndirect ? MAC_TXOPTION_ACK | MAC_TXOPTION_INDIRECT : MAC_TXOPTION_ACK;
    u8 payload[2];

    payload[0] = MAC_DISASSOC_NOTIF_FRAME;
    payload[1] = pReq->disassociateReason;
    return mac_buildCommonReq((u8*)pData, MAC_DISASSOC_NOTIF_FRAME, ADDR_MODE_EXT, &pReq->devAddr, pReq->devPanId, txoption, payload);
}

void mac_doDisassociate(u8* pData)
{
    u8 fSendToCoord = 0;

    mac_disassociateReq_t *pReq = (mac_disassociateReq_t*)pData;
    mac_sts_t status = MAC_SUCCESS;

    if ( mac_asso_v == NULL ) {
        mac_asso_v = &mac_assoCtrl_v;
    }
    /* Store the confirm buffer, reuse the request buffer */
    mac_asso_v->pDisassoCnf = (mac_disassociateCnf_t*)pData;
    mac_asso_v->disassoPanID = pReq->devPanId;
    memcpy((u8*)&mac_asso_v->disassoAddr, (u8*)&pReq->devAddr, sizeof(addr_t));

    /* Verify the PAN ID */
    if (pReq->devPanId != macPib.panId) {
        status = MAC_INVALID_PARAMETER;
    }

    if (pReq->devAddr.addrMode == ADDR_MODE_SHORT) {
        fSendToCoord = (pReq->devAddr.addr.shortAddr == macPib.coordShortAddress) ? 1 : 0;
    } else {
        fSendToCoord = memcmp(pReq->devAddr.addr.extAddr, macPib.coordExtendedAddress.addr.extAddr, EXT_ADDR_LEN) ? 0 : 1;
    }

    if (fSendToCoord) {
        pReq->txIndirect = 0;
    } else {
        if (pReq->txIndirect) {
            status = MAC_INVALID_PARAMETER;
        }
    }

    if (MAC_SUCCESS == status) {
        /* send disassociate notification */
        status = mac_sendDisassociateNotify((mac_genericFrame_t*)pReq);
    }

    if (MAC_SUCCESS != status) {
        /* Send confirm of failure */
        mac_sendDisassociateConfirm(status);
    }
    
    
}



void mac_disassociateCnfHandler(u8* pData)
{

}



void mac_disassociateIndHandler(u8* pData)
{

}
