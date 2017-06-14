#pragma once
#include "mac_include.h"

#define RF_CHANNEL_TO_FREQUENCY(n)        (5+5*(n-11))

/*
 * Definition for RF Rx buffer format
 */
typedef struct {
    u8* rxBuf;
    u8  len;
    u8  fPending;
    u8  rssi;
    u8  reserved;
    u32 timestamp;
} rx_buf_t;

/**        
 *  @brief Definition of MAC generic frame type, used in both TX and RX
 */
typedef struct {
    u8    primitive;
    union {
        struct {
            u8 cmdID:4;
            u8 fAck:1;                 /*!< Used in TX  */
            u8 fFramePending:1;        /*!< Used in Poll  */
            u8 reserved:2;
        } bf;
        u8 byteVal;
    } flags;
    addr_t srcAddr;
    u16   dstPanId;                    /*!< Destination PAN ID  */

    union {
        /* Used for TX */
        struct {
            u8     psduLen;
            //u8     *psdu;
            u32    psduOffset;
            u8     cnfStatus;
            u8     msduHandle;
            u8     reserved[12];
        } tx;

        /* Used for RX */
        struct {
            u8     msduLen;
            u16    srcPanId;
            addr_t dstAddr;           
            u8     lqi;      
            u32    timestamp;
            u8     frameType;
            u8     dsn;
        } rx;
    } data;

    u8    reserved[15];
    u8    msdu[1];
} mac_genericFrame_t;

void mac_sendFrame(mac_genericFrame_t *pFrame);

