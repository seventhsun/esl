#include "mac_frame.h"

/* Table of MAC command payload length by command identifier */
static const u8 macCmdLen[] =
{
    0,                                  /* No MAC commond on index 0 */
    MAC_ASSOC_REQ_PAYLOAD,              /* MAC_ASSOC_REQ_FRAME */
    MAC_ASSOC_RSP_PAYLOAD,              /* MAC_ASSOC_RSP_FRAME */
    MAC_DISASSOC_NOTIF_PAYLOAD,         /* MAC_DISASSOC_NOTIF_FRAME */
    MAC_DATA_REQ_PAYLOAD,               /* MAC_DATA_REQ_FRAME */
    MAC_PAN_CONFLICT_PAYLOAD,           /* MAC_PAN_CONFLICT_FRAME */
    MAC_ORPHAN_NOTIF_PAYLOAD,           /* MAC_ORPHAN_NOTIF_FRAME */
    MAC_BEACON_REQ_PAYLOAD,             /* MAC_BEACON_REQ_FRAME */
    MAC_COORD_REALIGN_PAYLOAD,          /* MAC_COORD_REALIGN_FRAME */
    MAC_GTS_REQ_PAYLOAD                 /* MAC_GTS_REQ_FRAME */
};


static u8 mac_buildHeader(u8 *pEoh, u8 frameType, u8 srcAddrMode, addr_t *pDstAddr, u16 dstPanID, u8 txOption)
{
    u16 srcPanID;
    u16 fcf = 0;
    u8 *p = pEoh;

    /* initialize source PAN ID */
    if (srcAddrMode & MAC_SRC_PAN_ID_BROADCAST) {
        srcPanID = MAC_PAN_ID_BROADCAST;
    } 
    else {
        srcPanID = macPib.panId;
    }
    srcAddrMode &= ~MAC_SRC_PAN_ID_BROADCAST;

    /* header is built in reverse order */

    /* set some frame control field bits */
    if (txOption & MAC_TXOPTION_ACK){
        fcf |= MAC_FCF_ACK_REQUEST_MASK;
    }

    fcf |= frameType;

    /* source address */
    fcf |= (u16) srcAddrMode << MAC_FCF_SRC_ADDR_MODE_POS;
    if (srcAddrMode == ADDR_MODE_SHORT) {
        *p-- = (macPib.shortAddress >> 8) & 0xff;
        *p-- = macPib.shortAddress & 0xff;
    } 
    else if (srcAddrMode == ADDR_MODE_EXT) {
        p -= EXT_ADDR_LEN - 1;
        memcpy(p, macPib.extendedAddress, EXT_ADDR_LEN);
        p--;
    }

    /* source PAN ID */
    if ((srcAddrMode != ADDR_MODE_NONE) && 
        (pDstAddr->addrMode != ADDR_MODE_NONE) &&
        (srcPanID == dstPanID) ) {
        fcf |= MAC_FCF_INTRA_PAN_MASK;
    } 
    else if (srcAddrMode != ADDR_MODE_NONE) {
        *p-- = (srcPanID >> 8) & 0xff;
        *p-- = srcPanID & 0xff;
    }

    /* destination address */
    fcf |= (u16) (pDstAddr->addrMode << MAC_FCF_DST_ADDR_MODE_POS);
    if (pDstAddr->addrMode == ADDR_MODE_SHORT) {
        *p-- = (pDstAddr->addr.shortAddr >> 8) & 0xff;
        *p-- = pDstAddr->addr.shortAddr & 0xff;
    } 
    else if (pDstAddr->addrMode == ADDR_MODE_EXT) {
        p -= EXT_ADDR_LEN - 1;
        memcpy(p, pDstAddr->addr.extAddr, EXT_ADDR_LEN);
        p--;
    }

    /* destination PAN ID */
    if (pDstAddr->addrMode != ADDR_MODE_NONE) {
        *p-- = (dstPanID >> 8) & 0xff;
        *p-- = dstPanID & 0xff;
    }

    /* sequence number */
    *p-- = (frameType == MAC_FRAME_TYPE_BEACON) ? macPib.bsn++ : macPib.dsn++;

    /* frame control field */
    *p-- = (fcf >> 8) & 0xff;
    *p   = fcf & 0xff;

    return pEoh - p + 1;
}

