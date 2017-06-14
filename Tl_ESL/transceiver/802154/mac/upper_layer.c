#include "upper_layer.h"
#include "mac_include.h"

static void DefaultCb_DataInd(unsigned char *pData);
static void DefaultCb_AssociateInd(unsigned char *pData);
static void DefaultCb_CommStatusInd(unsigned char *pData);
static void DefaultCb_BeaconNotifyInd(unsigned char *pData);
static void DefaultCb_ScanCnf(unsigned char *pData);
static void DefaultCb_StartCnf(unsigned char *pData);
static void DefaultCb_ResetCnf(unsigned char *pData);
static void DefaultCb_McpsDataCnf(unsigned char *pData);
static void DefaultCb_McpsPurgeCnf(unsigned char *pData);
static void DefaultCb_RxEnableCnf(unsigned char *pData);
static void DefaultCb_SetCnf(unsigned char *pData);
static void DefaultCb_GetCnf(unsigned char *pData);
static void DefaultCb_AssociateCnf(unsigned char *pData);
static void DefaultCb_DisassociateCnf(unsigned char *pData);
static void DefaultCb_PollCnf(unsigned char *pData);

UpperLayerCb_Type UpperLayerCallbacks[CALLBACK_MAX_NUM] = {
    DefaultCb_DataInd,
    DefaultCb_AssociateInd,
    DefaultCb_CommStatusInd,
    DefaultCb_BeaconNotifyInd,
    DefaultCb_ScanCnf,
    DefaultCb_StartCnf,
    DefaultCb_ResetCnf,
    DefaultCb_McpsDataCnf,
    DefaultCb_McpsPurgeCnf,
    DefaultCb_RxEnableCnf,
    DefaultCb_SetCnf,
    DefaultCb_GetCnf,
    DefaultCb_AssociateCnf,
    DefaultCb_DisassociateCnf,
    DefaultCb_PollCnf,
};

static void DefaultCb_DataInd(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_AssociateInd(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_CommStatusInd(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_BeaconNotifyInd(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_ScanCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_StartCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_ResetCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_McpsDataCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_McpsPurgeCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_RxEnableCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_SetCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_GetCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_AssociateCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_DisassociateCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

static void DefaultCb_PollCnf(unsigned char *pData)
{
    if (pData) {
        ev_buf_free(pData);
    }
}

void UpperLayerCallbackSet(unsigned char Index, UpperLayerCb_Type Callback)
{
    if ((Index < CALLBACK_MAX_NUM) && Callback) {
        UpperLayerCallbacks[Index] = Callback;
    }
    
}

static void UpperLayerDoProcess(void *arg)
{
    unsigned char *pMsg = (unsigned char *)arg;

    if (pMsg != NULL) {
        switch (*pMsg) {
        case MAC_MCPS_DATA_IND:
            UpperLayerCallbacks[CALLBACK_DATA_INDICATION](pMsg);
            break;
        case MAC_MLME_ASSOCIATE_IND:
            UpperLayerCallbacks[CALLBACK_ASSOCIATE_INDICATION](pMsg);
            break;
        case MAC_MLME_COMM_STATUS_IND:
            UpperLayerCallbacks[CALLBACK_COMM_STATUS_INDICATION](pMsg);
            break;
        case MAC_MLME_BEACON_NOTIFY_IND:
            UpperLayerCallbacks[CALLBACK_BEACON_NOTIFY_INDICATION](pMsg);
            break;
        case MAC_MLME_SCAN_CNF:
            UpperLayerCallbacks[CALLBACK_SCAN_CONFIRM](pMsg);
            break;
        case MAC_MLME_START_CNF:
            UpperLayerCallbacks[CALLBACK_START_CONFIRM](pMsg);
            break;
        case MAC_MLME_RESET_CNF:
            UpperLayerCallbacks[CALLBACK_MAC_RESET_CONFIRM](pMsg);
            break;
        case MAC_MCPS_DATA_CNF: 
            UpperLayerCallbacks[CALLBACK_MCPS_DATA_CONFIRM](pMsg);
            break;
        case MAC_MCPS_PURGE_CNF: 
            UpperLayerCallbacks[CALLBACK_MCPS_PURGE_CONFIRM](pMsg);
            break;
        case MAC_MLME_RX_ENABLE_CNF:
            UpperLayerCallbacks[CALLBACK_RX_ENABLE_CONFIRM](pMsg);
            break;
        case MAC_MLME_SET_CNF:
            UpperLayerCallbacks[CALLBACK_SET_CONFIRM](pMsg);
            break;
        case MAC_MLME_GET_CNF:
            UpperLayerCallbacks[CALLBACK_GET_CONFIRM](pMsg);
            break;
        case MAC_MLME_ASSOCIATE_CNF:
            UpperLayerCallbacks[CALLBACK_ASSOCIATE_CONFIRM](pMsg);
            break;
        case MAC_MLME_DISASSOCIATE_CNF:
            UpperLayerCallbacks[CALLBACK_DISASSOCIATE_CONFIRM](pMsg);
            break;
        case MAC_MLME_POLL_CNF:
            UpperLayerCallbacks[CALLBACK_POLL_CONFIRM](pMsg);
            break;
        default:                  
            break;          
        }        
    }
}

void UpperLayerProcess(void)
{
    unsigned char *pMsg;
    
    pMsg = buf_message_poll(BUF_ITEM_STATE_MAC2NWK);
    if (pMsg != NULL) {
        UpperLayerDoProcess((void *)pMsg);
    }
}
