/**************************************************************************************************
  Filename:       mac_const.h
  Revised:        $Date: 2013-05-27 $

  Description:    Constant definition in MAC layer 
**************************************************************************************************/

#pragma once


#define MAC_DEBUG

#define MAC_DEVICE_FFD  1
//#define MAC_DEVICE_RFD  1


#ifdef MAC_DEVICE_FFD
    #define MAC_SUPPORTED_START                         1

    #define MAC_SUPPORTED_MLME_ASSOCIATE_REQ            1
    #define MAC_SUPPORTED_MLME_ASSOCIATE_CNF            1
    #define MAC_SUPPORTED_MLME_ASSOCIATE_IND            1
    #define MAC_SUPPORTED_MLME_ASSOCIATE_RSP            1

    #define MAC_SUPPORTED_MLME_DISASSOCIATE_REQ         1
    #define MAC_SUPPORTED_MLME_DISASSOCIATE_CNF         1
    #define MAC_SUPPORTED_MLME_DISASSOCIATE_IND         1

    #define MAC_SUPPORTED_MLME_BEACON_NOTIFY_IND        1

    #define MAC_SUPPORTED_MLME_GET_REQ                  1
    #define MAC_SUPPORTED_MLME_GET_CNF                  1

    #define MAC_SUPPORTED_MLME_GTS_REQ                  0
    #define MAC_SUPPORTED_MLME_GTS_CNF                  0
    #define MAC_SUPPORTED_MLME_GTS_IND                  0

    #define MAC_SUPPORTED_MLME_ORPHAN_IND               1
    #define MAC_SUPPORTED_MLME_ORPHAN_RSP               1

    #define MAC_SUPPORTED_MLME_RESET_REQ                1
    #define MAC_SUPPORTED_MLME_RESET_CNF                1

    #define MAC_SUPPORTED_MLME_RX_ENABLE_REQ            0
    #define MAC_SUPPORTED_MLME_RX_ENABLE_CNF            0

    #define MAC_SUPPORTED_MLME_SCAN_REQ                 1
    #define MAC_SUPPORTED_MLME_SCAN_CNF                 1

    #define MAC_SUPPORTED_MLME_COMM_STATUS_IND          1

    #define MAC_SUPPORTED_MLME_SET_REQ                  1
    #define MAC_SUPPORTED_MLME_SET_CNF                  1

    #define MAC_SUPPORTED_MLME_START_REQ                1
    #define MAC_SUPPORTED_MLME_START_CNF                1

    #define MAC_SUPPORTED_MLME_SYNC_REQ                 0
    #define MAC_SUPPORTED_MLME_SYNC_LOSS_IND            1

    #define MAC_SUPPORTED_MLME_POLL_REQ                 1
    #define MAC_SUPPORTED_MLME_POLL_CNF                 1


#endif  /* MAC_DEVICE_FFD */


#ifdef MAC_DEVICE_RFD
    #define MAC_SUPPORTED_START                         0

    #define MAC_SUPPORTED_MLME_ASSOCIATE_REQ            1
    #define MAC_SUPPORTED_MLME_ASSOCIATE_CNF            1
    #define MAC_SUPPORTED_MLME_ASSOCIATE_IND            0
    #define MAC_SUPPORTED_MLME_ASSOCIATE_RSP            0

    #define MAC_SUPPORTED_MLME_DISASSOCIATE_REQ         1
    #define MAC_SUPPORTED_MLME_DISASSOCIATE_CNF         1
    #define MAC_SUPPORTED_MLME_DISASSOCIATE_IND         1

    #define MAC_SUPPORTED_MLME_BEACON_NOTIFY_IND        1

    #define MAC_SUPPORTED_MLME_GET_REQ                  1
    #define MAC_SUPPORTED_MLME_GET_CNF                  1

    #define MAC_SUPPORTED_MLME_GTS_REQ                  0
    #define MAC_SUPPORTED_MLME_GTS_CNF                  0
    #define MAC_SUPPORTED_MLME_GTS_IND                  0

    #define MAC_SUPPORTED_MLME_ORPHAN_IND               0
    #define MAC_SUPPORTED_MLME_ORPHAN_RSP               0

    #define MAC_SUPPORTED_MLME_RESET_REQ                1
    #define MAC_SUPPORTED_MLME_RESET_CNF                1

    #define MAC_SUPPORTED_MLME_RX_ENABLE_REQ            0
    #define MAC_SUPPORTED_MLME_RX_ENABLE_CNF            0

    #define MAC_SUPPORTED_MLME_SCAN_REQ                 1
    #define MAC_SUPPORTED_MLME_SCAN_CNF                 1

    #define MAC_SUPPORTED_MLME_COMM_STATUS_IND          1

    #define MAC_SUPPORTED_MLME_SET_REQ                  1
    #define MAC_SUPPORTED_MLME_SET_CNF                  1

    #define MAC_SUPPORTED_MLME_START_REQ                0
    #define MAC_SUPPORTED_MLME_START_CNF                0

    #define MAC_SUPPORTED_MLME_SYNC_REQ                 0
    #define MAC_SUPPORTED_MLME_SYNC_LOSS_IND            1

    #define MAC_SUPPORTED_MLME_POLL_REQ                 1
    #define MAC_SUPPORTED_MLME_POLL_CNF                 1

#endif  /* MAC_DEVICE_RFD */

