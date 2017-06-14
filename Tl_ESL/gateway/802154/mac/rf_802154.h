#pragma once

#include "mac_include.h"

/**
 * Default RF Channel when power up
 */
#define DFLT_LOGIC_CHANNEL         11

/**
 *  @brief  Definition for TX power level
 */
#define PHY_TX_POWER_MAX           0
#define PHY_TX_POWER_5DB           1
#define PHY_TX_POWER_0DB           2
#define PHY_TX_POWER_N_5DB         3
#define PHY_TX_POWER_N_10DB        4
#define PHY_TX_POWER_N_13DB        5
#define PHY_TX_POWER_N_18DB        6
#define PHY_TX_POWER_N_23DB        7
#define PHY_TX_POWER_N_27DB        8
#define PHY_TX_POWER_N_30DB        9
#define PHY_TX_POWER_N_37DB        10
#define PHY_TX_POWER_MIN           100

#define RF_RXBUF_MAX_SIZE          192

enum {
    /* Protocol Specific ID*/
    RF_ID_ZIGBEE                = 0xA0,
    RF_ID_MAC_SHORT_ADDR,
    RF_ID_MAC_EXT_ADDR,
    RF_ID_MAC_PANID,
    RF_ID_MAC_FRAME_PENDING,
    RF_ID_MAC_PAN_COORD,
    RF_ID_MAC_RECV_START,
    RF_ID_BECON_RECV,

    RF_ID_BLE                   = 0xB0,
};

extern u8 *rf_rxBuf;
extern u8 *rf_rxBackupBuf;

/*********************************************************************
 * TYPES
 */

/*
 * Definition of callback function type
 */
typedef void (*rf_txCB_t)(void *arg);
typedef void (*rf_rxCB_t)(void *arg); 
typedef void (*rf_ackCB_t)(int fPendingFrame, u8 seq);

typedef void (*rf_protocolSpecificInit_t)(void);
typedef void (*rf_protocolSpecificReset_t)(void);
typedef void (*rf_protocolSpecificSet_t)(u8 id, u8 *pValue, u8 len);

void rf_802154_set(u8 id, u8 *pValue, u8 len);
void rf_setCBFunc(rf_txCB_t txCbFunc);
void rf_setAckCB(rf_ackCB_t ackCbFunc);
void rf_tx(u8 *buf, u8 len);
