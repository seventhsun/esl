#include "mac_include.h"

/* Attribute index constants, based on attribute ID values from spec */
#define MAC_ATTR_SET1_START       0x40
#define MAC_ATTR_SET1_END         0x5D
#define MAC_ATTR_SET1_OFFSET      0
#define MAC_ATTR_SET2_START       0x71
#define MAC_ATTR_SET2_END         0x7e
#define MAC_ATTR_SET2_OFFSET      (MAC_ATTR_SET1_END - MAC_ATTR_SET1_START + MAC_ATTR_SET1_OFFSET + 1)
#define MAC_ATTR_SET3_START       0xE0
#define MAC_ATTR_SET3_END         0xE5
#define MAC_ATTR_SET3_OFFSET      (MAC_ATTR_SET2_END - MAC_ATTR_SET2_START + MAC_ATTR_SET2_OFFSET + 1)


/* frame response values */
#define MAC_MAX_FRAME_RESPONSE_MIN  143
#define MAC_MAX_FRAME_RESPONSE_MAX  25776

#define MAC_IEEE_ADDRESS_IN_FLASH         0x1f000
#define MAC_IEEE_ADDRESS_IN_OTP           0x3f00

/* PIB access and min/max table type */
typedef struct {
    u8 offset;
    u8 len;
    u8 min;
    u8 max;
} mac_pibTbl_t;

const u8 startIEEEAddr[] = {0x05, 0xd5, 0xb3, 0x70};

/*
 *  @brief security related values
 */
TN_MEM_DEF(macKeyIDMem, mac_keyid_lookup_desc_t, MAC_KEYID_LOOKUP_TABLE_MAX_LEN);
TN_MEM_DEF(macDeviceMem, mac_deviceDesc_t, MAC_DEV_TABLE_MAX_LEN);
TN_MEM_DEF(macKeyUsageMem, mac_keyusageDesc_t, MAC_KEY_USAGE_TABLE_MAX_LEN);
TN_MEM_DEF(macSecLvlMem, mac_seclevelDesc_t, MAC_SEC_LVL_TABLE_MAX_LEN);
TN_MEM_DEF(macKeyMem, mac_keyDesc_t, MAC_KEY_TABLE_MAX_LEN);
TN_MEM_DEF(macKeyDevMem, mac_keydevDesc_t, MAC_KEY_DEV_TABLE_MAX_LEN);

LIST_DEF(macKeyTable);
LIST_DEF(macDevTable);
LIST_DEF(macSecLvlTable);

void *mac_secMaterial_alloc(mac_secMaterial_t type)
{
    void *ret = NULL;

    switch (type) {
        case MAC_SECMATERIAL_KEYID_LOOKUP_DESC:
            ret = tn_mem_alloc(&macKeyIDMem);
            break;
        case MAC_SECMATERIAL_DEVICE_DESC:
            ret = tn_mem_alloc(&macDeviceMem);
            break;
        case MAC_SECMATERIAL_KEYUSAGE_DESC:
            ret = tn_mem_alloc(&macKeyUsageMem);
            break;
        case MAC_SECMATERIAL_SECLEVEL_DESC:
            ret = tn_mem_alloc(&macSecLvlMem);
            break;
        case MAC_SECMATERIAL_KEY_DESC:
            ret = tn_mem_alloc(&macKeyMem);
            break;
        case MAC_SECMATERIAL_KEYDEV_DESC:
            ret = tn_mem_alloc(&macKeyDevMem);
            break;
        default:
            break;
    }

    return ret;
}

char mac_secMaterial_free(mac_secMaterial_t type, void *item)
{
    char ret = -1;

    switch (type) {
        case MAC_SECMATERIAL_KEYID_LOOKUP_DESC:
            ret = tn_mem_free(&macKeyIDMem, item);
            break;
        case MAC_SECMATERIAL_DEVICE_DESC:
            ret = tn_mem_free(&macDeviceMem, item);
            break;
        case MAC_SECMATERIAL_KEYUSAGE_DESC:
            ret = tn_mem_free(&macKeyUsageMem, item);
            break;
        case MAC_SECMATERIAL_SECLEVEL_DESC:
            ret = tn_mem_free(&macSecLvlMem, item);
            break;
        case MAC_SECMATERIAL_KEY_DESC:
            ret = tn_mem_free(&macKeyMem, item);
            break;
        case MAC_SECMATERIAL_KEYDEV_DESC:
            ret = tn_mem_free(&macKeyDevMem, item);
            break;
        default:
            break;
    }

    return ret;
}

/**
 *  @brief PIB default values
 */
