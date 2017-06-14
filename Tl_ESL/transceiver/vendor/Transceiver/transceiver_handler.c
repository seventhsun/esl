#include "../../drivers.h"
#include "../../common.h"
#include "../../802154/mac/mac_include.h"
#include "transceiver_handler.h"

#define    CMD_LED_PIN     GPIOD_GP7
#define    TRIG_LED_PIN    GPIOB_GP4

//gateway command type definition
enum {
    GATEWAY_CMD_READ_RSSI = 0,
    GATEWAY_CMD_SET_MAC,
    GATEWAY_CMD_SET_PANID,
    GATEWAY_CMD_SET_POWER,
    GATEWAY_CMD_SET_CHANNEL,
    GATEWAY_CMD_TX_PACKET,

    GATEWAY_RES_READ_RSSI_DONE,
    GATEWAY_RES_SET_MAC_DONE,
    GATEWAY_RES_SET_PANID_DONE,
    GATEWAY_RES_SET_POWER_DONE,
    GATEWAY_RES_SET_CHANNEL_DONE,
    GATEWAY_RES_TX_PACKET_DONE,
    GATEWAY_RES_RX_PACKET,
};

static unsigned char transceiver_buf[128];

unsigned char mac_channel = 0x17;
unsigned char mac_channelPage = 0;

unsigned char tran_ext_addr[8] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
unsigned short tran_short_addr = 0x2211;
unsigned short pan_id = 0xaabb;

