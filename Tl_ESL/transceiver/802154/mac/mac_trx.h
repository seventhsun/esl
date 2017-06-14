#pragma once

#include "mac_include.h"

typedef enum {
    MAC_TX_IDLE,
    MAC_TX_CSMA,
    MAC_TX_UNDERWAY,
    MAC_TX_WAIT_ACK,
    MAC_TX_RETRY,
    MAC_TX_TIME_OUT,
} mac_txState_t;

typedef enum {
    MAC_TX_EV_NEW_DATA,
    MAC_TX_EV_CSMA_IDLE,
    MAC_TX_EV_CSMA_BUSY,
    MAC_TX_EV_SEND_SUCC,
    MAC_TX_EV_SEND_FAIL,
    MAC_TX_EV_ACK_RECV,
    MAC_TX_EV_ACK_RETRY,
    MAC_TX_EV_NO_ACK,
    MAC_TX_EV_HAVE_PENDING_DATA,
} mac_txEvt_t;

typedef enum {
    PHY_CCA_IDLE       = 0x04,
    PHY_CCA_TRX_OFF    = 0x03,
    PHY_CCA_BUSY       = 0x00,
} phy_ccaSts_t;


typedef u8 (*mac_ackPendingCheckFunc_t)(addr_t* pAddr);

#define MAC_TX_STATISTICS_EN    1
typedef struct {
    unsigned int txPacketCnt;
    unsigned int csmaFail;
    unsigned int txBusyFail;
    unsigned int txAborted;
    unsigned int noAck;
    unsigned int retry;
    unsigned int rssi_coor_to_dev;
    unsigned int rssi_dev_to_coor;
} mac_txInfo_t;
extern mac_txInfo_t txStatiInfo;

#define IS_MAC_WAITING_ACK()     (mac_getTrxState() == MAC_TX_WAIT_ACK)

 /** 
 * @fn      mac_trxReset
 
 * @brief  	Call this function to reset the mac_trx_vars
 
 * @param   none
 
 * @return  none
 
 */
void mac_trxReset(void);
 /** 
 * @fn      mac_trxReset
 
 * @brief  	Call this function to initialize the mac_trx_vars
 
 * @param   none
 
 * @return  none
 
 */
void mac_trxInit(void);
/**
* @brief   This function is called by upper layer when there is data to send.
*          Firstly, it will check whether the transcevier is idle to send
*          a new packet. If not, currently it will return as MAC_TX_ACTIVE
*          state. If idle, send a NEW DATA to the transceiver task.
*          TODO: Using the queue to buffer the data in Transceiver layer.
*
* @param   pData - New data to be sent from upper layer
*
* @return  Status of the operation
*/
mac_sts_t mac_tx(mac_genericFrame_t* pData);
/**
* @brief   This function is to get tx state.
*
* @param   none
*
* @return  Status of tx
*/
u8 mac_getTrxState(void);
/**
* @brief   	   handle ack callback function.
*
* @param[in]   fPendingFrame - 1 :the Frame Pending subfield of the acknowledgment frame is set to one
* 						       0 :the Frame Pending subfield of the acknowledgment frame is set to zero
*
* @param[in]   seq - Parameters is the value of Sequence Number field of ack frame
*
* @return 	   none
*/
void mac_ackCb(u8 fPendingFrame, u8 seq);
/**
* @brief   	   RX Interrupt callback function.
*
* @param[in]   pData - The current recieved data.
*
* @return 	   none
*/
void mac_rxCb_1(void* pData);
/**
* @brief   	   Check the recieved frame, if the frame is no problem ,Send frame to upper layer
*
* @param[in]   pRxBuf - recieved data buffer.
*
* @param[in]   len -   length of recieved data
*
* @param[in]   timestamp -   The time, in symbols, at which the data were received
*
* @param[in]   lqi -   Link Quality value of recieved data
*
* @return 	   none
*/
void mac_rxCb(u8* pRxBuf, u8 len, u32 timestamp, u8 lqi);
/**
 * @brief   tx done callback function
 *
 * @param   tx status: 0:success  1:failure
 *
 * @return  none
 */
void mac_txCb(void* status);