static const mac_pib_t macPibDefaults =
{
    0x36,                                         
    FALSE,                                      
    FALSE,                                      
    FALSE,                                      
    0x06,                                          

    NULL,                                       
    0x00,                                          
    MAC_BO_NON_BEACON,                          
    0x00,                                          
    0x00,                                          

    {0x00, ADDR_MODE_EXT},                         
    MAC_SHORT_ADDR_NONE,                        
    0x00,                                          
    FALSE,                                      
    0x04,                                          

    0x05,                                          
    0xFFFF,                                     
    FALSE,                                      
    TRUE,                                       
    MAC_SHORT_ADDR_NONE,                        

    MAC_SO_NONE,                                
    0x01F4,                                     
    FALSE,                                      
    0x08,                                          
    1220,                                       

    0x03,                                          
    0x20,                                         
    0x00,                                          
    TRUE,                                       
    FALSE,                                      

    /* security related */
    NULL,
    0x00,
    NULL,
    0x00,
    NULL,
    0x00,                                          
    0x00,                                          
    SECURITY_LEVEL_ENC_MIC_32,                                          
    KEY_ID_MODE_KEY_INDEX,
    {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},  
    0x01,                                       
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff},  
    {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},  
    0,                                          

    /* Proprietary */
    PHY_TX_POWER_MAX,                           
    MAC_CHAN_11,                                
    {0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb},  
    1,                                          
    MAC_BO_NON_BEACON,                          
};



static const mac_pibTbl_t macPibTbl[] =
{
    {OFFSETOF(mac_pib_t, ackWaitDuration), sizeof(u8), 0x36, 0x36},                         
    {OFFSETOF(mac_pib_t, associationPermit), sizeof(bool), FALSE, TRUE},               
    {OFFSETOF(mac_pib_t, autoRequest), sizeof(bool), FALSE, TRUE},                     
    {OFFSETOF(mac_pib_t, battLifeExt), sizeof(bool), FALSE, TRUE},                     
    {OFFSETOF(mac_pib_t, battLifeExtPeriods), sizeof(u8), 0x06, 0x06},                       

    {OFFSETOF(mac_pib_t, pBeaconPayload), sizeof(u8 *), 0, 0},                         
    {OFFSETOF(mac_pib_t, beaconPayloadLength), sizeof(u8), 0, 0x34},                     
    {OFFSETOF(mac_pib_t, beaconOrder), sizeof(u8), 0, 0x0f},                             
    {OFFSETOF(mac_pib_t, beaconTxTime), sizeof(u32), 0x01, 0x01},                            
    {OFFSETOF(mac_pib_t, bsn), sizeof(u8), 0x00, 0xFF},                                

    {OFFSETOF(mac_pib_t, coordExtendedAddress.addr.extAddr), sizeof(addrExt_t), 0, 0},
    {OFFSETOF(mac_pib_t, coordShortAddress), sizeof(u16), 0, 0},                       
    {OFFSETOF(mac_pib_t, dsn), sizeof(u8), 0x00, 0xFF},                                
    {OFFSETOF(mac_pib_t, gtsPermit), sizeof(bool), FALSE, TRUE},                       

 
    {OFFSETOF(mac_pib_t, maxCsmaBackoffs), sizeof(u8), 0, 0xfe},                        

    {OFFSETOF(mac_pib_t, minBe), sizeof(u8), 0, 8},                                    
    {OFFSETOF(mac_pib_t, panId), sizeof(u16), 0, 0},                                   
    {OFFSETOF(mac_pib_t, promiscuousMode), sizeof(bool), FALSE, TRUE},                 
    {OFFSETOF(mac_pib_t, rxOnWhenIdle), sizeof(bool), FALSE, TRUE},                    
    {OFFSETOF(mac_pib_t, shortAddress), sizeof(u16), 0, 0},                            

    {OFFSETOF(mac_pib_t, superframeOrder), sizeof(u8), 0, 0x0f},                         
    {OFFSETOF(mac_pib_t, transactionPersistenceTime), sizeof(u16), 0, 0},              
    {OFFSETOF(mac_pib_t, associatedPanCoord), sizeof(bool), FALSE, TRUE},              

  
    {OFFSETOF(mac_pib_t, maxBe), sizeof(u8), 0, 0x08},                                    
    {OFFSETOF(mac_pib_t, maxFrameTotalWaitTime), sizeof(u16), 0x00, 0xFF},             

    {OFFSETOF(mac_pib_t, maxFrameRetries), sizeof(u8), 0, 0x07},                          
    {OFFSETOF(mac_pib_t, responseWaitTime), sizeof(u8), 0x02, 0x40},                        
    {OFFSETOF(mac_pib_t, syncSymbolOffset), sizeof(u8), 0, 0},                         
    {OFFSETOF(mac_pib_t, timeStampSupported), sizeof(bool), FALSE, TRUE},              
    {OFFSETOF(mac_pib_t, securityEnabled), sizeof(bool), FALSE, TRUE},                 

    /* Security related PIBs*/
    {OFFSETOF(mac_pib_t, keyTable), sizeof(void *), 0, 0},
    {OFFSETOF(mac_pib_t, keyTableEntries), sizeof(u8), 0, MAC_KEY_TABLE_MAX_LEN},
    {OFFSETOF(mac_pib_t, devTable), sizeof(void *), 0, 0},
    {OFFSETOF(mac_pib_t, devTableEntries), sizeof(u8), 0, MAC_DEV_TABLE_MAX_LEN},
    {OFFSETOF(mac_pib_t, secLvlTable), sizeof(void *), 0, 0},
    {OFFSETOF(mac_pib_t, secLvlTableEntries), sizeof(u8), 0, MAC_SEC_LVL_TABLE_MAX_LEN},
    {OFFSETOF(mac_pib_t, frameCounter), sizeof(u32), 0, 0},
    {OFFSETOF(mac_pib_t, autoReqSecLvl), sizeof(u8), 0, 7},
    {OFFSETOF(mac_pib_t, autoReqKeyIdMode), sizeof(u8), 0, 3},
    {OFFSETOF(mac_pib_t, autoReqKeySource), 8, 0, 0},
    {OFFSETOF(mac_pib_t, autoReqKeyIndex), sizeof(u8), 1, 0xff},
    {OFFSETOF(mac_pib_t, defaultKeySource), 8, 0, 0},
    {OFFSETOF(mac_pib_t, panCoordExtAddr), 8, 0, 0},
    {OFFSETOF(mac_pib_t, panCoordShortAddr), sizeof(u16), 0, 0},

    /* Proprietary PIBs */
    {OFFSETOF(mac_pib_t, phyTransmitPower), sizeof(u8), 0, 0xFF},                      
    {OFFSETOF(mac_pib_t, logicalChannel), sizeof(u8), MAC_CHAN_11, MAC_CHAN_28},       
    {OFFSETOF(mac_pib_t, extendedAddress), sizeof(addrExt_t), 0, 0},                   
    {OFFSETOF(mac_pib_t, altBe), sizeof(u8), 0, 8},                                    
    {OFFSETOF(mac_pib_t, deviceBeaconOrder), sizeof(u8), 0, 15},                       
    {OFFSETOF(mac_pib_t, phyTransmitPower), sizeof(u8), 0, 0},                         
};

