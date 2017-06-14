#include "mac_include.h"

typedef struct {
    mac_state_t curState;
    mac_state_t prevState;
    u8 panCoord;
    mac_cb_t cbFunc;
} mac_ctrl_t;
mac_ctrl_t mac_vars;

static void mac_dropFrame(u8* pData);

static const mac_stateMachine_t mac_stateMachine[] = {
    /*  current state,         primitive,                 next state,              action    */
    { MAC_STATE_IDLE,        MAC_MCPS_DATA_REQ,          MAC_STATE_IDLE,         mac_dropFrame}, // mac_do_data},
    { MAC_STATE_IDLE,        MAC_MCPS_PURGE_REQ,         MAC_STATE_IDLE,         mac_dropFrame}, // mac_do_purge
    { MAC_STATE_IDLE,        MAC_MLME_ASSOCIATE_REQ,     MAC_STATE_ASSOCIATING,  mac_doAssociate},
    { MAC_STATE_IDLE,        MAC_MLME_ASSOCIATE_RES,     MAC_STATE_IDLE,         mac_dropFrame}, // mac_do_asso_resp
    { MAC_STATE_IDLE,        MAC_MLME_DISASSOCIATE_REQ,  MAC_STATE_IDLE,         mac_doScan}, // mac_do_disasso
    { MAC_STATE_IDLE,        MAC_MLME_ORPHAN_RES,        MAC_STATE_IDLE,         mac_dropFrame},
    { MAC_STATE_IDLE,        MAC_MLME_POLL_REQ,          MAC_STATE_POLLING,      mac_dropFrame},
    { MAC_STATE_IDLE,        MAC_MLME_SCAN_REQ,          MAC_STATE_SCAN,         mac_doScan},
    { MAC_STATE_IDLE,        MAC_MLME_START_REQ,         MAC_STATE_STARTING,     mac_doStart},
    { MAC_STATE_IDLE,        MAC_MLME_SYNC_LOSS_IND,     MAC_STATE_SCAN,         mac_dropFrame},
    { MAC_STATE_IDLE,        MAC_MLME_BEACON_NOTIFY_IND, MAC_STATE_IDLE,         mac_dropFrame}, // TODO: add handler
    { MAC_STATE_IDLE,        MAC_MLME_BEACON_REQ_CMD,    MAC_STATE_IDLE,         mac_dropFrame},


    { MAC_STATE_SCAN,        MAC_MLME_SCAN_REQ,          MAC_STATE_SCAN,         mac_doScan},   // MAC_SCAN_FAILED_IN_PROGRESS
    { MAC_STATE_SCAN,        MAC_MLME_BEACON_NOTIFY_IND, MAC_STATE_SCAN,         mac_scanRecvBeacon},   // MAC_API_PENDING
    { MAC_STATE_SCAN,        MAC_MLME_BEACON_REQ_CMD,    MAC_STATE_SCAN,         mac_dropFrame},
    { MAC_STATE_SCAN,        MAC_MLME_BEACON_REQ_CMD,    MAC_STATE_SCAN,         mac_dropFrame},

    { MAC_STATE_ASSOCIATING, MAC_MLME_ASSOCIATE_CNF,     MAC_STATE_ASSOCIATING,  mac_associateCnfHandler},
    { MAC_STATE_ASSOCIATING, MAC_MLME_ASSOCIATE_RES,     MAC_STATE_ASSOCIATING,  mac_associateRspHandler},
    { MAC_STATE_ASSOCIATING, MAC_MLME_POLL_CNF,          MAC_STATE_ASSOCIATING,  mac_associatePollCnfHandler},

    { MAC_STATE_COORD,       MAC_MLME_BEACON_REQ_CMD,    MAC_STATE_COORD,        mac_BeaconReqHanler},
    { MAC_STATE_COORD,       MAC_MLME_ASSOCIATE_IND,     MAC_STATE_COORD,        mac_associateReqHandler},
    { MAC_STATE_COORD,       MAC_MLME_ASSOCIATE_RES,     MAC_STATE_COORD,        mac_doAssociateRspSend},
    { MAC_STATE_COORD,       MAC_MLME_ASSO_RSP_CNF,      MAC_STATE_COORD,        mac_assoRspCnfHandler},
    { MAC_STATE_COORD,       MAC_MLME_SCAN_REQ,          MAC_STATE_IDLE,         mac_doScan},   // MAC_SCAN_FOR_FREQ_AGILITY
    { MAC_STATE_COORD,       MAC_MCPS_DATA_REQ,          MAC_STATE_COORD,        mac_doSendData},
    { MAC_STATE_COORD,       MAC_MCPS_DATA_CNF,          MAC_STATE_COORD,        mac_dataCnfHandler},
    { MAC_STATE_COORD,       MAC_MCPS_DATA_IND,          MAC_STATE_COORD,        mac_dataIndHandler},
    { MAC_STATE_COORD,       MAC_MLME_DATA_REQ_CMD,      MAC_STATE_COORD,        mac_dataReqHandler},
    { MAC_STATE_COORD,       MAC_MLME_DISASSOCIATE_REQ,  MAC_STATE_COORD,        mac_doDisassociate},
    { MAC_STATE_COORD,       MAC_MLME_DISASSOCIATE_CNF,  MAC_STATE_COORD,        mac_disassociateCnfHandler},
    { MAC_STATE_COORD,       MAC_MLME_DISASSOCIATE_IND,  MAC_STATE_COORD,        mac_disassociateIndHandler},
    { MAC_STATE_COORD,       MAC_MCPS_PURGE_REQ,         MAC_STATE_COORD,        mac_doPurge},
    { MAC_STATE_COORD,       MAC_MLME_START_REQ,         MAC_STATE_COORD,        mac_dropFrame},
    { MAC_STATE_COORD,       MAC_MLME_RX_ENABLE_REQ,     MAC_STATE_COORD,        mac_rxOnEnableReqHanler},


    { MAC_STATE_DEV,         MAC_MCPS_DATA_REQ,          MAC_STATE_DEV,          mac_doSendData},
    { MAC_STATE_DEV,         MAC_MCPS_DATA_CNF,          MAC_STATE_DEV,          mac_dataCnfHandler},
    { MAC_STATE_DEV,         MAC_MCPS_DATA_IND,          MAC_STATE_DEV,          mac_dataIndHandler},
    { MAC_STATE_DEV,         MAC_MLME_POLL_REQ,          MAC_STATE_POLLING,      mac_doPoll},
    { MAC_STATE_DEV,         MAC_MLME_POLL_CNF,          MAC_STATE_POLLING,      mac_dropFrame},
    { MAC_STATE_DEV,         MAC_MLME_DISASSOCIATE_REQ,  MAC_STATE_DEV,          mac_doDisassociate},
    { MAC_STATE_DEV,         MAC_MLME_DISASSOCIATE_CNF,  MAC_STATE_DEV,          mac_disassociateCnfHandler},
    { MAC_STATE_DEV,         MAC_MLME_DISASSOCIATE_IND,  MAC_STATE_DEV,          mac_disassociateIndHandler},
    { MAC_STATE_DEV,         MAC_MLME_RX_ENABLE_REQ,     MAC_STATE_DEV,          mac_rxOnEnableReqHanler},


    { MAC_STATE_POLLING,     MAC_MLME_POLL_CNF,          MAC_STATE_POLLING,      mac_pollCnfHandler},
    { MAC_STATE_POLLING,     MAC_MCPS_DATA_IND,          MAC_STATE_POLLING,      mac_dataIndHandler},
};