unsigned char node_ext_adr[8] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xfe};
unsigned char node_short_adr = 0x0001;
unsigned char test_key[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                              0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

void Blink_LED(unsigned short pin, unsigned char times)
{
    while (times--) {
        GPIO_SetBit(pin);
        WaitMs(10);
        GPIO_ResetBit(pin);
        WaitMs(40);
    }
}

void Transceiver_MACInit(void)
{
    unsigned char keySrcDefault[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff};
    unsigned int frmCnt = 1;
    unsigned short maxCsmaBackoffs = 5;
    unsigned short transPersisTime = 0x0260;
    unsigned char maxSecEn = 1;
    unsigned char rxOnWhenIdle = 1;

    unsigned char *pData;
    pData = ev_buf_allocate(0);
    if (NULL == pData) {
        while(1);
    }
    memset(pData, 0, BUFFER_SIZE);
    mac_startReq_t *pStartReq = (mac_startReq_t *)pData;

    mac_mlmeResetReq(1);

    /*set mac long address*/
    mac_mlmeSetReq(MAC_EXTENDED_ADDRESS, 0, tran_ext_addr);

    /*set mac short address*/
    mac_mlmeSetReq(MAC_SHORT_ADDRESS, 0, &tran_short_addr);

    /*set mac mac_max_csma_backoffs*/
    mac_mlmeSetReq(MAC_MAX_CSMA_BACKOFFS, 0, &maxCsmaBackoffs);

    /*set mac mac_security_enabled*/
    mac_mlmeSetReq(MAC_SECURITY_ENABLED, 0, &maxSecEn);

    /*set mac defaultKeySource*/
    mac_mlmeSetReq(MAC_DEFAULT_KEY_SOURCE, 0, keySrcDefault);

    /*set mac frameCounter*/
    mac_mlmeSetReq(MAC_FRAME_COUNTER, 0, &frmCnt);

    /*set mac rxOnWhenIdle*/
    mac_mlmeSetReq(MAC_RX_ON_WHEN_IDLE, 0, &rxOnWhenIdle);

    /*set mac transPersisTime*/
    mac_mlmeSetReq(MAC_TRANSACTION_PERSISTENCE_TIME, 0, &transPersisTime);

    /*start as coordinator*/
    pStartReq->primitive = MAC_MLME_START_REQ;
    pStartReq->panId = pan_id;
    pStartReq->logicalChannel = mac_channel;
    pStartReq->channelPage = mac_channelPage;
    pStartReq->startTime = 0;
    pStartReq->beaconOrder = 15;
    pStartReq->superframeOrder = 15;
    pStartReq->panCoordinator = 1;
    pStartReq->batteryLifeExt = 0;
    pStartReq->coordRealignment = 0;
    mac_mlmeStartReq(pStartReq);

    /*set mac rxOnWhenIdle*/
    mac_mlmeSetReq(MAC_RX_ON_WHEN_IDLE, 0, &rxOnWhenIdle);

    //set device table
    mac_deviceDesc_t *pDevDesc = mac_deviceDesc_alloc();
    if (!pDevDesc) {
        while(1);
    }
    mac_deviceDesc_set(pDevDesc, pan_id, node_short_adr, node_ext_adr, 0, 0);
    mac_mlmeSetReq(MAC_DEVICE_TABLE, 0, pDevDesc);

    //set key_id_lookup_list
    mac_keyid_lookup_desc_t *pKeyIDDesc = (mac_keyid_lookup_desc_t *)mac_keyidDesc_alloc();
    if (!pKeyIDDesc) {
        while(1);
    }
    unsigned char look_data[9];
    memset(look_data, 0, 9);
    look_data[7] = 0xff;
    look_data[8] = 0x01;
    mac_keyidDesc_set(pKeyIDDesc, LOOKUP_DATA_SIZE_9, look_data);
    
    //set key_device_list
    mac_keydevDesc_t *pKeyDevDesc = (mac_keydevDesc_t *)mac_keydevDesc_alloc();
    if (!pKeyDevDesc) {
        while(1);
    }
    mac_keydevDesc_set(pKeyDevDesc, pDevDesc, 0, 0);
    
    //set key_usage_list
    mac_keyusageDesc_t *pKeyUsgDesc[2];
    pKeyUsgDesc[0]= (mac_keyusageDesc_t *)mac_keyusageDesc_alloc();
    if (!pKeyUsgDesc[0]) {
        while(1);
    }
    mac_keyusageDesc_set(pKeyUsgDesc[0], 1, 0);
    pKeyUsgDesc[1] = (mac_keyusageDesc_t *)mac_keyusageDesc_alloc();
    if (!pKeyUsgDesc[1]) {
        while(1);
    } 
    mac_keyusageDesc_set(pKeyUsgDesc[1], 3, 4);

    //set key table
    mac_keyDesc_t *pKeyDesc = (mac_keyDesc_t *)mac_keyDesc_alloc();
    if (!pKeyDesc) {
        while(1);
    } 
    mac_keyDesc_set(pKeyDesc, &pKeyIDDesc, 1, &pKeyDevDesc, 1, pKeyUsgDesc, 2, test_key);
    mac_mlmeSetReq(MAC_KEY_TABLE, 0, pKeyDesc);  
}

void Transceiver_DataIndCb(unsigned char *pData)
{
    mac_dataInd_t *pDataInd = (mac_dataInd_t*)pData;
    unsigned char msdu_len = pDataInd->msduLen;

    transceiver_buf[0] = GATEWAY_RES_RX_PACKET;
    transceiver_buf[1] = pDataInd->msduLen + 9;
    transceiver_buf[2] = pDataInd->srcAddr.addr.shortAddr & 0xff;
    transceiver_buf[3] = pDataInd->srcAddr.addr.shortAddr >> 8;
    transceiver_buf[4] = pDataInd->srcPanId & 0xff;
    transceiver_buf[5] = pDataInd->srcPanId >> 8;
    transceiver_buf[6] = pDataInd->lqi;
    memcpy(&transceiver_buf[7], &pDataInd->timestamp, 4);
    memcpy(&transceiver_buf[11], pDataInd->msdu, msdu_len);

    if ((transceiver_buf[1]+2) == ResuBuf_Write(transceiver_buf, transceiver_buf[1]+2)) {
        GW_IrqTrig();
    }
    ev_buf_free(pData);
}

static void Transceiver_ReadRssiHandler(unsigned char *data)
{
    //TODO: trig rf to read the current rssi

    unsigned char result[3];
    result[0] = GATEWAY_RES_READ_RSSI_DONE;
    result[1] = 1;
    result[2] = 0x55;

    if (3 == ResuBuf_Write(result, 3)) {
        // Blink_LED(TRIG_LED_PIN, 1);
        GW_IrqTrig();
    }
}

static void Transceiver_SetMACHandler(unsigned char *data)
{
    unsigned char result[3];
    result[0] = GATEWAY_RES_SET_MAC_DONE;
    result[1] = 1;
    result[2] = 0;
    if (3 == ResuBuf_Write(result, 3)) {
        //call mac_mlmeSetReq to set the MAC_SHORT_ADDRESS
    	unsigned short mac_addr = data[2] + (data[3]<<8);
    	mac_mlmeSetReq(MAC_SHORT_ADDRESS, 0, &mac_addr);
        // Blink_LED(TRIG_LED_PIN, 1);
        GW_IrqTrig();
    }
}

static void Transceiver_SetPANIDHandler(unsigned char *data)
{
    unsigned char result[3];
    result[0] = GATEWAY_RES_SET_PANID_DONE;
    result[1] = 1;
    result[2] = 0;

    if (3 == ResuBuf_Write(result, 3)) {
        //call mac_mlmeSetReq to set the MAC_PAN_ID
    	unsigned short pan_id = data[2] + (data[3]<<8);
    	mac_mlmeSetReq(MAC_PAN_ID, 0, &pan_id);
        // Blink_LED(TRIG_LED_PIN, 1);
        GW_IrqTrig();
    }
}

static void Transceiver_SetPowerHandler(unsigned char *data)
{
    unsigned char result[3];
    result[0] = GATEWAY_RES_SET_POWER_DONE;
    result[1] = 1;
    result[2] = 0;
    if (3 == ResuBuf_Write(result, 3)) {
        //call mac_mlmeSetReq to set the MAC_PHY_TRANSMIT_POWER
    	mac_mlmeSetReq(MAC_PHY_TRANSMIT_POWER, 0, &data[2]);
        // Blink_LED(TRIG_LED_PIN, 1);
        GW_IrqTrig();
    }
}

static void Transceiver_SetChannelHandler(unsigned char *data)
{
    unsigned char result[3];
    result[0] = GATEWAY_RES_SET_CHANNEL_DONE;
    result[1] = 1;
    result[2] = 0;
    if (3 == ResuBuf_Write(result, 3)) {
        //call mac_mlmeSetReq to set the MAC_LOGICAL_CHANNEL
    	mac_mlmeSetReq(MAC_LOGICAL_CHANNEL, 0, &data[3]);
        // Blink_LED(TRIG_LED_PIN, 1);
        GW_IrqTrig();
    }
}

static void Transceiver_TxPacketHandler(unsigned char *data)
{
    unsigned short dst_addr = data[2] + (data[3]<<8);
    unsigned short dst_pan_id = data[4] + (data[5]<<8);
    unsigned char len = data[1] - 4;

    static unsigned char msduHandle = 0;
    unsigned char *pData = NULL;
    pData = ev_buf_allocate(0);
    if (NULL == pData) {
        while(1);
        return;
    }
    memset(pData, 0, BUFFER_SIZE);

    unsigned char result[2];
    result[0] = GATEWAY_RES_TX_PACKET_DONE;
    result[1] = 1;
    result[2] = 0;
    if (3 == ResuBuf_Write(result, 3)) {
        mac_dataReq_t *pDataReq = (mac_dataReq_t *)pData;
        pDataReq->primitive = MAC_MCPS_DATA_REQ;
        pDataReq->srcAddrMode = ADDR_MODE_SHORT;
        pDataReq->dstAddr.addr.shortAddr = dst_addr;
        pDataReq->dstAddr.addrMode = ADDR_MODE_SHORT;
        pDataReq->dstPanId = dst_pan_id;

        pDataReq->msduLen = len;
        pDataReq->msduHandle = msduHandle++;
        pDataReq->txOptions = 0x01;
        // // pDataReq->sec.securityLevel = SECURITY_LEVEL_ENC_MIC_32;
        pDataReq->sec.securityLevel = SECURITY_LEVEL_NONE;
        pDataReq->sec.key_id_mode = KEY_ID_MODE_KEY_INDEX;
        pDataReq->sec.key_index = 0x01;
        memcpy(pDataReq->msdu, &data[6], len);
        mac_mcpsDataReq(pDataReq);
        GW_IrqTrig();
    }
}

static void data_send(void);
void Transceiver_Mainloop(void)
{
    unsigned char *cmd = ParaBuf_Read();

    if (cmd) {
        if (GATEWAY_CMD_READ_RSSI == cmd[0]) {
            Transceiver_ReadRssiHandler(cmd); 
        }
        else if (GATEWAY_CMD_SET_MAC == cmd[0]) {
            Transceiver_SetMACHandler(cmd);
        }
        else if (GATEWAY_CMD_SET_PANID == cmd[0]) {
            Transceiver_SetPANIDHandler(cmd);
        }
        else if (GATEWAY_CMD_SET_POWER == cmd[0]) {
            Transceiver_SetPowerHandler(cmd);
        }
        else if (GATEWAY_CMD_SET_CHANNEL == cmd[0]) {
            Transceiver_SetChannelHandler(cmd);
        }
        else if (GATEWAY_CMD_TX_PACKET == cmd[0]) {
            Blink_LED(CMD_LED_PIN, 1);
            Transceiver_TxPacketHandler(cmd);
            // data_send();
        }
    }
}


static void data_send(void)
{
    static unsigned char msduHandle = 0;
    static unsigned int test_data = 0;
    
    unsigned char *pData;
    pData = ev_buf_allocate(0);
    if (NULL == pData) {
        while(1);
        return;
    }
    memset(pData, 0, BUFFER_SIZE);

    mac_dataReq_t *pDataReq = (mac_dataReq_t *)pData;
    pDataReq->primitive = MAC_MCPS_DATA_REQ;
    pDataReq->srcAddrMode = ADDR_MODE_SHORT;
    pDataReq->dstAddr.addr.shortAddr = 0x5566;
    pDataReq->dstAddr.addrMode = ADDR_MODE_SHORT;
    pDataReq->dstPanId = 0xddff;

    pDataReq->msduLen = 4;
    pDataReq->msduHandle = msduHandle++;
    pDataReq->txOptions = 0x01;
    // pDataReq->sec.securityLevel = SECURITY_LEVEL_ENC_MIC_32;
    pDataReq->sec.securityLevel = SECURITY_LEVEL_NONE;
    pDataReq->sec.key_id_mode = KEY_ID_MODE_KEY_INDEX;
    pDataReq->sec.key_index = 0x01;
    test_data++;
    memcpy(pDataReq->msdu, &test_data, 4);
    mac_mcpsDataReq(pDataReq); 
}

static int SendTestTimerCb(void *arg)
{
    data_send();
    return 0;
}

static ev_time_event_t *SendTestTimer = NULL;
void SendTestTimerStart()
{
    if (SendTestTimer) {
        ev_unon_timer(&SendTestTimer);
    }
    SendTestTimer = ev_on_timer(SendTestTimerCb, NULL, 500*1000);
}
