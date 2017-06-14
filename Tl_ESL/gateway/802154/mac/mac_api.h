/**
* @file      	mac_api.h
*
* @brief       	Public interface file for 802.15.4 MAC

* @date     	2013-05-29
*/
#pragma once

#include "mac_include.h"

/** @addtogroup  TELINK_802_15_4_STACK TELINK 802.15.4 Stack
 *  @{
 */

/** @addtogroup  MAC_Module MAC
 *  @{
 */
 
/** @addtogroup  MAC_Constant MAC Constants
 *  @{
 */


/** @addtogroup mac_primitive_id MAC primitive ID
 * Primitive ID of MAC request/response/indication/confirmation
 * @{
 */
#define MAC_MCPS_DATA_REQ                    0x40      //!< MCPS Data Request
#define MAC_MCPS_DATA_CNF                    0x41      //!< MCPS Data Confirm
#define MAC_MCPS_DATA_IND                    0x42      //!< MCPS Data Indication
#define MAC_MCPS_PURGE_REQ                   0x43      //!< MCPS Purge request
#define MAC_MCPS_PURGE_CNF                   0x44      //!< MCPS Purge Confirm

#define MAC_MLME_ASSOCIATE_REQ               0x45      //!< MLME Associate Request
#define MAC_MLME_ASSOCIATE_CNF               0x46      //!< MLME Associate Confirm
#define MAC_MLME_ASSOCIATE_IND               0x47      //!< MLME Associate Indication
#define MAC_MLME_ASSOCIATE_RES               0x48      //!< MLME Associate Response
#define MAC_MLME_DISASSOCIATE_REQ            0x49      //!< MLME Disassociate Request
#define MAC_MLME_DISASSOCIATE_CNF            0x4a      //!< MLME Disassociate Confirm
#define MAC_MLME_DISASSOCIATE_IND            0x4b      //!< MLME Disassociate Indication
#define MAC_MLME_BEACON_NOTIFY_IND           0x4c      //!< MLME Beacon Notify Indication
#define MAC_MLME_GET_REQ                     0x4d      //!< MLME Get Request
#define MAC_MLME_GET_CNF                     0x4e      //!< MLME Get Confirm
#define MAC_MLME_ORPHAN_IND                  0x4f      //!< MLME Orphan Indication
#define MAC_MLME_ORPHAN_RES                  0x50      //!< MLME Orphan Response
#define MAC_MLME_RESET_REQ                   0x51      //!< MLME Reset Request
#define MAC_MLME_RESET_CNF                   0x52      //!< MLME Reset Confirm
#define MAC_MLME_RX_ENABLE_REQ               0x53      //!< MLME RX Enable Request
#define MAC_MLME_RX_ENABLE_CNF               0x54      //!< MLME RX Enable Confirm
#define MAC_MLME_SCAN_REQ                    0x55      //!< MLME Scan Request
#define MAC_MLME_SCAN_CNF                    0x56      //!< MLME Scan Confirm
#define MAC_MLME_COMM_STATUS_IND             0x57      //!< MLME Comm Status Indication
#define MAC_MLME_SET_REQ                     0x58      //!< MLME Set Request
#define MAC_MLME_SET_CNF                     0x59      //!< MLME Set Confirm
#define MAC_MLME_START_REQ                   0x5a      //!< MLME Start Request
#define MAC_MLME_START_CNF                   0x5b      //!< MLME Start Confirm
#define MAC_MLME_SYNC_LOSS_IND               0x5c      //!< MLME Sync Loss Indication
#define MAC_MLME_POLL_REQ                    0x5d      //!< MLME Poll Request
#define MAC_MLME_POLL_CNF                    0x5e      //!< MLME Poll Confirm

#define MAC_MLME_BEACON_REQ_CMD              0x60      //!< internal use, Receive Beacon Request Command
#define MAC_MLME_DATA_REQ_CMD                0x61      //!< internal use, Receive Data Request Command
#define MAC_MLME_ASSO_RSP_CNF                0x62      //!< internal use, Receive Associate Response Confirm