void mac_reset(u8 fSetDfltPib)
{
    mac_vars.curState = MAC_STATE_IDLE;
    mac_vars.prevState = MAC_STATE_IDLE;
    mac_vars.panCoord = 0;

    //reset PIB 
    mac_pibReset();
}

void mac_mlmeResetReq(u8 fSetDfltPib)
{
    unsigned char *pData;
    pData = ev_buf_allocate(0);
    if (NULL == pData) {
        return;
    }
    mac_resetCnf_t *pResetCnf = (mac_resetCnf_t *)pData;
    pResetCnf->status = MAC_SUCCESS;
    pResetCnf->primitive = MAC_MLME_RESET_CNF;

    mac_reset(fSetDfltPib);
    mac_trxInit();
    mac_scanInit();
    mac_dataInit();
    mac_sendConfirm((u8*)pResetCnf);
}

void mac_runStateMachine(mac_generalReq_t *pData, u8 type)
{
    int i = 0;
    int size = sizeof(mac_stateMachine)/sizeof(mac_stateMachine_t);

    /* Search an appropriate event handler */
    for (i = 0; i < size; i++) {
        if (mac_stateMachine[i].curState == mac_vars.curState &&
            mac_stateMachine[i].primitive == pData->primitive) {
                /* set mac state to next state */
                if (mac_vars.curState != mac_stateMachine[i].nxtState) {
                    /* The state save only happened when two state is not same */
                    mac_vars.prevState = mac_vars.curState;
                    mac_vars.curState = mac_stateMachine[i].nxtState;
                }

                /* call event handler */
                mac_stateMachine[i].evHandlerFunc((u8 *)pData);
                return;
        }
    }

    if (NWK_2_MAC_TYPE == type) {
        /* if not found, send invalid request as confirm */
        pData->primitive++;  /* convert the request id to confirm id */
        ((mac_generalCnf_t *)pData)->status = MAC_INVALID_REQ;

        if (MAC_MCPS_DATA_CNF == pData->primitive) {
            /* copy the msduHandle field to the confirm */
            ((mac_dataCnf_t*)pData)->msduHandle = ((mac_dataReq_t*)pData)->msduHandle;
        }

        /* send the confirm to nwk layer */
        buf_message_post(pData, BUF_ITEM_STATE_MAC2NWK);
    } 
    else { /* Drop the invalid frame */
        mac_dropFrame((u8 *)pData);
    }
}