/* Invalid PIB table index used for error code */
#define MAC_PIB_INVALID     ((u8) (sizeof(macPibTbl) / sizeof(macPibTbl[0])))




/* MAC PIB */
static u8 beaconPld_buff[20];
mac_pib_t macPib;

static void mac_key_free(mac_keyDesc_t *key)
{
    //free key_id_lookup_list
    while (key->key_id_lookup_list_entries--) {
        tn_mem_free(&macKeyIDMem, tn_list_pop(key->key_id_lookup_list));
    }

    //free key_device_list
    while (key->key_device_list_entries--) {
        tn_mem_free(&macKeyDevMem, tn_list_pop(key->key_device_list));
    }

    //free key_usage_list
    while (key->key_usage_list_entries--) {
        tn_mem_free(&macKeyUsageMem, tn_list_pop(key->key_usage_list));
    }

    //free key
    tn_mem_free(&macKeyMem, (void *)key);
}

static u8 mac_pibIndex(u8 pibAttribute)
{
    if ((MAC_ATTR_SET1_START <= pibAttribute ) && (MAC_ATTR_SET1_END >= pibAttribute )) {
        return (pibAttribute - MAC_ATTR_SET1_START + MAC_ATTR_SET1_OFFSET);
    }
    else if ((MAC_ATTR_SET2_START <= pibAttribute ) && (MAC_ATTR_SET2_END >= pibAttribute )) {
        return (pibAttribute - MAC_ATTR_SET2_START + MAC_ATTR_SET2_OFFSET);
    }
    else if ((MAC_ATTR_SET3_START <= pibAttribute ) && (MAC_ATTR_SET3_END >= pibAttribute )) {
        return (pibAttribute - MAC_ATTR_SET3_START + MAC_ATTR_SET3_OFFSET);
    }
    else {
        return MAC_PIB_INVALID;
    }
}


/*********************************************************************
 * @fn      mac_pibReset
 *
 * @brief   Reset the MAC PIB to default value
 *
 * @param   None
 *
 * @return  None
 */	
void mac_pibReset(void)
{
    /* copy PIB defaults */
    macPib = macPibDefaults;

    /* initialize random sequence numbers */
    macPib.dsn = (u8)Rand();
    macPib.bsn = (u8)Rand();

    /* initialize security related PIBs */
    tn_mem_init(&macKeyIDMem);
    tn_mem_init(&macDeviceMem);
    tn_mem_init(&macKeyUsageMem);
    tn_mem_init(&macSecLvlMem);
    tn_mem_init(&macKeyMem);
    tn_mem_init(&macKeyDevMem);
    tn_list_init(macKeyTable);
    tn_list_init(macDevTable);
    tn_list_init(macSecLvlTable);
    macPib.keyTable = (void *)macKeyTable; 
    macPib.devTable = (void *)macDevTable;
    macPib.secLvlTable = (void *)macSecLvlTable;
    
    const u8 invalidIEEEAddr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    u8 extAddr[8];
	
    //read IEEE extended address from flash
    FLASH_PageRead(MAC_IEEE_ADDRESS_IN_FLASH, 8, extAddr);

    /* check the ieee address is valid or not */
    if (memcmp(invalidIEEEAddr, extAddr, 8) == 0) {
        memcpy(extAddr+4, startIEEEAddr, 4);
        u8 randomAddr[4];
        generateRandomData(randomAddr, 4);
        randomAddr[3] &= 0x0f;
        randomAddr[3] |= 0x80;
        memcpy(extAddr, randomAddr, 4);
    }
    mac_mlmeSetReq(MAC_EXTENDED_ADDRESS, 0, extAddr);

    macPib.pBeaconPayload = beaconPld_buff;
}