/** @} end of group mac_primitive_id */

/** @addtogroup mac_status_id MAC Status Code
 * Definition of MAC layer status
 * @{
 */
typedef enum {
	MAC_SUCCESS                 = 0x00,
    MAC_CHANNEL_ACCESS_FAILURE  = 0xE1,
    MAC_COUNTER_ERROR           = 0xDB,
                                                           
    TL_MAC_FRAME_TOO_LONG          = 0xE5,                
                                                       
    TL_MAC_IMPROPER_KEY_TYPE       = 0xDC,                
                                                       
    TL_MAC_IMPROPER_SECURITY_LEVEL = 0xDD,                
                                                       
    TL_MAC_INVALID_ADDRESS         = 0xF5,                
                                                       
    TL_MAC_INVALID_HANDLE          = 0xE7,                
    TL_MAC_INVALID_PARAMETER       = 0xE8,                
                                                       
    TL_MAC_NO_ACK                  = 0xE9,                
                                                       
    TL_MAC_NO_BEACON               = 0xEA,                
                                                       
    TL_MAC_NO_DATA                 = 0xEB,                
                                                       
    TL_MAC_NO_SHORT_ADDRESS        = 0xEC,                
                                                       
    TL_MAC_READ_ONLY               = 0xFB,                
    TL_MAC_SECURITY_ERROR          = 0xE4,                
                                                       
    TL_MAC_TRANSACTION_EXPIRED     = 0xF0,                
                                                       
                                                       
    TL_MAC_TRANSACTION_OVERFLOW    = 0xF1,                
    TL_MAC_TX_ACTIVE               = 0xF2,                
    TL_MAC_UNAVAILABLE_KEY         = 0xF3,                
                                                       
    TL_MAC_UNSUPPORTED_ATTRIBUTE   = 0xF4,                
    TL_MAC_UNSUPPORTED_LEGACY      = 0xDE,                
                                                       
    TL_MAC_UNSUPPORTED_SECURITY    = 0xDF,                
    TL_MAC_NO_RESOURCES            = 0x1A,                
                                                       
    MAC_TX_ABORTED              = 0x1D,                
	MAC_INVALID_REQ             = 0x1F,                
} mac_sts_t;

#define MAC_FRAME_TOO_LONG              TL_MAC_FRAME_TOO_LONG                                        
#define MAC_IMPROPER_KEY_TYPE           TL_MAC_IMPROPER_KEY_TYPE                                         
#define MAC_IMPROPER_SECURITY_LEVEL     TL_MAC_IMPROPER_SECURITY_LEVEL                                   
#define MAC_INVALID_ADDRESS             TL_MAC_INVALID_ADDRESS                
#define MAC_INVALID_HANDLE              TL_MAC_INVALID_HANDLE                 
#define MAC_INVALID_PARAMETER           TL_MAC_INVALID_PARAMETER              
#define MAC_NO_ACK                      TL_MAC_NO_ACK                                                   
#define MAC_NO_BEACON                   TL_MAC_NO_BEACON                                                 
#define MAC_NO_DATA                     TL_MAC_NO_DATA                                                 
#define MAC_NO_SHORT_ADDRESS            TL_MAC_NO_SHORT_ADDRESS               
#define MAC_READ_ONLY                   TL_MAC_READ_ONLY                      
#define MAC_SECURITY_ERROR              TL_MAC_SECURITY_ERROR                 
#define MAC_TRANSACTION_EXPIRED         TL_MAC_TRANSACTION_EXPIRED                                                             
#define MAC_TRANSACTION_OVERFLOW        TL_MAC_TRANSACTION_OVERFLOW           
#define MAC_TX_ACTIVE                   TL_MAC_TX_ACTIVE                      
#define MAC_UNAVAILABLE_KEY             TL_MAC_UNAVAILABLE_KEY                                          
#define MAC_UNSUPPORTED_ATTRIBUTE       TL_MAC_UNSUPPORTED_ATTRIBUTE          
#define MAC_UNSUPPORTED_LEGACY          TL_MAC_UNSUPPORTED_LEGACY                                        
#define MAC_UNSUPPORTED_SECURITY        TL_MAC_UNSUPPORTED_SECURITY           
#define MAC_NO_RESOURCES                TL_MAC_NO_RESOURCES                   