void mac_init()
{
    /* Reset Mac */
    mac_reset(0);

    /* Initialize Tx Rx module */
    mac_trxInit();
    /* Initialize scan module */
    mac_scanInit();
    /* Initialize data module */
    mac_dataInit();
}


mac_state_t mac_getCurState(void)
{
    return mac_vars.curState;
}


void mac_sendConfirm(u8* pData)
{
    buf_message_post(pData, BUF_ITEM_STATE_MAC2NWK);
}

void mac_sendIndication(u8* pData)
{
	buf_message_post(pData, BUF_ITEM_STATE_MAC2NWK);
}

void mac_restoreState(void)
{
    mac_vars.curState = mac_vars.prevState;
}

void mac_setState(mac_state_t status)
{
    mac_vars.prevState = mac_vars.curState;
    mac_vars.curState = status;
}

mac_state_t mac_getState(void)
{
    return mac_vars.curState;
}

void mac_dropFrame(u8* pData)
{
    if (SUCCESS != ev_buf_free(pData)) {
        while(1);
    }
}

void mac_setPanCoord(u8 fCoord)
{
    u8 value;
    mac_vars.panCoord = fCoord;
    value = 1;
	rf_802154_set(RF_ID_MAC_PAN_COORD, &value, 1);
}

u8 mac_getPanCoord(void)
{
    return mac_vars.panCoord;
}


void mac_genCommStatusInd(u8* pData, u8 status, addr_t* pSrcAddr, addr_t*pDstAddr)
{
    mac_commStatusInd_t *pInd = (mac_commStatusInd_t *)pData;

    switch(*pData) {
    case MAC_MLME_ASSO_RSP_CNF:
        pInd->primitive = MAC_MLME_COMM_STATUS_IND;
        pInd->status = status;
        pInd->panId = macPib.panId;
        addrCpy(&pInd->srcAddr, pSrcAddr);
        addrCpy(&pInd->dstAddr, pDstAddr);
        break;
    }

    mac_sendIndication((u8*)pInd);
}


void mac_getCoordAddr(addr_t *pAddr)
{
    if (macPib.coordShortAddress == MAC_ADDR_USE_EXT) {
        pAddr->addrMode = ADDR_MODE_EXT;
        memcpy(pAddr, &macPib.coordExtendedAddress, EXT_ADDR_LEN);
    }
    else {
        pAddr->addrMode = ADDR_MODE_SHORT;
        pAddr->addr.shortAddr = macPib.coordShortAddress;
    }
}

u8 mac_getSelfAddrMode(void)
{
    return (macPib.shortAddress >= MAC_ADDR_USE_EXT) ? ADDR_MODE_EXT : ADDR_MODE_SHORT;
}

void process_Nwk2Mac(void)
{
    u8* pData = NULL;
    pData = buf_message_poll(BUF_ITEM_STATE_NWK2MAC);
    
    if (pData) {
        mac_runStateMachine((mac_generalReq_t*)pData, NWK_2_MAC_TYPE);
    }
}

void process_CSMA2Mac(void){
    u8* pData = NULL;
    pData = buf_message_poll(BUF_ITEM_STATE_CSMA2MAC);
    
    if (pData) {
        mac_runStateMachine((mac_generalReq_t*)pData, PHY_2_MAC_TYPE);
    }
}