static void mac_buildCmdPayload(u8 cmdID, u8 *pPayload, u8 *pValue)
{
    switch (cmdID) {
    case MAC_BEACON_REQ_FRAME:
        *pPayload = cmdID;
        break;

    case MAC_ASSOC_REQ_FRAME:
        *pPayload++ = cmdID;
        *pPayload = *pValue;
        break;

    case MAC_ASSOC_RSP_FRAME:
        memcpy(pPayload, pValue, MAC_ASSOC_RSP_PAYLOAD);
        break;

    case MAC_DATA_REQ_FRAME:
        *pPayload = cmdID;
        break;

    case MAC_DISASSOC_NOTIF_FRAME:
        memcpy(pPayload, pValue, MAC_DISASSOC_NOTIF_PAYLOAD);
        break;

    default:
        break;
    }

    return;
}

mac_sts_t mac_buildCommonReq(u8 *pBuff, u8 cmdID, u8 srcAddrMode, addr_t *pDstAddr, u16 dstPanID, u8 txOption, u8 *payload)
{
    mac_genericFrame_t *pMacFrame;
    u8 hdrLen;
    addr_t dstAddr;
    mac_sec_t secInfo;

    memset(&secInfo, 0, sizeof(mac_sec_t));
    if (cmdID == MAC_DATA_REQ_FRAME) {
        my_memcpy(&secInfo, &(((mac_pollReq_t*)pBuff)->sec), sizeof(mac_sec_t));
    }
    else if (cmdID == MAC_DISASSOC_NOTIF_FRAME) {
        my_memcpy(&secInfo, &(((mac_disassociateReq_t*)pBuff)->sec), sizeof(mac_sec_t));
    }
    else if (cmdID == MAC_ASSOC_REQ_FRAME) {
        my_memcpy(&secInfo, &(((mac_associateReq_t*)pBuff)->sec), sizeof(mac_sec_t));
    }
    else if (cmdID == MAC_ASSOC_RSP_FRAME) {
        my_memcpy(&secInfo, &(((mac_associateRsp_t*)pBuff)->sec), sizeof(mac_sec_t));
    }
    else {
    }

    if (pBuff) { /* Reuse the specified buffer */
        pMacFrame = (mac_genericFrame_t *)pBuff;
    } 
    else { /* Allocate a new buffer */
        pMacFrame = (mac_genericFrame_t*)ev_buf_allocate(LARGE_BUFFER);
        if (!pMacFrame) {
            return MAC_NO_RESOURCES;
        }
        pMacFrame->flags.byteVal = 0;
    }
    
    pMacFrame->flags.bf.cmdID = cmdID;
    addrCpy(&dstAddr, pDstAddr);

    /* Build MAC header */
    hdrLen = mac_buildHeader(pMacFrame->msdu - 1, MAC_FRAME_TYPE_COMMAND, srcAddrMode, pDstAddr, dstPanID, txOption);
    pMacFrame->data.tx.psduOffset = (((u32)pMacFrame->msdu) - hdrLen - TELINK_RF_TX_HDR_LEN) - (u32)pMacFrame;
    pMacFrame->data.tx.psduLen = macCmdLen[cmdID] + hdrLen;
    /* Build payload according to the command */
    mac_buildCmdPayload(cmdID, pMacFrame->msdu, payload);

    if (txOption & MAC_TXOPTION_ACK) {
        pMacFrame->flags.bf.fAck = 1;
    }

    if (secInfo.securityLevel > SECURITY_LEVEL_NONE) {
        mac_sts_t secure_stat = mac_secureOutgoingFrame((u8*)pMacFrame, &secInfo);
        if (MAC_SUCCESS != secure_stat) {
            return secure_stat;
        }
    }

    if (txOption & MAC_TXOPTION_INDIRECT) {
        return mac_indirectTx((u8*)pMacFrame, &dstAddr);
    }
    return mac_tx(pMacFrame);
}