/** @} end of group mac_status_id */


/** @addtogroup mac_scan_type MAC Scan Type
 * Scan type of MAC layer
 * @{
 */
#define MAC_SCAN_ED                          0x00    //!< Energy detect scan.
#define MAC_SCAN_ACTIVE                      0x01    //!< Active scan.
#define MAC_SCAN_PASSIVE                     0x02    //!< Passive scan.
#define MAC_SCAN_ORPHAN                      0x03    //!< Orphan scan.
/** @} end of group mac_scan_type */

/** @addtogroup mac_txoption_type MAC TX Option Type
 * TX Options bit field
 * @{
 */
#define MAC_TX_OPTION_DIRECT                 0x00    //!< non-acknowledged transmission.
#define MAC_TXOPTION_ACK                     0x01    //!< Acknowledged transmission.  The MAC will attempt to retransmit the frame until it is acknowledged
#define MAC_TXOPTION_GTS                     0x02    //!< GTS transmission (unused)
#define MAC_TXOPTION_INDIRECT                0x04    //!< Indirect transmission.  The MAC will queue the data and wait for the destination device to poll for it.  This can only be used by a coordinator device
#define MAC_TXOPTION_NO_RETRANS              0x10    //!< This proprietary option prevents the frame from being retransmitted
#define MAC_TXOPTION_NO_CNF                  0x20    //!< This proprietary option prevents a MAC_MCPS_DATA_CNF event from being sent for this frame
#define MAC_TXOPTION_ALT_BE                  0x40    //!< Use PIB value MAC_ALT_BE for the minimum backoff exponent
#define MAC_TXOPTION_PWR_CHAN                0x80    //!< Use the power and channel values in macDataReq_t instead of the PIB values
/** @} end of group mac_txoption_type */


/** @addtogroup mac_asso_rsp_status MAC Association Response Status Type
 * Associate response command frame status values
 * @{
 */
#define MAC_ASSOC_SUCCESS                     0       //!< association successful
#define MAC_ASSOC_CAPACITY                    1       //!< PAN at capacity
#define MAC_ASSOC_DENIED                      2       //!< PAN access denied
/** @} end of group mac_asso_rsp_status */

/** @addtogroup mac_comm_status_reason MAC Communication Indication Status Type
 * Comm status indication reasons
 * @{
 */
#define MAC_COMM_ASSOCIATE_RSP                0      //!< Event sent in response to MAC_AssociateRsp()
#define MAC_COMM_ORPHAN_RSP                   1      //!< Event sent in response to MAC_OrphanRsp()
#define MAC_COMM_RX_SECURE                    2      //!< Event sent as a result of receiving a secure frame
/** @} end of group mac_comm_status_reason */

/**        
 *  @brief Key source length definition in Mac security
 */
#define MAC_KEY_SOURCE_MAX_LEN          8

/**        
 *  @brief Define the max number of ED scan results supported
 */
#define MAX_ED_SCAN_RESULTS_SUPPORTED   16

/**        
 *  @brief Define the max number of PAN descriptors supported
 */
#define MAX_PAN_DESC_SUPPORTED          3

/** @} end of group MAC_Constant */

/**  @} end of group MAC_Module */

/**  @} end of group TELINK_802_15_4_STACK */
 
 /**
 *  @brief  Definition for MAC layer callback function type
 */
typedef void (*mac_cb_t)(u8 *pData);

