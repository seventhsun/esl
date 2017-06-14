#include "mac_scan.h"

typedef struct {
    mac_scanCnf_t    *pScanCnf;
    ev_time_event_t  *scanTimer;    
    u32              scanChannels;
    u8               scanDuration;
    u8               currentChannel;
    u8               scanType;
    u8               origChannel;
} mac_scanCtrl_t;
mac_scanCtrl_t mac_scanCtrl_v;
mac_scanCtrl_t *mac_scan_v = &mac_scanCtrl_v;

static void mac_startScanTimer(void);
static void mac_buildBeaconNotify(u8 *pData, mac_panDesc_t *pPanDesc);

void mac_scanReset(void)
{

}

void mac_scanInit(void)
{
    mac_scanReset();
}

void mac_mlmeScanReq(mac_scanReq_t *pScanReq)
{
    buf_message_post((u8 *)pScanReq,BUF_ITEM_STATE_NWK2MAC);
}

void mac_genScanConfirm(mac_sts_t status)
{
    mac_scan_v->pScanCnf->primitive = MAC_MLME_SCAN_CNF;
    mac_scan_v->pScanCnf->status = status;
    mac_scan_v->pScanCnf->scanType = mac_scan_v->scanType;
    mac_scan_v->pScanCnf->unscannedChannels = mac_scan_v->scanChannels;
    mac_scan_v->pScanCnf->resultListSize = 0;
}

static void mac_scanComplete(void *arg)
{
    u8 status = (u8)arg;
    if (status != MAC_INVALID_PARAMETER) {
        if (mac_scan_v->scanTimer) {
            ev_unon_timer(&mac_scan_v->scanTimer);
        }

        /*restore the rf channel after scan if neither the scan type is orphan scan nor the orphan scan fails */
        if (!((mac_scan_v->pScanCnf->scanType == MAC_SCAN_ORPHAN) &&
            (mac_scan_v->pScanCnf->status == MAC_SUCCESS))) {
            mac_mlmeSetReq(MAC_LOGICAL_CHANNEL, 0, &mac_scan_v->origChannel);
        }
    }

    /* Restore the MAC state and send scan confirm back to higher layer */
    mac_scan_v->pScanCnf->status = status;

    if (status == MAC_SUCCESS) {
        mac_setState(MAC_STATE_IDLE);
    } 
    else {
        mac_restoreState();
    }
    
    mac_sendConfirm((u8 *)mac_scan_v->pScanCnf);
}


void mac_scanRecvBeacon(u8 *pData)
{
    u8 *p = ((mac_genericFrame_t *)pData)->msdu;
    mac_genericFrame_t* pMacFrame = (mac_genericFrame_t*)pData;
    if (mac_scan_v->pScanCnf->resultListSize >= MAX_PAN_DESC_SUPPORTED) {
        if (SUCCESS != ev_buf_free(pData)) {
            while(1);
        }
        return;
    }
    mac_panDesc_t* pPanDesc = (mac_panDesc_t*)&mac_scan_v->pScanCnf->resultList.panDescList[mac_scan_v->pScanCnf->resultListSize];

    if (mac_scan_v->scanType == MAC_SCAN_PASSIVE || 
        mac_scan_v->scanType == MAC_SCAN_ACTIVE) {
        /* Set the confirm to SUCC */
        mac_scan_v->pScanCnf->status = MAC_SUCCESS;

        /* Build Pan Descriptor */
        pPanDesc->timestamp = ((mac_genericFrame_t *)pData)->data.rx.timestamp;;
        pPanDesc->linkQuality = ((mac_genericFrame_t *)pData)->data.rx.lqi;
        pPanDesc->channelPage = MAC_CHANNEL_PAGE_0;
        addrCpy(&pPanDesc->coordAddr, &pMacFrame->srcAddr);
        pPanDesc->coordPanId = ((mac_genericFrame_t*)pData)->data.rx.srcPanId;
        pPanDesc->securityFailure = FALSE;
        pPanDesc->logicalChannel = mac_scan_v->currentChannel;
        pPanDesc->superframeSpec = p[0] | (p[1] << 8);
        pPanDesc->gtsPermit = p[2];

        /* Security, currently not use */
        memset((u8*)&pPanDesc->sec, 0, sizeof(mac_sec_t));
        mac_scan_v->pScanCnf->resultListSize++;
        
        /* Build BeaconNotifyIndication */
        mac_buildBeaconNotify(pData, pPanDesc);
        if (!macPib.autoRequest) {
            mac_sendIndication(pData);
        } 
        else {
            if (SUCCESS != ev_buf_free(pData) ) {
                while(1);
            }      
        }
        /* If result list is full, scan is complete */
        if (mac_scan_v->pScanCnf->resultListSize == MAX_PAN_DESC_SUPPORTED) {
            mac_scanComplete((void*)mac_scan_v->pScanCnf->status);
        }
    }
}

