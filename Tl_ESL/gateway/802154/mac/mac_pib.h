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



/** @addtogroup mac_pib_id MAC PIB
 * Standard MAC PIB Get and Set Attributes
 * @{
 */
#define MAC_ACK_WAIT_DURATION             64  
#define MAC_ASSOCIATION_PERMIT            65  
#define MAC_AUTO_REQUEST                  66  
#define MAC_BATT_LIFE_EXT                 67  
#define MAC_BATT_LIFE_EXT_PERIODS         68  
#define MAC_BEACON_PAYLOAD                69 
#define MAC_BEACON_PAYLOAD_LENGTH         70  
#define MAC_BEACON_ORDER                  71  
#define MAC_BEACON_TX_TIME                72  
#define MAC_BSN                           73  
#define MAC_COORD_EXTENDED_ADDRESS        74  
#define MAC_COORD_SHORT_ADDRESS           75  
#define MAC_DSN                           76  
#define MAC_GTS_PERMIT                    77  
#define MAC_MAX_CSMA_BACKOFFS             78  
#define MAC_MIN_BE                        79  
#define MAC_PAN_ID                        80  
#define MAC_PROMISCUOUS_MODE              81  
#define MAC_RX_ON_WHEN_IDLE               82  
#define MAC_SHORT_ADDRESS                 83  
#define MAC_SUPERFRAME_ORDER              84  
#define MAC_TRANSACTION_PERSISTENCE_TIME  85  
#define MAC_ASSOCIATED_PAN_COORD          86  
#define MAC_MAX_BE                        87  
#define MAC_MAX_FRAME_TOTAL_WAIT_TIME     88  
#define MAC_MAX_FRAME_RETRIES             89  
#define MAC_RESPONSE_WAIT_TIME            90  
#define MAC_SYNC_SYMBOL_OFFSET            91  
#define MAC_TIMESTAMP_SUPPORTED           92  
#define MAC_SECURITY_ENABLED              93  
//added for MAC security
#define MAC_KEY_TABLE                     0x71  //!< A table of KeyDescriptor entries
#define MAC_KEY_TABLE_ENTRIES             0x72  //!< The number of entries in macKeyTable
#define MAC_DEVICE_TABLE                  0x73  //!< A table of DeviceDescriptor entries
#define MAC_DEVICE_TABLE_ENTRIES          0x74  //!< The number of entries in macDeviceTable.
#define MAC_SECURITY_LEVEL_TABLE          0x75  //!< A table of SecurityLevelDescriptor entries
#define MAC_SECURITY_LEVEL_TABLE_ENTRIES  0x76  //!< The number of entries in macSecurityLevelTable.
#define MAC_FRAME_COUNTER                 0x77  //!< The outgoing frame counter for this device
#define MAC_AUTO_REQUEST_SECURITY_LEVEL   0x78  //!< The security level used for automatic data requests
#define MAC_AUTO_REQUEST_KEYID_MODE       0x79  //!< The key identifier mode used for automatic data requests. This attribute is invalid if the macAutoRequestSecurityLevel attribute is set to 0x00.
#define MAC_AUTO_REQUEST_KEY_SOURCE       0x7a  //!< The originator of the key used for automatic data requests. This attribute is invalid if the macAutoRequestKeyIdMode element is invalid or set to 0x00.
#define MAC_AUTO_REQUEST_KEY_INDEX        0x7b  //!< The index of the key used for automatic data requests. This attribute is invalid if the macAutoRequestKeyIdMode attribute is invalid or set to 0x00.
#define MAC_DEFAULT_KEY_SOURCE            0x7c  //!< The originator of the default key used for key identifier mode 0x01
#define MAC_PAN_COORD_EXTENDED_ADDRESS    0x7d  //!< The 64-bit address of the PAN coordinator.
#define MAC_PAN_COORD_SHORT_ADDRESS       0x7e  //!< The 16-bit short address assigned to the PAN coordinator. A value of 0xfffe indicates that the PAN coordinator is only using its 64-bit extended address. A value of 0xffff indicates that this value is unknown.

#define MAC_PHY_TRANSMIT_POWER            0xE0  //!<  The transmit power in units of -1 dBm
#define MAC_LOGICAL_CHANNEL               0xE1  //!<  The logical channel
#define MAC_EXTENDED_ADDRESS              0xE2  //!<  The extended address of the device
#define MAC_ALT_BE                        0xE3  //!<  alternate minimum backoff exponent
#define MAC_DEVICE_BEACON_ORDER           0xE4  //!<  Device beacon order
#define MAC_PHY_TRANSMIT_POWER_SIGNED     0xE5  //!<  Duplicate transmit power attribute in signed (2's complement) dBm unit
                                                 
/** @} end of group mac_pib_id */

/** @} end of group MAC_Constant */


/*
 *  @brief MAC security related consts
 */
#define MAC_DEV_TABLE_MAX_LEN               3  /*maximum number of devices that can securely communicate with the local device*/
#define MAC_KEY_TABLE_MAX_LEN               3
#define LOOKUP_MAX_NUM_PER_ONE_KEY          1 /*one KeyIDLookupDesc entry correspondings to one KeyDescriptor in macKeyTable*/
#define MAC_KEYID_LOOKUP_TABLE_MAX_LEN      (MAC_KEY_TABLE_MAX_LEN*LOOKUP_MAX_NUM_PER_ONE_KEY)
#define MAC_KEY_USAGE_TABLE_MAX_LEN         (6*MAC_KEY_TABLE_MAX_LEN) /* data frame + beacon frame + 4 command frame(data request,beacon request,associate request,associate response) */
#define MAC_KEY_DEV_TABLE_MAX_LEN           (MAC_KEY_TABLE_MAX_LEN*MAC_DEV_TABLE_MAX_LEN)  /*one entry correspondings to one DeviceDescriptor in macDeviceTable*/
#define MAC_SEC_LVL_TABLE_MAX_LEN           6 /* data frame + beacon frame + 4 command frame(data request,beacon request,associate request,associate response) */