/**
 *  @brief  Define Data request parameters type
 */
typedef struct {
    u8        primitive;               //!< Primitive id of MCPS Data Request @ref mac_primitive_id 
    u8        srcAddrMode;             //!< Source addressing mode used 
    addr_t    dstAddr;                 //!< Destination address 
    u16       dstPanId;                //!< Destination PAN ID  

    u8        msduLen;                 //!< The number of octets contained in the MSDU 
    u8        msduHandle;              //!< The handle associated with the MSDU to be transmitted  
    u8        txOptions;               //!< The transmission options for the MSDU 
    u8        reserved[20];            //!< Reserved for LMAC 
    
    mac_sec_t sec;                     //!< Security parameters 
    u8        msdu[1];                 //!< The set of octets forming the MSDU 
} mac_dataReq_t;

/**
 *  @brief  Define the structure for MCPS DATA Confirm which will be used for
 *          reporting the results of MAC data related requests from the
 *          application
 */
typedef struct {
    u8        primitive;               //!< Primitive id of MCPS Data Confirm @ref mac_primitive_id  
    u8        status;                  //!< The status of the last MSDU transmission @ref mac_status_id
    u8        msduHandle;              //!< The handle associated with the MSDU being confirmed 
    u32       timestamp;               //!< The time, in symbols, at which the data were transmitted 
} mac_dataCnf_t;

/**
 *  @brief  Define Data indication parameters type
 */
typedef struct {
    u8        primitive;               //!< Primitive id of MCPS Data Indication @ref mac_primitive_id 
    u8        dsn;                     //!< The data sequence number of the received data frame 
    addr_t    srcAddr;                 //!< Source address 
    u16       dstPanId;                //!< Destination PAN ID  

    u8        msduLen;                 //!< The number of octets contained in the MSDU being indicated 
    u16       srcPanId;                //!< Source PAN ID  
    addr_t    dstAddr;                 //!< Destination address 
    u8        lqi;                     //!< LQI value measured during reception of the MPDU 
    u32       timestamp;               //!< The time, in symbols, at which the data were received 
    u8        reserved[6];

    mac_sec_t sec;                     //!< Security parameters 
    u8        msdu[1];                 //!< The set of octets forming the MSDU being indicated 
} mac_dataInd_t;



/**
 *  @brief  Define the structure for MCPS PURGE Request which will be used by
 *          the application to purge an MSDU from the transaction
 *          queue
 */
typedef struct {
    u8         primitive;              //!< Primitive id of MCPS Purge Request @ref mac_primitive_id 
    u8         msduHandle;             //!< The handle associated with the MSDU to be purged from the transaction queue 
} mac_purgeReq_t;


/**
 *  @brief  Definition of Purge confirm type
 */
typedef struct {
    u8         primitive;              //!< Primitive id of MCPS Purge Confirm @ref mac_primitive_id 
    u8         msduHandle;             //!< The handle associated with the MSDU to be purged from the transaction queue 
    u8         status;                 //!< Status of the purge request @ref mac_status_id
} mac_purgeCnf_t;


/**
 *  @brief  Definition of PAN descriptor type
 */
typedef struct {
    u16        coordPanId;             //!< PAN identifier of the coordinator 
    addr_t     coordAddr;              //!< Coordinator address 
    u8         logicalChannel;         //!< The current logical channel occupied by the network 
    u8         channelPage;            //!< The current channel page occupied by the network 
    u16        superframeSpec;         //!< Super frame specification as specified in the received beacon frame 
    u8         gtsPermit;              //!< PAN coordinator is accepting GTS requests or not 
    u8         linkQuality;            //!< The LQI at which the network beacon was received 
    u32        timestamp;              //!< The time at which the beacon frame was received, in symbols 
    u8         securityFailure;        //!< Set to TRUE if there was an error in the security processing 
    mac_sec_t  sec;                    //!< Security parameters 
} mac_panDesc_t;