static mac_sts_t mac_sendBeaconRequest(void)
{
    addr_t dstAddr;

    dstAddr.addrMode = ADDR_MODE_SHORT;
    dstAddr.addr.shortAddr = MAC_SHORT_ADDR_BROADCAST;

    return mac_buildCommonReq(NULL, MAC_BEACON_REQ_FRAME, ADDR_MODE_NONE | MAC_SRC_PAN_ID_BROADCAST, &dstAddr, MAC_PAN_ID_BROADCAST, 0, NULL);
}

static void mac_EDScanStart(void)
{
    rf_edDetect_ptr = RF_EdDetect;
    RF_TrxStateSet(RF_MODE_RX, macPib.logicalChannel); //enable rx mode on current channel
}

static unsigned char mac_EDScanStop(void)
{
    unsigned char ret = 0;

    rf_edDetect_ptr = NULL;
    ret = RF_StopEd();
    
    return ret;
}

static int mac_scanNextChannel(void *arg)
{
    mac_sts_t status = MAC_SUCCESS;

    /* Get ED scan result in last channel */
    if ((mac_scan_v->pScanCnf->scanType == MAC_SCAN_ED) &&
        (mac_scan_v->currentChannel != MAC_SCAN_CHAN_INIT)) {
        /* we used maxResults element to temporarily store value */
        mac_scan_v->pScanCnf->resultList.energyDetectList[mac_scan_v->currentChannel - MAC_CHAN_11] = mac_EDScanStop();

        /* increment number of results here */
        mac_scan_v->pScanCnf->resultListSize++;
    }

    do { /* get next channel */
        mac_scan_v->scanChannels >>= 1;
        mac_scan_v->currentChannel++;
    } while(((mac_scan_v->scanChannels & 1) == 0) && (mac_scan_v->currentChannel < MAC_SCAN_CHAN_END));

    /* if all channels have been scanned */
    if (mac_scan_v->currentChannel == MAC_SCAN_CHAN_END) {
        mac_scanComplete((void*)mac_scan_v->pScanCnf->status);
        mac_scan_v->scanTimer = NULL;
        return -1;  // remove the scan timer
    }

    /* scan the new channel */
    mac_mlmeSetReq(MAC_LOGICAL_CHANNEL, 0, &mac_scan_v->currentChannel);

    if (mac_scan_v->scanType == MAC_SCAN_ED || 
        mac_scan_v->scanType == MAC_SCAN_PASSIVE) {
        mac_scan_v->pScanCnf->unscannedChannels &= ~((u32) 1 << mac_scan_v->currentChannel);
        mac_EDScanStart();
        return 0;
    } 
    else { 
        /* Active scan */
        if (mac_scan_v->scanType == MAC_SCAN_ACTIVE) {
            status = mac_sendBeaconRequest();
        }
        /* Orphan scan */
        else { 
            //ToDo
        }

        if (status != MAC_SUCCESS) {
            /* Scan complete, confirm is failure */
            mac_scanComplete((void*)status);
            mac_scan_v->scanTimer = NULL;
            return -1;
        } 
        else {
            mac_scan_v->pScanCnf->unscannedChannels &= ~((u32) 1 << mac_scan_v->currentChannel);
            return 0;
        }
    }
}