mac_sts_t mac_sendBeacon(u8 *pData)
{
    u16 superFrame = 0;
    mac_genericFrame_t* pBeacon;
    u8 *payload;
    u8 hdrLen;
    addr_t dstAddr;

    /* Build superframe */
    superFrame |= macPib.beaconOrder;
    superFrame |= macPib.superframeOrder << MAC_SFS_SUPERFRAME_ORDER_POS;
    superFrame |= (MAC_A_NUM_SUPERFRAME_SLOTS - 1) << MAC_SFS_FINAL_CAP_SLOT_POS;
    superFrame |= macPib.battLifeExt << MAC_SFS_BATT_LIFE_EXT_POS;
    superFrame |= mac_getPanCoord() << MAC_SFS_PAN_COORD_POS;
    superFrame |= macPib.associationPermit << MAC_SFS_ASSOC_PERMIT_POS;

    pBeacon = (mac_genericFrame_t*)pData;
    pBeacon->flags.byteVal = 0;
    pBeacon->flags.bf.cmdID = MAC_BEACON_FRAME;
     
    /* Build Beacon header */
    dstAddr.addrMode = ADDR_MODE_NONE;
    hdrLen = mac_buildHeader(pBeacon->msdu - 1, MAC_FRAME_TYPE_BEACON, mac_getSelfAddrMode(), &dstAddr, 0, 0);

    /* Build payload */
    payload = pBeacon->msdu;
    *payload++ = superFrame & 0xff;
    *payload++ = (superFrame >> 8) & 0xff;
    *payload++ = MAC_NO_GTS_FIELD;
    *payload++ = MAC_NO_PENDING_ADDR_FIELD;

    memcpy(payload, macPib.pBeaconPayload, macPib.beaconPayloadLength);
    pBeacon->data.tx.psduLen = macPib.beaconPayloadLength + 4 + hdrLen;
	pBeacon->data.tx.psduOffset = pBeacon->msdu - hdrLen - TELINK_RF_TX_HDR_LEN - pData;

    return mac_tx(pBeacon);
 }

mac_sts_t mac_sendDataFrame(mac_genericFrame_t* pData)
{
    mac_dataReq_t *pReq = (mac_dataReq_t*)pData;
    u8 srcAddrMode = pReq->srcAddrMode;
    u8 txOptions = pReq->txOptions;
    addr_t dstAddr;
    u8 hdrLen;
    u8 msduHandle = pReq->msduHandle;
    addrCpy(&dstAddr, &pReq->dstAddr);
    mac_sec_t secInfo;
    my_memcpy(&secInfo, &pReq->sec, sizeof(mac_sec_t));

    /* Fill some internal use field */
    if (txOptions & MAC_TXOPTION_ACK) {
        pData->flags.bf.fAck = 1;
    }
    pData->flags.bf.cmdID = MAC_DATA_FRAME;

    /* Build MAC header */
    hdrLen = mac_buildHeader(pData->msdu - 1, MAC_FRAME_TYPE_DATA, srcAddrMode, &pReq->dstAddr, pReq->dstPanId, txOptions);
    pData->data.tx.psduLen = pReq->msduLen + hdrLen;
	pData->data.tx.psduOffset = ((u32)(pData->msdu)) - hdrLen - TELINK_RF_TX_HDR_LEN - ((u32)pData);
    pData->data.tx.msduHandle = msduHandle;

    if (secInfo.securityLevel > SECURITY_LEVEL_NONE) {
    	mac_sts_t secure_stat = mac_secureOutgoingFrame((u8*)pData, &secInfo);
    	if (MAC_SUCCESS != secure_stat) {
    	   return secure_stat;
    	}
    }

    if (txOptions & MAC_TXOPTION_INDIRECT) {
        return mac_indirectTx((u8*)pData, &dstAddr);
    }

    return mac_tx(pData);
}