/**
 *  @brief  Define MLME scan request type 
 */
typedef struct {
    u8        primitive;               //!< Primitive id of MLME Scan Request @ref mac_primitive_id 
    u8        scanType;                //!< The type of scan to be performed @ref mac_scan_type
    u32       scanChannels;            //!< Indicate which channels are to be scanned 
    u8        scanDuration;            //!< The time spent on scanning each channel 
    u8        channelPage;             //!< Channel page on which to perform the scan 
    mac_sec_t sec;                     //!< The security parameters for orphan scan 
} mac_scanReq_t;

/**
 *  @brief  Define MLME scan confirm type 
 */
typedef struct {
    u8        primitive;                //!< Primitive id of MLME Scan Confirm @ref mac_primitive_id 
    u8        status;                   //!< Status of the scan request @ref mac_status_id
    u8        scanType;                 //!< The type of scan performed @ref mac_scan_type 
    u8        channelPage;              //!< Channel page on which the scan was performed 
    u32       unscannedChannels;        //!< Channels given in the request which were not scanned 
    u8        resultListSize;           //!< Number of elements returned in the appropriate result lists 
    union {                             //!< Define a union of energy measurements list and pan descriptor list 
        u8 energyDetectList[MAX_ED_SCAN_RESULTS_SUPPORTED]; //!< List of energy measurements 
        mac_panDesc_t panDescList[MAX_PAN_DESC_SUPPORTED];  //!< List of PAN descriptors 
    } resultList;
} mac_scanCnf_t;


/**
 *  @brief  Define MLME beacon notify indication type 
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Beacon Notify Indication @ref mac_primitive_id 
    u8              bsn;                 //!< Beacon Sequence Number 
    mac_panDesc_t   panDesc;             //!< The PAN Descriptor for the received beacon 
    u8              pendAddrSpec;        //!< The beacon pending address specification 
    u8              *pAddrList;          //!< The list of device addresses for which the sender of the beacon has data 
    u8              psduLength;          //!< Number of octets contained in the beacon payload of the beacon frame 
    u8              psdu[1];             //!< The beacon payload 
} mac_beaconNotifyInd_t;


/**
 *  @brief  Define MLME start request type 
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Start Request @ref mac_primitive_id 
    u16             panId;               //!< PAN identifier to be used by the device 
    u8              logicalChannel;      //!< Logical channel on which to begin 
    u8              channelPage;         //!< Channel page on which to begin 
    u32             startTime;           //!< Time at which to begin transmitting beacons 
    u8              beaconOrder;         //!< Indicates how often the beacon is to be transmitted 
    u8              superframeOrder;     //!< Length of the active portion of the superframe 
    u8              panCoordinator;      //!< Length of the active portion of the superframe 
    u8              batteryLifeExt;      //!< Indicates if the receiver of the beaconing device is disabled or not 
    u8              coordRealignment;    //!< Indicates if the coordinator realignment command is to be transmitted 
    mac_sec_t       coordRealignSec;     //!< Security parameters for the coordinator realignment frame 
    mac_sec_t       beaconSec;           //!< Security parameters for the beacon frame 
} mac_startReq_t;


/**
 *  @brief  Define MLME start confirm type 
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Start Confirm @ref mac_primitive_id 
    u8              status;              //!< Result of the attempt to start using an updated superframe configuration @ref mac_status_id
} mac_startCnf_t;


/**
 *  @brief  Define MLME associate request type 
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Associate Request @ref mac_primitive_id 
    u8              logicalChannel;      //!< The logical channel on which to attempt association 
    u8              channelPage;         //!< The channel page on which to attempt association 
    u16             coordPanId;          //!< The identifier of the PAN with which to associate
    addr_t          coordAddress;        //!< Coordinator address 
    u8              capabilityInfo;      //!< Operational capabilities of the associating device 
    mac_sec_t       sec;                 //!< The security parameters for this message 
} mac_associateReq_t;

/**
 *  @brief  Define MLME Rx-On enable request type 
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Rx-On Enable Request @ref mac_primitive_id 
    u32             rxOnTime;            //!< The Number of symbols measured before the receiver is to be enabled
    mac_sec_t       sec;                 //!< The security parameters for this message 
} mac_rxOnEnableReq_t;


/**
 *  @brief  Define MAC_MLME_ASSOCIATE_CNF type
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Associate Confirm @ref mac_primitive_id
    u8              status;              //!< Status of the association attempt @ref mac_status_id
    u16             assocShortAddress;   //!< Short address allocated by the coordinator on successful association 
    mac_sec_t       sec;                 //!< The security parameters for this message
} mac_associateCnf_t;

/**
 *  @brief  Define MLME associate response type
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Associate Response @ref mac_primitive_id 
    addrExt_t       deviceAddress;       //!< Extended address of the device requesting association 
    u16             assocShortAddr;      //!< 16-bit short device address allocated by the coordinator on successful association 
    u8              status;              //!< Status of the association attempt @ref mac_asso_rsp_status 
    mac_sec_t       sec;                 //!< The security parameters for this message 
} mac_associateRsp_t;


/**
 *  @brief  Define MAC_MLME_ASSOCIATE_IND type
 */