static void mac_startScanTimer(void)
{
    u32 timeout;

    /* mark current channel as scanned */
    mac_scan_v->pScanCnf->unscannedChannels &= ~((u32) 1 << mac_scan_v->currentChannel);
    /* set timeout; different for orphan and other scans */
    if (mac_scan_v->pScanCnf->scanType == MAC_SCAN_ORPHAN) {
        timeout = (u32) macPib.responseWaitTime * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
    } 
    else {
        timeout = (((u32) 1 << mac_scan_v->scanDuration) + 1) * MAC_A_BASE_SUPERFRAME_DURATION * MAC_SPEC_USECS_PER_SYMBOL;
    }

    /* start timer; different for energy detect and other scans */
    if (mac_scan_v->pScanCnf->scanType == MAC_SCAN_ED) {
        mac_EDScanStart();
    }

    /* The scanTimer will be started automatically when it expires*/
    mac_scan_v->scanTimer = ev_on_timer(mac_scanNextChannel, NULL, timeout);
}

void mac_doScan(u8 *pData)
{
    mac_scanReq_t *pReq = (mac_scanReq_t*)pData;
    u32 temp;
    u8 temp2;

    /* Save the scan parameters */
    mac_scan_v->scanType        = pReq->scanType;
    temp = pReq->scanChannels;
    temp2 = pReq->channelPage;
    mac_scan_v->scanChannels    = pReq->scanChannels >> MAC_SCAN_CHAN_INIT;
    mac_scan_v->scanDuration    = pReq->scanDuration;
    mac_scan_v->currentChannel  = MAC_SCAN_CHAN_INIT;
    mac_scan_v->origChannel     = macPib.logicalChannel;

    /* Reuse the buffer for scan confirm */
    mac_scan_v->pScanCnf = (mac_scanCnf_t *)pReq;
    mac_scan_v->pScanCnf->primitive = MAC_MLME_SCAN_CNF;
    mac_scan_v->pScanCnf->unscannedChannels = temp;
    mac_scan_v->pScanCnf->scanType = mac_scan_v->scanType;
    mac_scan_v->pScanCnf->resultListSize = 0;
    mac_scan_v->pScanCnf->channelPage = temp2;
    memset(&(mac_scan_v->pScanCnf->resultList), 0, sizeof(mac_panDesc_t) * MAX_PAN_DESC_SUPPORTED);

    /* Check parameter */
    if (mac_scan_v->scanType > MAC_SCAN_ORPHAN ||
        mac_scan_v->scanDuration > MAC_SCAN_DURATION_MAX) {   
        u8 status = MAC_INVALID_PARAMETER;
        mac_scanComplete((void *)status);
        return;
    }

    if (mac_scan_v->scanType != MAC_SCAN_ED) {
        mac_scan_v->pScanCnf->status = MAC_NO_BEACON;
    }

    /* Start scan */
    mac_startScanTimer();
}

static void mac_buildBeaconNotify(u8*pData , mac_panDesc_t* pPanDesc)
{
    mac_beaconNotifyInd_t *pInd = (mac_beaconNotifyInd_t *)pData;
    u8 *p = ((mac_genericFrame_t *)pData)->msdu;
    u8 msduLen = ((mac_genericFrame_t *)pData)->data.rx.msduLen;

    pInd->primitive = MAC_MLME_BEACON_NOTIFY_IND;
    pInd->bsn = ((mac_genericFrame_t *)pData)->data.rx.dsn;
    memcpy((u8 *)&pInd->panDesc, (u8 *)pPanDesc, sizeof(mac_panDesc_t));
    
    /* Currently the gts permit field must be 0 */
    pInd->pendAddrSpec = p[3];

    /* Currently the pending address spec field also should be 0 */
    pInd->pAddrList = NULL;
    pInd->psduLength = msduLen - 4;
    memcpy(pInd->psdu, p+4, pInd->psduLength);
}