//added by zhangjian for 802.15.4 security 2015/10/20
/**
* @brief   get the offset of the Frame Control field from the start of
*          the mac_genericFrame_t type mac frame intended to transmit.
*
* @param   pData - the frame to be transmitted
*
* @return  the offset of the Frame Control field
*/
u8 mac_tx_getFrameControlPos(mac_genericFrame_t* pData)
{
    return  (u8)(pData->data.tx.psduOffset + TELINK_RF_TX_HDR_LEN);
}

/**
* @brief   get the offset of the Mac Payload field from the start of
*          the mac_genericFrame_t type mac frame intended to transmit.
*
* @param   pData - the frame to be transmitted
*
* @return  the offset of the Mac Payload field
*/
u8 mac_tx_getMacPayloadPos(mac_genericFrame_t* pData)
{
    return (u8)(((u32)(pData->msdu)) - ((u32)pData));
}

/**
* @brief   get the length of the Mac Payload field.
*
* @param   pData - the frame to be transmitted
*
* @return  the length of the Mac Payload field
*/
u8 mac_tx_getMacPayloadLen(mac_genericFrame_t* pData)
{
    u8 frameCtrlPos = mac_tx_getFrameControlPos(pData);
    u8 macPayloadPos = mac_tx_getMacPayloadPos(pData);
    u8 macHeaderLen = macPayloadPos - frameCtrlPos;
    return (pData->data.tx.psduLen-macHeaderLen);
}

/**
* @brief   get the offset of the actual payload field from the start of
*          the mac_genericFrame_t type mac frame intended to transmit.
*
* @param   pData - the frame to be transmitted
*
* @return  the offset of the actual payload field
*/
u8 mac_tx_getActualPayloadPos(mac_genericFrame_t* pData)
{
    u8 *pFrameCrtl = (u8*)((u32)pData+pData->data.tx.psduOffset+TELINK_RF_TX_HDR_LEN);
    if (((*pFrameCrtl) & 0x07) == MAC_FRAME_TYPE_DATA) {
        return (u8)(((u32)(pData->msdu)) - ((u32)pData));
    }
    else if (((*pFrameCrtl) & 0x07) == MAC_FRAME_TYPE_BEACON) {
        return (u8)(((u32)(pData->msdu)) - ((u32)pData) + 4);
    }
    else if (((*pFrameCrtl) & 0x07) == MAC_FRAME_TYPE_COMMAND) {
        return (u8)(((u32)(pData->msdu)) - ((u32)pData) + 1);
    }
    else {
        return 0;
    }
}

/**
* @brief   get the offset of the Frame Control field from the start of
*          the received mac_genericFrame_t type mac frame.
*
* @param   pData - the received frame
*
* @return  the offset of the Frame Control field
*/
u8 mac_rx_getFrameControlPos(mac_genericFrame_t* pData)
{
    return  TELINK_RF_RX_HDR_LEN;
}

/**
 * @brief   get the length of the MAC header  of
 *          the received mac_genericFrame_t type mac frame.
 *
 * @param   pData - the received frame
 *
 * @return  the length of the MAC header
 */
