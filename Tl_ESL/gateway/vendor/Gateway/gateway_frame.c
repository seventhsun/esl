#include "../../drivers.h"
#include "../../common.h"
#include "gateway_frame.h"
#include "gateway_interface.h"

#define GATEWAY_FRAME_BUF_LEN    128

static unsigned char frame_buffer[GATEWAY_FRAME_BUF_LEN];

static Gateway_ResultHandler Gateway_ReadRssiDoneHandler = NULL;
static Gateway_ResultHandler Gateway_SetMacDoneHandler = NULL;
static Gateway_ResultHandler Gateway_SetPanIDDoneHandler = NULL;
static Gateway_ResultHandler Gateway_SetChannelDoneHandler = NULL;
static Gateway_ResultHandler Gateway_SetPowerDoneHandler = NULL;
static Gateway_ResultHandler Gateway_TxPacketDoneHandler = NULL;
static Gateway_ResultHandler Gateway_RxPacketHandler = NULL;

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

int Gateway_ReadRssi(void)
{
    unsigned char len = 0;
    unsigned char *p = frame_buffer;

    *p++ = GATEWAY_CMD_READ_RSSI;
    *p++ = 0;

    len = p - frame_buffer;
    if (len == ParaBuf_Write(frame_buffer, len)) {
        return 1;
    }
    else {
        return 0;
    }
}

int Gateway_SetMac(unsigned short mac)
{
    unsigned char len = 0;
    unsigned char *p = frame_buffer;

    *p++ = GATEWAY_CMD_SET_MAC;
    *p++ = 2;
    *p++ = mac & 0xff;
    *p++ = mac >> 8;

    len = p - frame_buffer;
    if (len == ParaBuf_Write(frame_buffer, len)) {
        return 1;
    }
    else {
        return 0;
    }
}

int Gateway_SetPanID(unsigned short pan_id)
{
    unsigned char len = 0;
    unsigned char *p = frame_buffer;

    *p++ = GATEWAY_CMD_SET_PANID;
    *p++ = 2;
    *p++ = pan_id & 0xff;
    *p++ = pan_id >> 8;

    len = p - frame_buffer;
    if (len == ParaBuf_Write(frame_buffer, len)) {
        return 1;
    }
    else {
        return 0;
    }
}

int Gateway_SetChannel(unsigned char channel)
{
    unsigned char len = 0;
    unsigned char *p = frame_buffer;

    *p++ = GATEWAY_CMD_SET_CHANNEL;
    *p++ = 1;
    *p++ = channel;

    len = p - frame_buffer;
    if (len == ParaBuf_Write(frame_buffer, len)) {
        return 1;
    }
    else {
        return 0;
    }
}

int Gateway_SetPower(unsigned char power)
{
    unsigned char len = 0;
    unsigned char *p = frame_buffer;

    *p++ = GATEWAY_CMD_SET_POWER;
    *p++ = 1;
    *p++ = power;

    len = p - frame_buffer;
    if (len == ParaBuf_Write(frame_buffer, len)) {
        return 1;
    }
    else {
        return 0;
    }
}

int Gateway_TxPacket(unsigned short dst_addr, unsigned short dst_pan_id, unsigned char *data, unsigned char len)
{
    unsigned char length = 0;
    unsigned char *p = frame_buffer;

    *p++ = GATEWAY_CMD_TX_PACKET;
    *p++ = 4 + len;
    *p++ = dst_addr & 0xff;
    *p++ = dst_addr >> 8;
    *p++ = dst_pan_id & 0xff;
    *p++ = dst_pan_id >> 8;
    memcpy(p, data, len);
    p += len;

    length = p - frame_buffer;
    if (len == ParaBuf_Write(frame_buffer, length)) {
        return 1;
    }
    else {
        return 0;
    }
}

int Gateway_SetHandlers(Gateway_ResultHandler ReadRssiDoneHandler,
                        Gateway_ResultHandler SetMacDoneHandler,
                        Gateway_ResultHandler SetPanIDDoneHandler,
                        Gateway_ResultHandler SetChannelDoneHandler,
                        Gateway_ResultHandler SetPowerDoneHandler,
                        Gateway_ResultHandler TxPacketDoneHandler,
                        Gateway_ResultHandler RxPacketHandler)
{
    Gateway_ReadRssiDoneHandler = ReadRssiDoneHandler;
    Gateway_SetMacDoneHandler = SetMacDoneHandler;
    Gateway_SetPanIDDoneHandler = SetPanIDDoneHandler;
    Gateway_SetChannelDoneHandler = SetChannelDoneHandler;
    Gateway_SetPowerDoneHandler = SetPowerDoneHandler;
    Gateway_TxPacketDoneHandler = TxPacketDoneHandler;
    Gateway_RxPacketHandler = RxPacketHandler;
}

int Gateway_IrqHandler(void)
{
    while (ResBuf_Read(frame_buffer, sizeof(frame_buffer))) {
        if ((GATEWAY_RES_READ_RSSI_DONE == frame_buffer[0]) && Gateway_ReadRssiDoneHandler) {
            Gateway_ReadRssiDoneHandler(frame_buffer);
        }
        else if ((GATEWAY_RES_SET_MAC_DONE == frame_buffer[0]) && Gateway_SetMacDoneHandler) {
            Gateway_SetMacDoneHandler(frame_buffer);
        }
        else if ((GATEWAY_RES_SET_PANID_DONE == frame_buffer[0]) && Gateway_SetPanIDDoneHandler) {
            Gateway_SetPanIDDoneHandler(frame_buffer);
        }
        else if ((GATEWAY_RES_SET_POWER_DONE == frame_buffer[0]) && Gateway_SetPowerDoneHandler) {
            Gateway_SetPowerDoneHandler(frame_buffer);
        }
        else if ((GATEWAY_RES_SET_CHANNEL_DONE == frame_buffer[0]) && Gateway_SetChannelDoneHandler) {
            Gateway_SetChannelDoneHandler(frame_buffer);
        }
        else if ((GATEWAY_RES_TX_PACKET_DONE == frame_buffer[0]) && Gateway_TxPacketDoneHandler) {
            Gateway_TxPacketDoneHandler(frame_buffer);
        }
        else if ((GATEWAY_RES_RX_PACKET == frame_buffer[0]) && Gateway_RxPacketHandler) {
            Gateway_RxPacketHandler(frame_buffer);
        }
        else {

        }
    }
}