/*********************************************************************
 * @fn      mac_mlmeSetReq
 *
 * @brief   Set an attribute value in the MAC PIB
 *
 * @param   pibAttribute - The attribute identifier
 * @param   pibIndex - The index of attributes that are tables
 * @param   pValue - Pointer to the attribute value
 *
 * @return  MAC Status
 */
mac_sts_t mac_mlmeSetReq(u8 pibAttribute, u8 pibIndex, void* pValue)
{
    u8 i, r;
    void *tmp_item = NULL;
    void *tmp_item2 = NULL;
    RF_StatusTypeDef curState;

    if (MAC_BEACON_PAYLOAD == pibAttribute) {
        memcpy(macPib.pBeaconPayload, pValue, macPib.beaconPayloadLength);
        return MAC_SUCCESS;
    }

    /* security related PIBs*/
    if (pibAttribute == MAC_KEY_TABLE) {
        if (pibIndex > macPib.keyTableEntries) {  //pibIndex must be within current number of KeyDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else if (pibIndex == macPib.keyTableEntries) { //pibIndex equals to the TableEntries indicating adding a new entry to the tail of current table
            tn_list_add((tn_list_t)macPib.keyTable, pValue);
            macPib.keyTableEntries++;
        }
        else { //set an exist entry to a new value
            tmp_item = tn_list_head((tn_list_t)macPib.keyTable); 
            while (pibIndex--) {
                tmp_item2 = tmp_item;
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.keyTable, tmp_item);
            if (tmp_item2) {
                tn_list_insert((tn_list_t)macPib.keyTable, tmp_item2, pValue);
            }
            else {
                tn_list_push((tn_list_t)macPib.keyTable, pValue);
            }
            mac_key_free((mac_keyDesc_t *)tmp_item);
        }
        return MAC_SUCCESS;
    }

    if (pibAttribute == MAC_DEVICE_TABLE) {
        if (pibIndex > macPib.devTableEntries) {  //pibIndex must be within current number of DeviceDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else if (pibIndex == macPib.devTableEntries) {
            tn_list_add((tn_list_t)macPib.devTable, pValue);
            macPib.devTableEntries++;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.devTable); 
            while (pibIndex--) {
                tmp_item2 = tmp_item;
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.devTable, tmp_item);
            if (tmp_item2) {
                tn_list_insert((tn_list_t)macPib.devTable, tmp_item2, pValue);
            }
            else {
                tn_list_push((tn_list_t)macPib.devTable, pValue);
            }
            tn_mem_free(&macDeviceMem, tmp_item);
        }
        return MAC_SUCCESS;
    }

    if (pibAttribute == MAC_SECURITY_LEVEL_TABLE) {
        if (pibIndex > macPib.secLvlTableEntries) {  //pibIndex must be within current number of SecurityLevelDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else if (pibIndex == macPib.secLvlTableEntries) {
            tn_list_add((tn_list_t)macPib.secLvlTable, pValue);
            macPib.secLvlTableEntries++;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.secLvlTable); 
            while (pibIndex--) {
                tmp_item2 = tmp_item;
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.secLvlTable, tmp_item);
            if (tmp_item2) {
                tn_list_insert((tn_list_t)macPib.secLvlTable, tmp_item2, pValue);
            }
            else {
                tn_list_push((tn_list_t)macPib.secLvlTable, pValue);
            }
            tn_mem_free(&macSecLvlMem, tmp_item);
        }
        return MAC_SUCCESS;
    }

    if (MAC_PIB_INVALID == (i = mac_pibIndex(pibAttribute)) ) {
        return MAC_UNSUPPORTED_ATTRIBUTE;
    }

    if ( (macPibTbl[i].max != 0) || (macPibTbl[i].min != 0))
    {
        if (macPibTbl[i].max == macPibTbl[i].min)
        {
            //return MAC_READ_ONLY;
        }

        /* check for special cases */
        if (MAC_MAX_FRAME_TOTAL_WAIT_TIME == pibAttribute)
        {
            if ((*((u16 *) pValue) < MAC_MAX_FRAME_RESPONSE_MIN) ||
                (*((u16 *) pValue) > MAC_MAX_FRAME_RESPONSE_MAX))
            {
                return MAC_INVALID_PARAMETER;
            }
        }

        /* range check for general case */
        if ((*((u8 *) pValue) > macPibTbl[i].max) || (*((u8 *) pValue) < macPibTbl[i].min))
        {
            return MAC_INVALID_PARAMETER;
        }

    }

    r = IRQ_Disable();
    my_memcpy((u8 *) &macPib + macPibTbl[i].offset, pValue, macPibTbl[i].len);
    IRQ_Restore(r);

    switch (pibAttribute) {
    case MAC_PAN_ID:
        rf_802154_set(RF_ID_MAC_PANID, (u8*)&macPib.panId, 2);
        break;

    case MAC_SHORT_ADDRESS:
        rf_802154_set(RF_ID_MAC_SHORT_ADDR, (u8*)&macPib.shortAddress, 2);
        break;

    case MAC_RX_ON_WHEN_IDLE:
        if (*(u8 *)pValue) {
            RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        } 
        else {
            RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        }        
        break;

    case MAC_LOGICAL_CHANNEL:
        curState = RF_TrxStateGet();
        RF_TrxStateSet(curState, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));        break;

    case MAC_EXTENDED_ADDRESS:
        /* set ext address in radio */
        rf_802154_set(RF_ID_MAC_EXT_ADDR, (u8*)&macPib.extendedAddress, 8);
        break;

    case MAC_PHY_TRANSMIT_POWER:
        /* Legacy transmit power attribute */