typedef struct {
    u8              primitive;           //!< Primitive id of MLME Associate Indication @ref mac_primitive_id 
    addrExt_t       deviceAddress;       //!< Extended address of the device requesting association 
    u8              capabilityInfo;      //!< Operational capabilities of the device requesting association 
    mac_sec_t       sec;                 //!< The security parameters for this message 
} mac_associateInd_t;


/**
 *  @brief  Define MLME disassociate request type
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Disassociate Request @ref mac_primitive_id 
    u16             devPanId;             //!< The identifier of the PAN of the device 
    addr_t          devAddr;              //!< Device address 
    u8              disassociateReason;   //!< The reason for the disassociation 
    u8              txIndirect;           //!< TRUE if the disassociation notification command is to be sent indirectly 
    mac_sec_t       sec;                  //!< The security parameters for this message 
} mac_disassociateReq_t;


/**
 *  @brief  Define MAC_MLME_DISASSOCIATE_IND type
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Disassociate Indication @ref mac_primitive_id 
    addrExt_t       devAddr;              //!< The address of the device sending the disassociate command 
    u8              disassociateReason;   //!< The reason for the disassociation @ref mac_disasso_reason
    mac_sec_t       sec;                  //!< The security parameters for this message 
} mac_disassociateInd_t;


/**
 *  @brief  MAC_MLME_DISASSOCIATE_CNF type
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Disassociate Confirm @ref mac_primitive_id 
    u8              status;               //!< The status of the disassociate @ref mac_status_id
    addr_t          devAddr;              //!< Device address 
    u16             panId;                //!< The identifier of the PAN of the device 
} mac_disassociateCnf_t;


/**
 *  @brief  Define MLME poll request type
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Poll Request @ref mac_primitive_id 
    u16             coordPanId;           //!< PAN identifier of the coordinator
    addr_t          coordAddress;         //!< Coordinator address
    mac_sec_t       sec;                  //!< The security parameters for this message
} mac_pollReq_t;

/**
 *  @brief  Define MLME Poll confirm type 
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Poll Confirm @ref mac_primitive_id
    u8              status;               //!< The status of the data request @ref mac_status_id
} mac_pollCnf_t;

/**
 *  @brief  Define MLME Reset confirm type 
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Reset Confirm @ref mac_primitive_id
    u8              status;               //!< The status of the reset @ref mac_status_id
} mac_resetCnf_t;


/**
 *  @brief  Define MAC_MLME_COMM_STATUS_IND type
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Comm Indication @ref mac_primitive_id 
    u16             panId;                //!< The PAN ID associated with the event
    addr_t          srcAddr;              //!< Source address
    addr_t          dstAddr;              //!< Destination address
    u8              status;               //!< The communications status
    mac_sec_t       sec;                  //!< The security parameters for this message
} mac_commStatusInd_t;

/**
 *  @brief  Define MLME Rx-On enable confirm type 
 */
