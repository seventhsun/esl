#include "mac_start.h"

u8 mac_nwkStarted;

#if MAC_SUPPORTED_START

void mac_doStart(u8 *pData)
{
    mac_sts_t status = MAC_SUCCESS;
    mac_startCnf_t* pCnf;
    mac_startReq_t* pReq = (mac_startReq_t*)pData;

    /* Verify Parameters */
    if (pReq->logicalChannel < MAC_CHAN_11  || 
        pReq->logicalChannel >= MAC_CHAN_26 ||
        pReq->beaconOrder != MAC_BO_NON_BEACON ||
        pReq->coordRealignSec.securityLevel > 0 ||
        pReq->panId == MAC_PAN_ID_BROADCAST) {
        status = MAC_INVALID_PARAMETER;
    }

    if (macPib.shortAddress == MAC_SHORT_ADDR_NONE) {
        status = MAC_NO_SHORT_ADDRESS;
    }

    if (status == MAC_SUCCESS) {    
        /* Do Start Coordinator or Coordinator realignment */
        if (pReq->coordRealignment) {
            /* TODO: add realignment handler */
            mac_mlmeSetReq(MAC_PAN_ID, 0, &pReq->panId);
            macPib.beaconOrder = pReq->beaconOrder;
            macPib.superframeOrder = pReq->superframeOrder;
            macPib.battLifeExt = pReq->batteryLifeExt;
            mac_setPanCoord(pReq->panCoordinator);
            mac_nwkStarted = TRUE;
        }
        else {
            /* set the beacon order, superframe order, and battery life extension in PIB */
            macPib.beaconOrder = pReq->beaconOrder;
            macPib.superframeOrder = pReq->superframeOrder;
            macPib.battLifeExt = pReq->batteryLifeExt;

            /* set pan coordinator bit in radio; if pan coordinator set pan id and logical channel */
            mac_setPanCoord(pReq->panCoordinator);
            if (pReq->panCoordinator) {
                mac_mlmeSetReq(MAC_PAN_ID, 0, &pReq->panId);
                mac_mlmeSetReq(MAC_LOGICAL_CHANNEL, 0, &pReq->logicalChannel);
            }

            mac_nwkStarted = TRUE;
        }
    }

    /* Send start confirm  */
    pCnf = (mac_startCnf_t*)pReq;
    pCnf->primitive = MAC_MLME_START_CNF;
    pCnf->status = status;

    if (status == MAC_SUCCESS) {
        mac_setState(MAC_STATE_COORD);
    } 
    else {
        mac_restoreState();
    }

    mac_sendConfirm((u8 *)pCnf);
}

void mac_mlmeStartReq(mac_startReq_t *pStartReq)
{
    buf_message_post((u8 *)pStartReq,BUF_ITEM_STATE_NWK2MAC);
}

void mac_BeaconReqHanler(u8 *pData)
{
	/* Create Beacon and send*/
    mac_sendBeacon(pData);
}

#endif