//        macPib.phyTransmitPower = (u8)(-(s8)macPib.phyTransmitPower);
    	 RF_PowerLevelSet(macPib.phyTransmitPower);

//    case MAC_PHY_TRANSMIT_POWER_SIGNED:
//        /* Set the transmit power */
//        RF_PowerLevelSet(macPib.phyTransmitPower);
        break;

    default:
        break;
    }

    return MAC_SUCCESS;
}

/*********************************************************************
 * @fn      mac_mlmeGetReq
 *
 * @brief   Get an attribute value in the MAC PIB
 *
 * @param   pibAttribute - The attribute identifier
 * @param   pibIndex - The index of attributes that are tables
 * @param   pValue - Pointer to the attribute value
 *
 * @return  MAC Status
 */
mac_sts_t mac_mlmeGetReq(u8 pibAttribute, u8 pibIndex, void *pValue)
{
    u8 i, r;
    void *tmp_item = NULL;

    if (pibAttribute == MAC_BEACON_PAYLOAD) {
        memcpy(pValue, macPib.pBeaconPayload, macPib.beaconPayloadLength);
        return MAC_SUCCESS;
    }

    /* security related PIBs*/
    if (pibAttribute == MAC_KEY_TABLE) {
        if (pibIndex >= macPib.keyTableEntries) {  //pibIndex must be within current number of KeyDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.keyTable); 
            while (pibIndex--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            *((mac_keyDesc_t **)pValue) = (mac_keyDesc_t *)tmp_item;
            return MAC_SUCCESS;
        }
    }

    if (pibAttribute == MAC_DEVICE_TABLE) {
        if (pibIndex >= macPib.devTableEntries) {  //index must be within current number of DeviceDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.devTable); 
            while (pibIndex--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            *((mac_deviceDesc_t **)pValue) = (mac_deviceDesc_t *)tmp_item;
            return MAC_SUCCESS;
        }
    }

    if (pibAttribute == MAC_SECURITY_LEVEL_TABLE) {
        if (pibIndex >= macPib.secLvlTableEntries) {  //index must be within current number of SecurityLevelDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.secLvlTable); 
            while (pibIndex--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            *((mac_seclevelDesc_t **)pValue) = (mac_seclevelDesc_t *)tmp_item;
            return MAC_SUCCESS;
        }
    }

    if ((i = mac_pibIndex(pibAttribute)) == MAC_PIB_INVALID)
    {
        return MAC_UNSUPPORTED_ATTRIBUTE;
    }

    r = IRQ_Disable();
    memcpy(pValue, (u8 *) &macPib + macPibTbl[i].offset, macPibTbl[i].len);
    IRQ_Restore(r);

    return MAC_SUCCESS;
}

mac_sts_t mac_mlmeDelReq(u8 pibAttribute, u8 pibIndex)
{
    u8 i, r;
    void *tmp_item = NULL;

    /* security related PIBs*/
    if (pibAttribute == MAC_KEY_TABLE) {
        if (pibIndex >= macPib.keyTableEntries) {  //pibIndex must be within current number of KeyDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else { //delete an exist entry from a new value
            tmp_item = tn_list_head((tn_list_t)macPib.keyTable); 
            while (pibIndex--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.keyTable, tmp_item);
            macPib.keyTableEntries--;
            mac_key_free((mac_keyDesc_t *)tmp_item);
        }
        return MAC_SUCCESS;
    }

    if (pibAttribute == MAC_DEVICE_TABLE) {
        if (pibIndex >= macPib.devTableEntries) {  //pibIndex must be within current number of DeviceDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.devTable); 
            while (pibIndex--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.devTable, tmp_item);
            macPib.devTableEntries--;
            tn_mem_free(&macDeviceMem, tmp_item);
        }
        return MAC_SUCCESS;
    }

    if (pibAttribute == MAC_SECURITY_LEVEL_TABLE) {
        if (pibIndex >= macPib.secLvlTableEntries) {  //pibIndex must be within current number of SecurityLevelDescriptor
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.secLvlTable); 
            while (pibIndex--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.secLvlTable, tmp_item);
            macPib.secLvlTableEntries--;
            tn_mem_free(&macSecLvlMem, tmp_item);
        }
        return MAC_SUCCESS;
    }

    return MAC_INVALID_PARAMETER;
}

mac_sts_t mac_mlmeSetReqWithCnf(mac_setReq_t *pSetReq)
{
    u8 i, r;
    u8 *pData = NULL;
    u8 pibAttribute;
    u8 index;
    void* pValue;
    void *tmp_item = NULL;
    void *tmp_item2 = NULL;
    RF_StatusTypeDef curState;

    pibAttribute = pSetReq->pibAttribute;
    index = pSetReq->pibAttributeIndex;
    pValue = (void*)&(pSetReq->pibAttributeValue);
    u8 *pValue2 = (u8 *)&(pSetReq->pibAttributeValue);

    mac_setCnf_t *mac_setCnf = (mac_setCnf_t *)pSetReq;

    if (MAC_BEACON_PAYLOAD == pibAttribute) {
        memcpy(macPib.pBeaconPayload, pValue, macPib.beaconPayloadLength);
        mac_setCnf->pibAttribute = pibAttribute;
        mac_setCnf->pibAttributeIndex = index;
        mac_setCnf->primitive = MAC_MLME_SET_CNF;
        mac_setCnf->status=MAC_SUCCESS;
        mac_sendConfirm((u8*)mac_setCnf);
        return MAC_SUCCESS;
    }

    /* security related PIBs*/
    if (pibAttribute == MAC_KEY_TABLE) {
        if (index > macPib.keyTableEntries) {  //index must be within current number of KeyDescriptor
            mac_setCnf->pibAttribute = pibAttribute;
            mac_setCnf->pibAttributeIndex = index;
            mac_setCnf->primitive = MAC_MLME_SET_CNF;
            mac_setCnf->status = TL_MAC_INVALID_PARAMETER;
            mac_sendConfirm((u8*)mac_setCnf);
            return MAC_INVALID_PARAMETER;
        }
        else if (index == macPib.keyTableEntries) { //index equals to the TableEntries indicating adding a new entry to the tail of current table
            tn_list_add((tn_list_t)macPib.keyTable, (void *)(pSetReq->pibAttributeValue));
            macPib.keyTableEntries++;
        }
        else { //set an exist entry to a new value
            tmp_item = tn_list_head((tn_list_t)macPib.keyTable); 
            while (index--) {
                tmp_item2 = tmp_item;
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.keyTable, tmp_item);
            if (tmp_item2) {
                tn_list_insert((tn_list_t)macPib.keyTable, tmp_item2, (void *)(pSetReq->pibAttributeValue));
            }
            else {
                tn_list_push((tn_list_t)macPib.keyTable, (void *)(pSetReq->pibAttributeValue));
            }
            mac_key_free((mac_keyDesc_t *)tmp_item);
        }
        mac_setCnf->pibAttribute = pibAttribute;
        mac_setCnf->pibAttributeIndex = index;
        mac_setCnf->primitive = MAC_MLME_SET_CNF;
        mac_setCnf->status = MAC_SUCCESS;
        mac_sendConfirm((u8*)mac_setCnf);
        return MAC_SUCCESS;
    }

    if (pibAttribute == MAC_DEVICE_TABLE) {
        if (index > macPib.devTableEntries) {  //index must be within current number of DeviceDescriptor
            mac_setCnf->pibAttribute = pibAttribute;
            mac_setCnf->pibAttributeIndex = index;
            mac_setCnf->primitive = MAC_MLME_SET_CNF;
            mac_setCnf->status = MAC_INVALID_PARAMETER;
            mac_sendConfirm((u8*)mac_setCnf);
            return MAC_INVALID_PARAMETER;
        }
        else if (index == macPib.devTableEntries) {
            tn_list_add((tn_list_t)macPib.devTable, (void *)(pSetReq->pibAttributeValue));
            macPib.devTableEntries++;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.devTable); 
            while (index--) {
                tmp_item2 = tmp_item;
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.devTable, tmp_item);
            if (tmp_item2) {
                tn_list_insert((tn_list_t)macPib.devTable, tmp_item2, (void *)(pSetReq->pibAttributeValue));
            }
            else {
                tn_list_push((tn_list_t)macPib.devTable, (void *)(pSetReq->pibAttributeValue));
            }
            tn_mem_free(&macDeviceMem, tmp_item);
        }
        mac_setCnf->pibAttribute = pibAttribute;
        mac_setCnf->pibAttributeIndex = index;
        mac_setCnf->primitive = MAC_MLME_SET_CNF;
        mac_setCnf->status = MAC_SUCCESS;
        mac_sendConfirm((u8*)mac_setCnf);
        return MAC_SUCCESS;
    }

    if (pibAttribute == MAC_SECURITY_LEVEL_TABLE) {
        if (index > macPib.secLvlTableEntries) {  //index must be within current number of SecurityLevelDescriptor
            mac_setCnf->pibAttribute = pibAttribute;
            mac_setCnf->pibAttributeIndex = index;
            mac_setCnf->primitive = MAC_MLME_SET_CNF;
            mac_setCnf->status = MAC_INVALID_PARAMETER;
            mac_sendConfirm((u8*)mac_setCnf);
            return MAC_INVALID_PARAMETER;
        }
        else if (index == macPib.secLvlTableEntries) {
            tn_list_add((tn_list_t)macPib.secLvlTable, (void *)(pSetReq->pibAttributeValue));
            macPib.secLvlTableEntries++;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.secLvlTable); 
            while (index--) {
                tmp_item2 = tmp_item;
                tmp_item = tn_list_item_next(tmp_item);
            }
            tn_list_remove((tn_list_t)macPib.secLvlTable, tmp_item);
            if (tmp_item2) {
                tn_list_insert((tn_list_t)macPib.secLvlTable, tmp_item2, (void *)(pSetReq->pibAttributeValue));
            }
            else {
                tn_list_push((tn_list_t)macPib.secLvlTable, (void *)(pSetReq->pibAttributeValue));
            }
            tn_mem_free(&macSecLvlMem, tmp_item);
        }
        mac_setCnf->pibAttribute = pibAttribute;
        mac_setCnf->pibAttributeIndex = index;
        mac_setCnf->primitive = MAC_MLME_SET_CNF;
        mac_setCnf->status = MAC_SUCCESS;
        mac_sendConfirm((u8*)mac_setCnf);
        return MAC_SUCCESS;
    }

    /* look up attribute in PIB table */
    if (MAC_PIB_INVALID == (i = mac_pibIndex(pibAttribute))) {
        mac_setCnf->pibAttribute = pibAttribute;
        mac_setCnf->pibAttributeIndex = index;
        mac_setCnf->primitive = MAC_MLME_SET_CNF;
        mac_setCnf->status = MAC_UNSUPPORTED_ATTRIBUTE;
        return MAC_UNSUPPORTED_ATTRIBUTE;
    }

    /* do range check; no range check if min and max are zero */
    if ((macPibTbl[i].max != 0) || (macPibTbl[i].min != 0))
    {
        /* if min == max, this is a read-only attribute */
        if (macPibTbl[i].max == macPibTbl[i].min)
        {
            //return MAC_READ_ONLY;
        }

        /* check for special cases */
        if (MAC_MAX_FRAME_TOTAL_WAIT_TIME == pibAttribute)
        {
            if ((*((u16 *) pValue) < MAC_MAX_FRAME_RESPONSE_MIN) ||
                (*((u16 *) pValue) > MAC_MAX_FRAME_RESPONSE_MAX))
            {
                mac_setCnf->pibAttribute = pibAttribute;
                mac_setCnf->pibAttributeIndex = index;
                mac_setCnf->primitive = MAC_MLME_SET_CNF;
                mac_setCnf->status = MAC_INVALID_PARAMETER;
                return MAC_INVALID_PARAMETER;
            }
        }

        /* range check for general case */
        if ((*((u8 *) pValue) > macPibTbl[i].max) || (*((u8 *) pValue) < macPibTbl[i].min))
        {
            mac_setCnf->pibAttribute = pibAttribute;
            mac_setCnf->pibAttributeIndex = index;
            mac_setCnf->primitive = MAC_MLME_SET_CNF;
            mac_setCnf->status = MAC_INVALID_PARAMETER;
            return MAC_INVALID_PARAMETER;
        }

    }

    /* set value in PIB */
    r = IRQ_Disable();
    memcpy((u8 *) &macPib + macPibTbl[i].offset, pValue, macPibTbl[i].len);
    IRQ_Restore(r);
  
    switch (pibAttribute) {
    case MAC_PAN_ID:
        rf_802154_set(RF_ID_MAC_PANID, (u8*)&macPib.panId, 2);
        break;

    case MAC_SHORT_ADDRESS:
        rf_802154_set(RF_ID_MAC_SHORT_ADDR, (u8*)&macPib.shortAddress, 2);
        break;

    case MAC_RX_ON_WHEN_IDLE:
        if (*(u8 *)pValue) {
            RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        } 
        else {
            RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        }
        break;

    case MAC_LOGICAL_CHANNEL:
        curState = RF_TrxStateGet();
        RF_TrxStateSet(curState, RF_CHANNEL_TO_FREQUENCY(macPib.logicalChannel));
        break;

    case MAC_EXTENDED_ADDRESS:
        /* set ext address in radio */
        rf_802154_set(RF_ID_MAC_EXT_ADDR, (u8*)&macPib.extendedAddress, 8);
        break;

    case MAC_PHY_TRANSMIT_POWER:
        /* Legacy transmit power attribute */
//        macPib.phyTransmitPower = (u8)(-(s8)macPib.phyTransmitPower);
    	RF_PowerLevelSet(macPib.phyTransmitPower);

//    case MAC_PHY_TRANSMIT_POWER_SIGNED:
//        /* Set the transmit power */
//        RF_PowerLevelSet(macPib.phyTransmitPower);
        break;

    default:
        break;
    }

    mac_setCnf->pibAttribute = pibAttribute;
    mac_setCnf->pibAttributeIndex = index;
    mac_setCnf->primitive = MAC_MLME_SET_CNF;
    mac_setCnf->status = MAC_SUCCESS;
    mac_sendConfirm((u8*)mac_setCnf);
    return MAC_SUCCESS;
}