typedef struct {
    u8              primitive;            //!< Primitive id of MLME Rx-on Enable Confirm @ref mac_primitive_id
    u8              status;               //!< The status of the data request @ref mac_status_id
} mac_rxOnEnableCnf_t;


/** @addtogroup  TELINK_802_15_4_STACK TELINK 802.15.4 Stack
 *  @{
 */

/** @addtogroup  MAC_Module MAC
 *  @{
 */
 
 
/** @addtogroup  MAC_FUNCTIONS MAC API
 *  @{
 */
 
 
 /**
  * @brief       Reset MAC layer
  *
  * @param[in]   fSetDfltPib - 1 indicating to reset MAC pib to default value
  *
  * @return      None
  */
void mac_reset(u8 fSetDfltPib);

 /**
  * @brief       Initialize MAC layer
  *
  * @return      None
  */
void mac_init();

 /**
  * @brief       Call this function to send a MAC layer data request
  *
  * @param[in]   pDataReq - Parameters of the data request
  *
  * @return      None
  */
void mac_mcpsDataReq(mac_dataReq_t *pDataReq);

/**
  * @brief       Call this function to send a MAC layer purge request
  *
  * @param[in]   pPurgeReq - Parameters of the purge request
  *
  * @return      None
  */
void mac_mcpsPurgeReq(mac_purgeReq_t* pPurgeReq);

 /**
  * @brief       Call this function to send a MAC layer scan request
  *
  * @param[in]   pScanReq - Parameters of the scan request
  *
  * @return      None
  */
void mac_mlmeScanReq(mac_scanReq_t *pScanReq);

 /**
  * @brief       Call this function to send a MAC layer start request
  *
  * @param[in]   pStartReq - Parameters of the start request
  *
  * @return      None
  */
void mac_mlmeStartReq(mac_startReq_t *pStartReq);

 /**
  * @brief       Call this function to send a MAC layer associate request
  *
  * @param[in]   pAssoReq - Parameters of the associate request
  *
  * @return      None
  */
void mac_mlmeAssociateReq(mac_associateReq_t* pAssoReq);

 /**
  * @brief       Call this function to send a MAC layer associate response
  *
  * @param[in]   pAssoRsp - Parameters of the associate response
  *
  * @return      None
  */
void mac_mlmeAssociateRsp(mac_associateRsp_t* pAssoRsp);

 /**
  * @brief       Call this function to send a MAC layer disassociate request
  *
  * @param[in]   pDissoReq - Parameters of the disassociate request
  *
  * @return      None
  */
void mac_mlmeDissociateReq(mac_disassociateReq_t* pDissoReq);

 /**
  * @brief       Call this function to send a MAC layer Reset primitive
  *
  * @param[in]   fSetDfltPib - 1 indicating to reset MAC pib to default value
  *
  * @return      None
  */
void mac_mlmeResetReq(u8 fSetDfltPib);

 /**
  * @brief       Call this function to send a MAC layer poll request
  *
  * @param[in]   pPollReq - Parameters of the poll request
  *
  * @return      None
  */
void mac_mlmePollReq(mac_pollReq_t* pPollReq);

 /**
  * @brief       Call this function to send a MAC layer Rx-on enable request
  *
  * @param[in]   pRxOnEnReq - Parameters of the Rx-on enable request
  *
  * @return      None
  */
void mac_mcpsRxOnReq(mac_rxOnEnableReq_t* pRxOnEnReq);

/**  @} end of group MAC_FUNCTIONS */

/**  @} end of group MAC_Module */

/**  @} end of group TELINK_802_15_4_STACK */