u8 mac_rx_getMACHeaderLen(mac_genericFrame_t* pData)
{
    rx_buf_t* pRxEvt = (rx_buf_t*)ev_buf_getTail((void*)pData, sizeof(rx_buf_t));
    u8 *pBuf = pRxEvt->rxBuf;
    u8 hdrLen = MAC_MIN_HDR_LEN;
    u8 *pFrame = pBuf + TELINK_RF_RX_HDR_LEN;
    u8 *p = pFrame;
    u16 fcf;
    u8 fIntraPan = 0;

    fcf = ( (*(p+1)) << 8) | (*p);
    p +=3;

    if (MAC_FRAME_TYPE_BEACON == (*pFrame & (u8)MAC_FCF_FRAME_TYPE_MASK)) {
        if (ADDR_MODE_SHORT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
            hdrLen = 7;
        }
        else if (ADDR_MODE_EXT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
            hdrLen = 13;
        }
    }
    else {
        /* Check PAN id */
        if (!(fcf & MAC_FCF_INTRA_PAN_MASK)) {
            fIntraPan = 0;
        }
        else { /* only one PAN id is present */
            fIntraPan = 1;
            /*skip dstPanID*/
            p += 2;
            hdrLen += 2;
        }

        /* Check Destination address */
        if (ADDR_MODE_NONE != (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
            if (fIntraPan == 0) {
                /*skip dstPanID*/
                hdrLen += 2;
                p += 2;
            }

            if (ADDR_MODE_SHORT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
                /*skip shortAddr*/
                hdrLen += 2;
                p += 2;
            }
            else if (ADDR_MODE_EXT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
                /*skip extAddr*/
                hdrLen += 8;
                p += 8;

            }
        }
        else {
        }

        /* Check Source address */
        if (ADDR_MODE_NONE != (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
            if (fIntraPan == 0) {
                /*skip srcPanID*/
                hdrLen += 2;
                p += 2;
            }

            if (ADDR_MODE_SHORT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
                /*skip shortAddr*/
                p += 2;
                hdrLen += 2;
            }
            else if (ADDR_MODE_EXT == (fcf & MAC_FCF_SRC_ADDR_MODE_MASK) >> MAC_FCF_SRC_ADDR_MODE_POS) {
                /*skip extAddr*/
                p += EXT_ADDR_LEN;
                hdrLen += 8;
            }
        }
    }
    return hdrLen;
}

/**
  * @brief   get the offset of the actual payload field from the start of
  *          the received mac_genericFrame_t type mac frame .
  *
  * @param   pData - the received frame
  *
  * @param   macHDRLen - the length of the MAC header
  *
  * @param   auxLen - the length of the Auxiliary security header field
  *
  * @return  the offset of the actual payload field
  */
u8 mac_rx_getActualPayloadPos(mac_genericFrame_t *pData, u8 macHDRLen, u8 auxLen)
{
    u8 *pFrameCrtl = (u8 *)((u32)pData + TELINK_RF_RX_HDR_LEN);
    if (((*pFrameCrtl) & 0x07) == MAC_FRAME_TYPE_DATA) {
        return (TELINK_RF_RX_HDR_LEN + macHDRLen + auxLen);
    }
    else if (((*pFrameCrtl) & 0x07) == MAC_FRAME_TYPE_BEACON) {
        return (TELINK_RF_RX_HDR_LEN + macHDRLen + auxLen + 4);
    }
    else if (((*pFrameCrtl) & 0x07) == MAC_FRAME_TYPE_COMMAND) {
        return (TELINK_RF_RX_HDR_LEN + macHDRLen + auxLen + 1);
    }
    else {
        return 0;
    }
}

u8 *mac_getDstPANIDPos(u8 *pFrameCtrl)
{
    return (pFrameCtrl + 3);
}

u8 *mac_getSrcPANIDPos(u8 *pFrameCtrl)
{
   u16 fcf = *(pFrameCtrl+1);
   fcf <<= 8;
   fcf |= *pFrameCtrl;

   if (fcf & MAC_FCF_INTRA_PAN_MASK) {
       return mac_getDstPANIDPos(pFrameCtrl);
   }

   if (ADDR_MODE_SHORT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
       return (pFrameCtrl + 7);
   }
   else if (ADDR_MODE_EXT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
       return (pFrameCtrl + 13);
   }
   else {
       return NULL;
   }
}

u8 *mac_getDstAddrPos(u8 *pFrameCtrl)
{
    return (pFrameCtrl + 5);
}

u8 *mac_getSrcAddrPos(u8 *pFrameCtrl)
{
    u16 fcf = *(pFrameCtrl+1);
    fcf <<= 8;
    fcf |= *pFrameCtrl;

    if (fcf & MAC_FCF_INTRA_PAN_MASK) {
        if (ADDR_MODE_SHORT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
            return (pFrameCtrl + 7);
        }
       else {
           return (pFrameCtrl + 13);
       }
    }
    else {
        if (ADDR_MODE_SHORT == (fcf & MAC_FCF_DST_ADDR_MODE_MASK) >> MAC_FCF_DST_ADDR_MODE_POS) {
            return (pFrameCtrl + 9);
        }
        else {
            return (pFrameCtrl + 15);
        }
    }
}
