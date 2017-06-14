#include "mac_rx_enable.h"

static ev_time_event_t *mac_rxOnTimer;

static int mac_rxOnEnableCb(void *arg);
static void mac_sendRxOnEnableConfirm(u8 *pData);

void mac_rxOnEnableReqHanler(u8* pData)
{
    mac_rxOnEnableReq_t *pRxEnable = (mac_rxOnEnableReq_t*)pData;
    u32 rxOnDuration;
    u8 value;
    /* timer is running, stop it */ 
    if (mac_rxOnTimer) {
        ev_unon_timer(&mac_rxOnTimer);
    } 
    
    /* ignore the highest byte. */
    rxOnDuration = pRxEnable->rxOnTime & 0x00ffffff;
    if (rxOnDuration == 0) {
    	value = 0;
    } 
    else if ( rxOnDuration == 0x00ffffff ) {
    	value = 1;        
    } 
    else {        
        value = 1;
        /* change to second */
        rxOnDuration = rxOnDuration * MAC_SPEC_USECS_PER_SYMBOL;
        mac_rxOnTimer = ev_on_timer(mac_rxOnEnableCb, NULL, rxOnDuration);
    }
    
    /* set to mac pib */
    mac_mlmeSetReq(MAC_RX_ON_WHEN_IDLE, 0, &value);
    /* send the confirm */
    mac_rxOnEnableCnf_t *pRxCnf = (mac_rxOnEnableCnf_t*)pData;
    pRxCnf->primitive = MAC_MLME_RX_ENABLE_CNF;
    pRxCnf->status = MAC_SUCCESS;
    mac_sendRxOnEnableConfirm((u8*)pRxCnf);
}

static int mac_rxOnEnableCb(void *arg)
{
	u8 value = 0;
	mac_mlmeSetReq(MAC_RX_ON_WHEN_IDLE, 0, &value);
    mac_rxOnTimer = NULL;
    return -1;
}

static void mac_sendRxOnEnableConfirm(u8 *pData)
{
    buf_message_post(pData,BUF_ITEM_STATE_MAC2NWK);
}

void mac_mcpsRxOnReq(mac_rxOnEnableReq_t* pRxOnEnReq)
{
    buf_message_post((u8 *)pRxOnEnReq,BUF_ITEM_STATE_NWK2MAC);
}