mac_sts_t mac_mlmeGetReqWithCnf(mac_getReq_t *pGetReq)
{
    u8 i, j, r;
    u8 pibAttribute;
    u8 index;
    void* pValue;
    void *tmp_item = NULL;

    pibAttribute = pGetReq->pibAttribute;
    index = pGetReq->pibAttributeIndex;
    mac_getCnf_t *mac_getCnf = (mac_getCnf_t *)pGetReq;
    mac_getCnf->pibAttribute = pibAttribute;
    mac_getCnf->pibAttributeIndex = index;
    mac_getCnf->primitive = MAC_MLME_GET_CNF;
    pValue = (void*)&(mac_getCnf->pibAttributeValue);
    
    if (pibAttribute == MAC_BEACON_PAYLOAD) {
        memcpy(pValue, macPib.pBeaconPayload, macPib.beaconPayloadLength);
        mac_getCnf->status = MAC_SUCCESS;
        mac_sendConfirm((u8*)mac_getCnf);
        return MAC_SUCCESS;
    }

    /* security related PIBs*/
    if (pibAttribute == MAC_KEY_TABLE) {
        if (index >= macPib.keyTableEntries) {  //index must be within current number of KeyDescriptor
            mac_getCnf->status = MAC_INVALID_PARAMETER;
            mac_sendConfirm((u8*)mac_getCnf);
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.keyTable); 
            while (index--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            *((mac_keyDesc_t **)pValue) = (mac_keyDesc_t *)tmp_item;
            mac_getCnf->status = MAC_SUCCESS;
            mac_sendConfirm((u8*)mac_getCnf);
            return MAC_SUCCESS;
        }
    }

    if (pibAttribute == MAC_DEVICE_TABLE) {
        if (index >= macPib.devTableEntries) {  //index must be within current number of DeviceDescriptor
            mac_getCnf->status = MAC_INVALID_PARAMETER;
            mac_sendConfirm((u8*)mac_getCnf);
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.devTable); 
            while (index--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            *((mac_deviceDesc_t **)pValue) = (mac_deviceDesc_t *)tmp_item;
            mac_getCnf->status = MAC_SUCCESS;
            mac_sendConfirm((u8*)mac_getCnf);
            return MAC_SUCCESS;
        }
    }

    if (pibAttribute == MAC_SECURITY_LEVEL_TABLE) {
        if (index >= macPib.secLvlTableEntries) {  //index must be within current number of SecurityLevelDescriptor
            mac_getCnf->status = MAC_INVALID_PARAMETER;
            mac_sendConfirm((u8*)mac_getCnf);
            return MAC_INVALID_PARAMETER;
        }
        else {
            tmp_item = tn_list_head((tn_list_t)macPib.secLvlTable); 
            while (index--) {
                tmp_item = tn_list_item_next(tmp_item);
            }
            *((mac_seclevelDesc_t **)pValue) = (mac_seclevelDesc_t *)tmp_item;
            mac_getCnf->status = MAC_SUCCESS;
            mac_sendConfirm((u8*)mac_getCnf);
            return MAC_SUCCESS;
        }
    }
    
    if ((i = mac_pibIndex(pibAttribute)) == MAC_PIB_INVALID) {
        return MAC_UNSUPPORTED_ATTRIBUTE;
    }

    r = IRQ_Disable();
    my_memcpy(pValue, (u8*)&macPib + macPibTbl[i].offset, macPibTbl[i].len);
    IRQ_Restore(r);
    mac_getCnf->status = MAC_SUCCESS;
    mac_sendConfirm((u8*)mac_getCnf);
    return MAC_SUCCESS;
}