typedef enum {
    MAC_SECMATERIAL_KEYID_LOOKUP_DESC = 0,
    MAC_SECMATERIAL_DEVICE_DESC,
    MAC_SECMATERIAL_KEYUSAGE_DESC,
    MAC_SECMATERIAL_SECLEVEL_DESC,
    MAC_SECMATERIAL_KEY_DESC,
    MAC_SECMATERIAL_KEYDEV_DESC,
} mac_secMaterial_t;

/*
 *  @brief Structure of MAC information base
 */
typedef struct {
    u8             ackWaitDuration;
    bool           associationPermit;
    bool           autoRequest;
    bool           battLifeExt;
    u8             battLifeExtPeriods;

    u8             *pBeaconPayload;
    u8             beaconPayloadLength;
    u8             beaconOrder;
    u32            beaconTxTime;
    u8             bsn;

    addr_t         coordExtendedAddress;
    u16            coordShortAddress;
    u8             dsn;
    bool           gtsPermit;
    u8             maxCsmaBackoffs;

    u8             minBe;
    u16            panId;
    bool           promiscuousMode;
    bool           rxOnWhenIdle;
    u16            shortAddress;

    u8             superframeOrder;
    u16            transactionPersistenceTime;
    bool           associatedPanCoord;
    u8             maxBe;
    u16            maxFrameTotalWaitTime;

    u8             maxFrameRetries;
    u8             responseWaitTime;
    u8             syncSymbolOffset;
    bool           timeStampSupported;
    bool           securityEnabled;

    /* security */
    void           *keyTable;
    u8             keyTableEntries;
    void           *devTable;
    u8             devTableEntries;
    void           *secLvlTable;
    u8             secLvlTableEntries;
    u32            frameCounter;
    u8             autoReqSecLvl;
    u8             autoReqKeyIdMode;
    u8             autoReqKeySource[8];
    u8             autoReqKeyIndex;
    u8             defaultKeySource[8];
    u8             panCoordExtAddr[8];
    u16            panCoordShortAddr;

    /* Proprietary */
    u8             phyTransmitPower;
    u8             logicalChannel;
    addrExt_t      extendedAddress;
    u8             altBe;
    u8             deviceBeaconOrder;

} mac_pib_t;

typedef struct {
    u8              primitive;            //!< Primitive id of MLME Reset Confirm @ref mac_primitive_id
    u8              status;               //!< The status of the reset @ref mac_status_id
    u8              pibAttribute;
    u8              pibAttributeIndex;    //add "pibAttributeIndex" member for MAC PIB attributes that are tables
} mac_setCnf_t;

typedef struct {
    u8              primitive;
    u8             	pibAttribute;
    u8              pibAttributeIndex;    //add "pibAttributeIndex" member for MAC PIB attributes that are tables
    u64             pibAttributeValue;
} mac_setReq_t;

typedef struct {
    u8              primitive;
    u8             	pibAttribute;
    u8              pibAttributeIndex;    //add "pibAttributeIndex" member for MAC PIB attributes that are tables
} mac_getReq_t;

typedef struct {
    u8              primitive;
    u8              status;
    u8              pibAttribute;
    u8              pibAttributeIndex;    //add "pibAttributeIndex" member for MAC PIB attributes that are tables
    u64             pibAttributeValue;
} mac_getCnf_t;
/*
 *  @brief MAC PIB variable for external use
 */
extern mac_pib_t macPib;

/** @addtogroup  MAC_FUNCTIONS MAC API
 *  @{
 */


 /**
  * @brief       Reset the MAC PIB to default value
  *
  * @param       None
  *
  * @return      None
  */
void mac_pibReset(void);


 /**
  * @brief       Set an attribute value in the MAC PIB
  *
  * @param[in]   pibAttribute - The attribute identifier @ref mac_pib_id
  * @param[in]   pibIndex - The index of attributes that are tables
  * @param[in]   pValue       - Pointer to the attribute value
  *
  * @return      MAC Status @ref mac_status_id
  */
mac_sts_t mac_mlmeSetReq(u8 pibAttribute, u8 pibIndex, void *pValue);


 /**
  * @brief       Get an attribute value in the MAC PIB
  *
  * @param[in]   pibAttribute - The attribute identifier @ref mac_pib_id
  * @param[in]   pibIndex - The index of attributes that are tables
  * @param[out]  pValue       - Pointer to the attribute value
  *
  * @return      MAC Status @ref mac_status_id
  */
mac_sts_t mac_mlmeGetReq(u8 pibAttribute, u8 pibIndex, void *pValue);

/**
  * @brief       Delete an entry in the MAC PIB Table Attribute
  *
  * @param[in]   pibAttribute - The attribute identifier @ref mac_pib_id
  * @param[in]   pibIndex - The index of attributes that are tables
  *
  * @return      MAC Status @ref mac_status_id
  */
mac_sts_t mac_mlmeDelReq(u8 pibAttribute, u8 pibIndex);

void *mac_secMaterial_alloc(mac_secMaterial_t type);
char mac_secMaterial_free(mac_secMaterial_t type, void *item);


/**  @} end of group MAC_FUNCTIONS */

/**  @} end of group MAC_Module */

/**  @} end of group TELINK_802_15_4_STACK */

