/**************************************************************************************
  Filename:       mac_security.c
  Revised:        $Date: 2015-10-15 $

  Description:    Implementation file for the MAC security module
**************************************************************************************/

#include "mac_include.h"

typedef enum {
    MAC_SECURITY_FAILED = -1,
    MAC_SECURITY_PASSED = 0,
    MAC_SECURITY_CONDITIONALLY_PASSED = 1,
} mac_secStatus_t;

mac_secStatus_t mac_getOutgoingFrameKey(mac_genericFrame_t* pMacFrame, mac_sec_t* pSecurity, u8** ppKey);
mac_secStatus_t mac_getIncomingFrameSecMaterial(mac_genericFrame_t* pMacFrame, 
                                                u8* pFrameCtrl,
                                                u8* pAuxSecHDR,
                                                mac_keyDesc_t** ppKeyDesc, 
                                                mac_deviceDesc_t** ppDeviceDesc, 
                                                mac_keydevDesc_t** ppKeyDevDesc);
mac_secStatus_t mac_lookupKeyDesc(mac_keyid_lookup_desc_t* pKeyIdLookupDesc, mac_keyDesc_t** ppKeyDesc);
mac_secStatus_t mac_checkBlackList(mac_keyDesc_t* pKeyDesc, mac_keyid_lookup_desc_t* pKeyIdLookupDesc, 
                                   mac_deviceDesc_t** ppDeviceDesc, mac_keydevDesc_t** ppKeyDevDesc);
mac_secStatus_t mac_lookupDeviceDesc(mac_deviceDesc_t* pDeviceDesc, mac_keyid_lookup_desc_t* pKeyIdLookupDesc);
mac_secStatus_t mac_checkIncomingSecLevel(mac_secLevel_t security_level, u8 frame_type, u8 cmd_frame_id);
mac_secStatus_t mac_checkIncomingKeyUsagePolicy(mac_keyDesc_t* pKeyDesc, u8 frame_type, u8 cmd_frame_id);
mac_secStatus_t mac_getLookupDesc(mac_genericFrame_t* pMacFrame, 
                                  mac_sec_t* pSecurity, 
                                  u8* pFrameCtrl, 
                                  u8 is_tx, 
                                  mac_keyid_lookup_desc_t* pKeyIdLookupDesc);
mac_secStatus_t mac_secLvlCmp(mac_secLevel_t sec1, mac_secLevel_t sec2);
u8 mac_getFramePayloadPos(mac_genericFrame_t* pMacFrame);

u8* my_memcpy(u8* dest, const u8* src, unsigned int count)
{
    u8* pDest = dest;
    const u8* pSrc = src;

    if ((pDest>pSrc) && (pDest<(pSrc+count))) {
        pDest = pDest + count-1;
        pSrc = pSrc + count-1;
        while (count--){
            *pDest-- = *pSrc--;
        }
    }
    else {
        while(count--){
            *pDest++ = *pSrc++;
        }
    }

    return pDest;
}
/*********************************************************************
 * @fn      mac_getOutgoingFrameKey
 *
 * @brief   Outgoing frame key retrieval procedure(see IEEE 802.15.4-2006 p. 200 )
 *
 * @param   pMacFrame - input the frame to be secured
 *
 * @param   pSecurity - input the Security Information with which to Secure the Frame
 *
 * @param   ppKey - output the key's address if it has been obtained successfully 
 *
 * @return  status (indicating status of retrieving the key, passed of failed)
 */
mac_secStatus_t mac_getOutgoingFrameKey(mac_genericFrame_t* pMacFrame, mac_sec_t* pSecurity, u8** ppKey)
{
    mac_keyid_lookup_desc_t keyid_lookup_desc;
    mac_keyDesc_t* pKeyDesc;

    u8* pFrameCtrl = (u8*)((u32)pMacFrame + mac_tx_getFrameControlPos(pMacFrame));

    /***determine the lookup data and lookup size***/
    if (MAC_SECURITY_PASSED != mac_getLookupDesc(pMacFrame, pSecurity, pFrameCtrl, 1, &keyid_lookup_desc)) {
        return MAC_SECURITY_FAILED;
    }
    
    /***obtain the KeyDescriptor by passing the key lookup data and lookup size to the KeyDescriptor lookup procedure***/
    if (MAC_SECURITY_PASSED == mac_lookupKeyDesc(&keyid_lookup_desc, &pKeyDesc)) {
        *ppKey = pKeyDesc->key;
        return MAC_SECURITY_PASSED;
    }
    else {
        return MAC_SECURITY_FAILED;
    }
}

 /*********************************************************************
 * @fn      mac_lookupKeyDesc
 *
 * @brief   The inputs to this function are the key lookup data and 
 *          lookup size. The outputs from this function are a passed or
 *          failed status and, if passed, a KeyDescriptor.
 *          (see IEEE 802.15.4-2006 p. 204 )
 *
 * @param   pKeyIdLookupDesc - input the key lookup data and lookup size
 *
 * @param   ppKeyDesc - output the obtained KeyDescriptor's address if passed
 *
 * @return  status (indicating status of obtaining the KeyDescriptor,
 *          passed of failed)
 */
mac_secStatus_t mac_lookupKeyDesc(mac_keyid_lookup_desc_t* pKeyIdLookupDesc, mac_keyDesc_t** ppKeyDesc)
{
    mac_keyDesc_t* pKeyDesc;
    mac_keyid_lookup_desc_t* pKeyIDDesc;

    pKeyDesc = (mac_keyDesc_t *)tn_list_head((tn_list_t)macPib.keyTable);
    for (; pKeyDesc; pKeyDesc = (mac_keyDesc_t *)tn_list_item_next((void *)pKeyDesc)) {
        pKeyIDDesc = (mac_keyid_lookup_desc_t *)tn_list_head(pKeyDesc->key_id_lookup_list);
        for (; pKeyIDDesc; pKeyIDDesc = (mac_keyid_lookup_desc_t *)tn_list_item_next((void *)pKeyIDDesc)) {
            if (pKeyIDDesc->lookup_data_size == pKeyIdLookupDesc->lookup_data_size) {
                if (memcmp(pKeyIDDesc->lookup_data, pKeyIdLookupDesc->lookup_data, pKeyIDDesc->lookup_data_size ? 9 : 5) == 0) {
                    *ppKeyDesc = pKeyDesc;
                    return MAC_SECURITY_PASSED;
                }
            }
        }
    }
    return MAC_SECURITY_FAILED;

#if 0
    pKeyDesc = macPib.keyTable;
    for (i=0; i<macPib.keyTableEntries; i++) {
        pKeyIDLookupDesc = pKeyDesc[i].key_id_lookup_list;
        for (j=0; j<pKeyDesc[i].key_id_lookup_list_entries; j++) {
            if (pKeyIDLookupDesc[j].lookup_data_size == pKeyIdLookupDesc->lookup_data_size) {
                if (memcmp(pKeyIDLookupDesc[j].lookup_data, pKeyIdLookupDesc->lookup_data, (pKeyIDLookupDesc[j].lookup_data_size?9:5)) == 0) {
                   *ppKeyDesc = &pKeyDesc[i];
                    return MAC_SECURITY_PASSED;
                }
            }
        }
    }
    return MAC_SECURITY_FAILED;
#endif

}

/*********************************************************************
 * @fn      mac_getIncomingFrameSecMaterial
 *
 * @brief   The input to this procedure is the frame to be unsecured. 
 *          The outputs from this procedure are a passed or failed status
 *          and, if passed, a KeyDescriptor, a DeviceDescriptor, and a 
 *          KeyDeviceDescriptor.(see IEEE 802.15.4-2006 p. 203 )
 *
 * @param   pMacFrame - input the frame to be unsecured
 *
 * @param   ppKeyDesc - output the Key Descriptor has been retrieved
 *
 * @param   ppDeviceDesc - output the Device Descriptor has been retrieved 
 *
 * @param   ppKeyDevDesc - output the Key-Dev Descriptor has been retrieved 
 *
 * @return  status (indicating status of retrieving the Material, passed of failed)
 */
mac_secStatus_t mac_getIncomingFrameSecMaterial(mac_genericFrame_t* pMacFrame, 
                                                u8* pFrameCtrl,
                                                u8* pAuxSecHDR,
                                                mac_keyDesc_t** ppKeyDesc, 
                                                mac_deviceDesc_t** ppDeviceDesc, 
                                                mac_keydevDesc_t** ppKeyDevDesc)
{
    mac_keyid_lookup_desc_t keyid_lookup_desc;
    mac_sec_t SecurityInfo; 

    memset(&keyid_lookup_desc, 0, sizeof(keyid_lookup_desc));
    memset(&SecurityInfo, 0, sizeof(SecurityInfo));
    SecurityInfo.securityLevel = pAuxSecHDR[0] & SEC_CRTL_SEC_LEVEL_MASK;
    SecurityInfo.key_id_mode = (pAuxSecHDR[0]&SEC_CRTL_KEYID_MODE_MASK)>>SEC_CRTL_KEYID_MODE_POS;

    if (KEY_ID_MODE_KEY_INDEX == SecurityInfo.key_id_mode) {
        SecurityInfo.key_index = pAuxSecHDR[5];
    }
    else if (KEY_ID_MODE_KEY_EXPLICIT_4 == SecurityInfo.key_id_mode) {
        memcpy(SecurityInfo.key_source, &pAuxSecHDR[5], 4);
        SecurityInfo.key_index = pAuxSecHDR[9];
    }
    else if (KEY_ID_MODE_KEY_EXPLICIT_8 == SecurityInfo.key_id_mode) {
        memcpy(SecurityInfo.key_source, &pAuxSecHDR[5], 8);
        SecurityInfo.key_index = pAuxSecHDR[13];
    }

    /***determine the key lookup data and key lookup size***/
    if (MAC_SECURITY_PASSED != mac_getLookupDesc(pMacFrame, &SecurityInfo, pFrameCtrl, 0, &keyid_lookup_desc)) {
        return MAC_SECURITY_FAILED;
    }

    /***obtain the KeyDescriptor by passing the key lookup data and lookup size to the KeyDescriptor lookup procedure***/
    if (MAC_SECURITY_PASSED != mac_lookupKeyDesc(&keyid_lookup_desc, ppKeyDesc)) {
        return MAC_SECURITY_FAILED;
    }

    /***determine the device lookup data and device lookup size(reuse the function and data structure for key lookup data )***/
    SecurityInfo.key_id_mode = KEY_ID_MODE_IMPLICIT;
    memset(&keyid_lookup_desc, 0, sizeof(keyid_lookup_desc));
    if (MAC_SECURITY_PASSED != mac_getLookupDesc(pMacFrame, &SecurityInfo, pFrameCtrl, 0, &keyid_lookup_desc)) {
        return MAC_SECURITY_FAILED;
    }

    /***obtain DeviceDescriptor, KeyDeviceDescriptor by passing KeyDescriptor,device lookup data and device lookup size to blacklist checking procedure ***/
    if (MAC_SECURITY_PASSED != mac_checkBlackList(*ppKeyDesc, &keyid_lookup_desc, ppDeviceDesc, ppKeyDevDesc)) {
        return MAC_SECURITY_FAILED;
    }
    return MAC_SECURITY_PASSED;
}

/*********************************************************************
 * @fn      mac_getLookupDesc
 *
 * @brief   Determine the key lookup data and key look up size. 
 *          (see IEEE 802.15.4-2006 p. 200, 203 )
 *
 * @param   pMacFrame - input the frame to be secured/unsecured
 *
 * @param   pSecurity - input the security information according to 
 *          which to detemine the lookup data and key look up size.
 *
 * @param   frame_control - input the frame_control according to which
 *          to detemine the lookup data and key look up size.
 *
 * @param   is_tx - input the Tx/Rx mode. 1: Tx, 0: Rx  
 *
 * @param   pKeyIdLookupDesc - output the key lookup data and key look
 *          up size has been retrieved 
 *
 * @return  status (indicating status of retrieving the Material, passed of failed)
 */
mac_secStatus_t mac_getLookupDesc(mac_genericFrame_t* pMacFrame, 
                                  mac_sec_t* pSecurity, 
                                  u8* pFrameCtrl, 
                                  u8 is_tx, 
                                  mac_keyid_lookup_desc_t* pKeyIdLookupDesc)
{
    u16 frame_control = *(pFrameCtrl+1);
    frame_control <<= 8;
    frame_control |= *pFrameCtrl;
    u8 addr_mode = 0;

    /***determine the lookup data and lookup size***/
    if (KEY_ID_MODE_IMPLICIT == pSecurity->key_id_mode) {
        if (is_tx) {
            addr_mode = (frame_control&MAC_FCF_DST_ADDR_MODE_MASK)>>MAC_FCF_DST_ADDR_MODE_POS;
        }
        else {
            addr_mode = (frame_control&MAC_FCF_SRC_ADDR_MODE_MASK)>>MAC_FCF_SRC_ADDR_MODE_POS;
        }

        if (0 == addr_mode) {
            if (macPib.panCoordShortAddr < 0xfffe) {
                /*lookup_data = 2-oct PANID||2-oct macpanCoordShortAddr||1-oct 0x00*/
                pKeyIdLookupDesc->lookup_data_size = LOOKUP_DATA_SIZE_5;
                my_memcpy(pKeyIdLookupDesc->lookup_data, (u8 *)&macPib.panId, 2);
                my_memcpy(pKeyIdLookupDesc->lookup_data+2, (u8 *)&macPib.panCoordShortAddr, 2);
                pKeyIdLookupDesc->lookup_data[4] = 0x00;
            }
            else if (0xfffe == macPib.panCoordShortAddr) {
                /*lookup_data = 8-oct macPANCoordExtendedAddres||1-oct 0x00*/
                pKeyIdLookupDesc->lookup_data_size = LOOKUP_DATA_SIZE_9;
                my_memcpy(pKeyIdLookupDesc->lookup_data, macPib.panCoordExtAddr, 8);
                pKeyIdLookupDesc->lookup_data[8] = 0x00;
            }
            else {
                return MAC_SECURITY_FAILED;
            }
        }
        else if (0x02 == addr_mode) {
            /*lookup_data = 2-oct Dst/Src PANID||2-oct Dst/Src Address||1-oct 0x00 */
            pKeyIdLookupDesc->lookup_data_size = LOOKUP_DATA_SIZE_5;
            if (is_tx) {
                my_memcpy(pKeyIdLookupDesc->lookup_data, mac_getDstPANIDPos(pFrameCtrl), 2);
                my_memcpy(pKeyIdLookupDesc->lookup_data+2, mac_getDstAddrPos(pFrameCtrl), 2);
            }
            else {
                my_memcpy(pKeyIdLookupDesc->lookup_data, mac_getSrcPANIDPos(pFrameCtrl), 2);
                my_memcpy(pKeyIdLookupDesc->lookup_data+2, mac_getSrcAddrPos(pFrameCtrl), 2);
            }
            pKeyIdLookupDesc->lookup_data[4] = 0x00;
        }
        else if (0x03 == addr_mode) {
            /*lookup_data = 8-oct Dst/Src Address||1-oct 0x00 */
            pKeyIdLookupDesc->lookup_data_size = LOOKUP_DATA_SIZE_9;
            if (is_tx) {
                my_memcpy(pKeyIdLookupDesc->lookup_data, mac_getDstAddrPos(pFrameCtrl), 8);
            }
            else {
                my_memcpy(pKeyIdLookupDesc->lookup_data, mac_getSrcAddrPos(pFrameCtrl), 8);
            }
            pKeyIdLookupDesc->lookup_data[8] = 0x00;
        }
        else {
            return MAC_SECURITY_FAILED;
        }
    } 
    else if (KEY_ID_MODE_KEY_INDEX == pSecurity->key_id_mode) {
        /*lookup_data = 8-oct macDefaultKeySource||1-oct KeyIndex */
        pKeyIdLookupDesc->lookup_data_size = LOOKUP_DATA_SIZE_9;
        my_memcpy(pKeyIdLookupDesc->lookup_data, (u8 *)macPib.defaultKeySource, 8);
        pKeyIdLookupDesc->lookup_data[8] = pSecurity->key_index;
    }
    else if (KEY_ID_MODE_KEY_EXPLICIT_4 == pSecurity->key_id_mode) {
        /*lookup_data = 4-oct KeySource||1-oct KeyIndex */
        pKeyIdLookupDesc->lookup_data_size = LOOKUP_DATA_SIZE_5;
        my_memcpy(pKeyIdLookupDesc->lookup_data, pSecurity->key_source, 4);
        pKeyIdLookupDesc->lookup_data[4] = pSecurity->key_index;
    }
    else if (KEY_ID_MODE_KEY_EXPLICIT_8 == pSecurity->key_id_mode) {
        /*lookup_data = 8-oct KeySource||1-oct KeyIndex */
        pKeyIdLookupDesc->lookup_data_size = LOOKUP_DATA_SIZE_9;
        my_memcpy(pKeyIdLookupDesc->lookup_data, pSecurity->key_source, 8);
        pKeyIdLookupDesc->lookup_data[8] = pSecurity->key_index;
    }

    return MAC_SECURITY_PASSED;
}

/*********************************************************************
 * @fn      mac_checkBlackList
 *
 * @brief   The inputs to this procedure are the KeyDescriptor, the 
 *          device lookup data, and the device lookup size. The outputs 
 *          from this procedure are a passed or failed status and, if 
 *          passed, a DeviceDescriptor and aKeyDeviceDescriptor.(see 
 *          IEEE 802.15.4-2006 p. 204 )
 *
 * @param   pKeyDesc - input the KeyDescriptor used to do BlackList checking
 *
 * @param   pKeyIdLookupDesc - input the device lookup data and the device  
 *          lookup size
 *
 * @param   ppDeviceDesc - output the DeviceDescriptor.
 *
 * @param   ppKeyDevDesc - output the ppKeyDevDesc.  
 *
 * @return  status (indicating status of retrieving the Material, passed of failed)
 */
mac_secStatus_t mac_checkBlackList(mac_keyDesc_t* pKeyDesc, 
                                   mac_keyid_lookup_desc_t* pKeyIdLookupDesc, 
                                   mac_deviceDesc_t** ppDeviceDesc, 
                                   mac_keydevDesc_t** ppKeyDevDesc)
{
    mac_deviceDesc_t* pDeviceDesc;
    mac_keydevDesc_t* pKeyDevDesc =  (mac_keydevDesc_t *)tn_list_head(pKeyDesc->key_device_list);
    for (; pKeyDevDesc; pKeyDevDesc = (mac_keydevDesc_t *)tn_list_item_next(pKeyDevDesc)) {
        pDeviceDesc = pKeyDevDesc->device_descriptor;
        if (TRUE == pKeyDevDesc->blacklisted) {
            return MAC_SECURITY_FAILED;
        }
        else {
            if (TRUE == pKeyDevDesc->unique_device) {
                *ppDeviceDesc = pDeviceDesc;
                *ppKeyDevDesc = pKeyDevDesc;
                return MAC_SECURITY_PASSED;
            }
            else {
                if (MAC_SECURITY_PASSED == mac_lookupDeviceDesc(pDeviceDesc, pKeyIdLookupDesc)) {
                    *ppDeviceDesc = pDeviceDesc;
                    *ppKeyDevDesc = pKeyDevDesc;
                    return MAC_SECURITY_PASSED;
                }
            }
        }
    }

    return MAC_SECURITY_FAILED;
}

/*********************************************************************
 * @fn      mac_lookupDeviceDesc
 *
 * @brief   The inputs to this procedure are the DeviceDescriptor, the 
 *          device lookup data, and the device lookup size.The output 
 *          from this procedure is a passed or failed status.(see IEEE
 *          802.15.4-2006 p. 205 )
 *
 * @param   pDeviceDesc - input the DeviceDescriptor
 *
 * @param   pKeyIdLookupDesc - input the device lookup data and the device  
 *          lookup size
 *
 * @return  status (indicating the result of the DeviceDescriptor lookup)
 */
mac_secStatus_t mac_lookupDeviceDesc(mac_deviceDesc_t* pDeviceDesc, mac_keyid_lookup_desc_t* pKeyIdLookupDesc)
{
    u32 tmp = 0;

    if (LOOKUP_DATA_SIZE_5 == pKeyIdLookupDesc->lookup_data_size) {
        tmp = pDeviceDesc->short_address;
        tmp <<= 16;
        tmp |= pDeviceDesc->pan_id;
        if (0 == memcmp(pKeyIdLookupDesc->lookup_data, &tmp, 4)) {
            return MAC_SECURITY_PASSED;
        }
        else {
            return MAC_SECURITY_FAILED;
        }
    }
    else if (LOOKUP_DATA_SIZE_9 == pKeyIdLookupDesc->lookup_data_size) {
        if (0 == memcmp(pKeyIdLookupDesc->lookup_data, pDeviceDesc->long_address, 8)) {
            return MAC_SECURITY_PASSED;
        }
        else {
            return MAC_SECURITY_FAILED;
        }
    }
    else {
        return MAC_SECURITY_FAILED;
    }
}

/*********************************************************************
 * @fn      mac_checkIncomingSecLevel
 *
 * @brief   The inputs to this procedure are the incoming security level, 
 *          the frame type and the command frame identifier. The output 
 *          from this procedure is a passed, failed, or 鈥渃onditionally
 *          passed鈥� status.(see IEEE 802.15.4-2006 p. 205)
 *
 * @param   security_level - input the incoming security level
 *
 * @param   frame_type - input the incoming security level
 *
 * @param   cmd_frame_id - input the command frame identifier
 *
 * @return  status (indicating the result of the checking)
 */
mac_secStatus_t mac_checkIncomingSecLevel(mac_secLevel_t security_level, u8 frame_type, u8 cmd_frame_id)
{
    mac_seclevelDesc_t* pSecLvlDesc = (mac_seclevelDesc_t *)tn_list_head((tn_list_t)macPib.secLvlTable);

    if (0 == macPib.secLvlTableEntries) {
        return MAC_SECURITY_PASSED;
    }

    for (; pSecLvlDesc; pSecLvlDesc = (mac_seclevelDesc_t *)tn_list_item_next(pSecLvlDesc)) {
        if (0x03 != frame_type) {
            if (frame_type == pSecLvlDesc->frame_type) {
                if (MAC_SECURITY_PASSED == mac_secLvlCmp(security_level, pSecLvlDesc->security_min)) {
                    return MAC_SECURITY_PASSED;
                }
                else {
                    if (TRUE == pSecLvlDesc->device_override_security_min) {
                        return MAC_SECURITY_CONDITIONALLY_PASSED;
                    }
                    else {
                        return MAC_SECURITY_FAILED;
                    }
                }
            }
        }
        else {
            if ((frame_type == pSecLvlDesc->frame_type) && (cmd_frame_id == pSecLvlDesc->command_frame_id)) {
                if (MAC_SECURITY_PASSED == mac_secLvlCmp(security_level, pSecLvlDesc->security_min)) {
                    return MAC_SECURITY_PASSED;
                }
                else {
                    if (TRUE == pSecLvlDesc->device_override_security_min) {
                        return MAC_SECURITY_CONDITIONALLY_PASSED;
                    }
                    else {
                        return MAC_SECURITY_FAILED;
                    }
                }
            }
        }
    }

    return MAC_SECURITY_FAILED;
}

/*********************************************************************
 * @fn      mac_checkIncomingKeyUsagePolicy
 *
 * @brief   The inputs to this procedure are the KeyDescriptor, the frame
 *          type, and the command frame identifier. The output from this
 *          procedure is a passed or failed status.(see IEEE 802.15.4-2006
 *          p. 206)
 *
 * @param   pKeyDesc - input the KeyDescriptor
 *
 * @param   frame_type - input the incoming security level
 *
 * @param   cmd_frame_id - input the command frame identifier
 *
 * @return  status (indicating the result of the checking)
 */
mac_secStatus_t mac_checkIncomingKeyUsagePolicy(mac_keyDesc_t* pKeyDesc, u8 frame_type, u8 cmd_frame_id)
{
    // my_printf("mac_checkIncomingKeyUsagePolicy\n");
    // my_printf("frame_type = %d\n", frame_type);
    // my_printf("cmd_frame_id = %d\n", cmd_frame_id);
    mac_keyusageDesc_t* pKeyUsageDesc = (mac_keyusageDesc_t *)tn_list_head(pKeyDesc->key_usage_list);
    for (; pKeyUsageDesc; pKeyUsageDesc = (mac_keyusageDesc_t *)tn_list_item_next(pKeyUsageDesc)) {
        // my_printf("pKeyUsageDesc->frame_type = %d\n", pKeyUsageDesc->frame_type);
        // my_printf("pKeyUsageDesc->command_frame_id = %d\n", pKeyUsageDesc->command_frame_id);
        if (0x03 != frame_type) {
            if (frame_type == pKeyUsageDesc->frame_type) {
                return MAC_SECURITY_PASSED;
            }
        }
        else {
            if ((frame_type == pKeyUsageDesc->frame_type) && (cmd_frame_id == pKeyUsageDesc->command_frame_id)) {
                return MAC_SECURITY_PASSED;
            }
        }
    }

    return MAC_SECURITY_FAILED;
}

/*********************************************************************
 * @fn      mac_secLvlCmp
 *
 * @brief   Here, a first security level SEC1 is greater than or equal 
 *          to a second security level SEC2 if and only if SEC1 offers 
 *          at least the protection offered by SEC2, both with respect 
 *          to data confidentiality and with respect to data authenticity.
 *          The statement 鈥淪EC1 is greater than or equal to SEC2鈥� shall
 *          be evaluated as TRUE if both of the following conditions apply:
 *              a) Bit position b2 in SEC1 is greater than or equal to bit  
 *                 position b2 in SEC2 (where Encryption OFF <Encryption ON).
 *              b) The integer value of bit positions b1 b0 in SEC1 is greater
 *                 than or equal to the integer value of bit positions
 *                 b1 b0 in SEC2 (where increasing integer values indicate 
 *                 increasing levels of dataauthenticity provided, i.e., 
 *                 message integrity code (MIC)-0 < MIC-32 < MIC-64 < MIC-128).
 *          Otherwise, the statement shall be evaluated as FALSE.(see IEEE 
 *          802.15.4-2006 p. 210 )
 *
 * @param   sec1 - input the security level SEC1
 *
 * @param   sec2 - input the security level SEC1
 *
 * @return  result of comparing. 
 *          sec1>=sec2: MAC_SECURITY_PASSED
 *          sec1<sec2:  MAC_SECURITY_FAILED
 */
mac_secStatus_t mac_secLvlCmp(mac_secLevel_t sec1, mac_secLevel_t sec2)
{
    if (((sec1 & 0x04) >= (sec2 & 0x04)) && ((sec1 & 0x03) >= (sec2 & 0x03))) {
        return MAC_SECURITY_PASSED;
    }
    else {
        return MAC_SECURITY_FAILED;
    }
}

/*********************************************************************
 * @fn      mac_getAuxSecurityHeaderLen
 *
 * @brief   get length of the Auxiliary security header field of a general 
 *          MAC frame in octets.(see IEEE 802.15.4-2006 p. 210)
 *
 * @param   key_id_mode - input the KeyID Mode
 *
 * @return  the length of the Auxiliary security header field.
 */
u8 mac_getAuxSecurityHeaderLen(mac_keyidMode_t key_id_mode)
{
    u8 len = 0;

    switch (key_id_mode) {
    case KEY_ID_MODE_KEY_INDEX:
        len = 6;
        break;
    case KEY_ID_MODE_KEY_EXPLICIT_4:
        len = 10;
        break;
    case KEY_ID_MODE_KEY_EXPLICIT_8:
        len = 14;
        break; 
    default:
        len = 5;
    }

    return len;
}

/*********************************************************************
 * @fn      mac_secureOutgoingFrame
 *
 * @brief   The inputs to this procedure are the frame to be secured 
 *          and the SecurityLevel, KeyIdMode, KeySource, and KeyIndex 
 *          parameters from the originating primitive or automatic 
 *          request PIB attributes. The outputs from this procedure are
 *          the status of the procedure and, if this status is SUCCESS, 
 *          the secured frame.(see IEEE 802.15.4-2006 p. 199)
 *
 * @param   pData - input the frame to be secured
 *
 * @param   pSecurity - input the Security Info according to which to 
 *          secure the input mac frame
 *
 * @param   cmd_frame_id - input the command frame identifier
 *
 * @return  the status of the securing procedure
 */
u8 mac_secureOutgoingFrame(u8* pData, mac_sec_t* pSecurity)
{
    u8* pFrameCrtl;
    mac_secLevel_t security_level;
    u8 M, aux_len;
    mac_genericFrame_t* pMacFrame = (mac_genericFrame_t*)pData;

    /***check the input parameters***/
    if ((pMacFrame == NULL) || (pMacFrame->data.tx.psduLen < 5) || (pSecurity == NULL)) {
       return MAC_INVALID_PARAMETER; 
    }

    /***check the security level***/
    pFrameCrtl = (u8 *)((u32)pMacFrame+mac_tx_getFrameControlPos(pMacFrame));
    if (pSecurity->securityLevel > SECURITY_LEVEL_NONE) {
        security_level = pSecurity->securityLevel;
    }
    else {
        return MAC_UNSUPPORTED_SECURITY;
    }

    if ((!macPib.securityEnabled) && (security_level>SECURITY_LEVEL_NONE)) {
        return MAC_UNSUPPORTED_SECURITY;
    }

    /***determine the length M of the Authentication field in octets***/
    switch (security_level) {
    case SECURITY_LEVEL_MIC_32:
    case SECURITY_LEVEL_ENC_MIC_32:
        M = 4;
        break;
    case SECURITY_LEVEL_MIC_64:
    case SECURITY_LEVEL_ENC_MIC_64:
        M = 8;
        break;
    case SECURITY_LEVEL_MIC_128:
    case SECURITY_LEVEL_ENC_MIC_128:
        M = 16;
        break;
    default:
        M = 0;
    }

    /***determine whether the frame to be secured satisfies the constraint on the maximum length of MAC frames***/
    aux_len = mac_getAuxSecurityHeaderLen (pSecurity->key_id_mode);
    if (MAC_A_MAX_PHY_PACKET_SIZE < (pMacFrame->data.tx.psduLen+M+aux_len)) {
        return MAC_FRAME_TOO_LONG;
    }

    /***If the security level is zero, the procedure shall set the secured frame to be the frame to be secured and return with the secured frame and a status of SUCCESS***/
    if (SECURITY_LEVEL_NONE == security_level) {
        return MAC_SUCCESS;
    }

    /***If the frame counter has the value 0xffffffff, the procedure shall return with a status of COUNTER_ERROR.***/
    if (0xffffffff == macPib.frameCounter) {
        return MAC_COUNTER_ERROR;
    }

    /***Obtain the key using the outgoing frame key retrieval procedure.***/
    u8* pKey = NULL;
    if (MAC_SECURITY_FAILED == mac_getOutgoingFrameKey(pMacFrame, pSecurity, &pKey)) {
        return MAC_UNAVAILABLE_KEY;
    }

    /***make room for the auxiliary security header***/
    u8* pMACPayload = (u8*)((u32)pMacFrame + mac_tx_getMacPayloadPos(pMacFrame));
    u8 macPayloadLen = mac_tx_getMacPayloadLen(pMacFrame);
    my_memcpy(pMACPayload+aux_len, pMACPayload, macPayloadLen);
    pMacFrame->data.tx.psduLen += aux_len;

    /***fill the auxiliary security header field of the frame***/
    pMACPayload[0] = (pSecurity->key_id_mode<<SEC_CRTL_KEYID_MODE_POS)|security_level;//Security Control field
    my_memcpy(&(pMACPayload[1]), &(macPib.frameCounter), 4);//Frame Counter field
    switch (pSecurity->key_id_mode) {//Key Indentifier field
    case KEY_ID_MODE_KEY_INDEX:
        pMACPayload[5] = pSecurity->key_index;
        break;
    case KEY_ID_MODE_KEY_EXPLICIT_4:
        my_memcpy (&(pMACPayload[5]), pSecurity->key_source, 4);
        pMACPayload[9] = pSecurity->key_index;
        break;
    case KEY_ID_MODE_KEY_EXPLICIT_8:
        my_memcpy (&(pMACPayload[5]), pSecurity->key_source, 8);
        pMACPayload[13] = pSecurity->key_index;
        break;
    default:
        break;
    }

    /*
     * The security scheme in IEEE 802.15.4-2006 is not compatible with
     * the one in IEEE 802.15.4-2003, to let the recipient know which
     * scheme is used the frame version is set to 1.
    */
    *(pFrameCrtl+1) &= (~0x30);
    *(pFrameCrtl+1) |= 0x10;
    if (security_level > SECURITY_LEVEL_NONE) {
        (*pFrameCrtl) |= 0x08;
    }

    /***build nounce for CCM* mode security operation***/
    u8 nounce[13];
    nounce[0] = macPib.extendedAddress[7];
    nounce[1] = macPib.extendedAddress[6];
    nounce[2] = macPib.extendedAddress[5];
    nounce[3] = macPib.extendedAddress[4];
    nounce[4] = macPib.extendedAddress[3];
    nounce[5] = macPib.extendedAddress[2];
    nounce[6] = macPib.extendedAddress[1];
    nounce[7] = macPib.extendedAddress[0];
    nounce[8] = *(u8*)((u32)(&(macPib.frameCounter))+3);
    nounce[9] = *(u8*)((u32)(&(macPib.frameCounter))+2);
    nounce[10] = *(u8*)((u32)(&(macPib.frameCounter))+1);
    nounce[11] = *(u8*)(&macPib.frameCounter);
    nounce[12] = security_level;

    /***locate the position of actual payload to determine the mstring and astring***/
    u8 payload_pos = mac_tx_getActualPayloadPos(pMacFrame);
    if (payload_pos == 0) {
        return MAC_INVALID_PARAMETER;
    }
    payload_pos += aux_len; //beause the MACPayload has been moved forward aux_len octets
    u8* mStr = NULL;
    u8 mStrLen = 0;
    u8* aStr = NULL;
    u8 aStrLen = 0;
    u8* mic = NULL;

    if (security_level < SECURITY_LEVEL_ENC) {
        aStr = pFrameCrtl;
        aStrLen = payload_pos-((u32)pFrameCrtl-(u32)pMacFrame);
        mStr = NULL;
        mStrLen = 0;
        mic = pFrameCrtl + pMacFrame->data.tx.psduLen;
    } 
    else {
        mStr = (u8*)((u32)pMacFrame + payload_pos);
        mStrLen = pMacFrame->data.tx.psduLen - (payload_pos-((u32)pFrameCrtl-(u32)pMacFrame)) ;
        if (security_level > SECURITY_LEVEL_ENC) {
            aStr = pFrameCrtl;
            aStrLen = payload_pos-((u32)pFrameCrtl-(u32)pMacFrame);
            mic = pFrameCrtl + pMacFrame->data.tx.psduLen;
        }
    }

    /***do CCM* authentication transformation***/ 
    aes_ccmAuthTran(M, pKey, nounce, mStr, mStrLen, aStr, aStrLen, mic);

    /***do CCM* encryption transformation***/
    aes_ccmEncTran(M, pKey, nounce, mStr, mStrLen, NULL, 0, mic);

    /***adjust the MAC frame length after the CCM* transformation***/
    pMacFrame->data.tx.psduLen += M;

    /***increment the frame counter by one and set the macFrameCounter attribute to the resulting value***/
    macPib.frameCounter++;

    /***return with the secured frame and a status of SUCCESS***/
    return MAC_SUCCESS;
}


/*********************************************************************
 * @fn      mac_unsecureIncomingFrame
 *
 * @brief   The input to this procedure is the frame to be unsecured. 
 *          The outputs from this procedure are the unsecured frame, 
 *          the security level, the key identifier mode, the key source,
 *          the key index, and the status of the procedure. (see IEEE 
 *          802.15.4-2006 p. 201)
 *
 * @param   pData - input the frame to be unsecured
 * @return  the status of the unsecuring procedure
 */
u8 mac_unsecureIncomingFrame(u8* pData)
{
    mac_secLevel_t security_level;
    mac_keyidMode_t key_id_mode = KEY_ID_MODE_IMPLICIT;
    u32 frame_counter = 0;
    u8* pFrameCrtl;
    u8* pAuxSecHeader;
    u8 macHeaderLen = 0;
    u8 frameCrtlPos = 0;
    u8 M;
    mac_genericFrame_t* pMacFrame = (mac_genericFrame_t*)pData;

    /***get the length of the frame to be unsecured***/
    rx_buf_t* pRxEvt = (rx_buf_t*)ev_buf_getTail((void*)pMacFrame, sizeof(rx_buf_t));
    u8 frameLen = pRxEvt->len;

    /***check the input parameters***/
    if ((pMacFrame == NULL) || (frameLen < 5)) {
       return MAC_INVALID_PARAMETER; 
    }

    /***check the security level***/
    frameCrtlPos = mac_rx_getFrameControlPos(pMacFrame);
    pFrameCrtl = (u8 *)((u32)pMacFrame + frameCrtlPos);

    if (0 == ((*pFrameCrtl) & (u8)MAC_FCF_SEC_ENABLED_MASK)) {
        security_level = SECURITY_LEVEL_NONE;
    }
    else if (0 == ((*(pFrameCrtl+1)) & (u8)(MAC_FCF_FRAME_VERSION_MASK>>8))) {
        return MAC_UNSUPPORTED_LEGACY;
    }
    else {
        macHeaderLen = mac_rx_getMACHeaderLen(pMacFrame);
        pAuxSecHeader = pFrameCrtl + macHeaderLen;
        security_level = (pAuxSecHeader[0] & SEC_CRTL_SEC_LEVEL_MASK);
        key_id_mode = (pAuxSecHeader[0] & SEC_CRTL_KEYID_MODE_MASK) >> SEC_CRTL_KEYID_MODE_POS;
        my_memcpy(&frame_counter, &(pAuxSecHeader[1]), sizeof(frame_counter));
        if (security_level == SECURITY_LEVEL_NONE) {
            return MAC_UNSUPPORTED_SECURITY;
        }
    }

    /*If the macSecurityEnabled attribute is set to FALSE, the procedure shall set the unsecured 
     *frame to be the frame to be unsecured and return with the unsecured frame, the security level, 
     *the key identifier mode, the key source, the key index, and a status of SUCCESS if the security
     *level is equal to zero and with the unsecured frame, the security level, the key identifier 
     *mode, the key source, the key index, and a status of UNSUPPORTED_SECURITY otherwise.
     */
    if (!macPib.securityEnabled) {
        if (security_level == SECURITY_LEVEL_NONE) {
            return MAC_SUCCESS; 
        }
        else {
            return MAC_UNSUPPORTED_SECURITY;
        } 
    }

    /*If the security level is set to zero, the procedure shall set the unsecured frame to be
     * the frame to be unsecured and return with the unsecured frame, the security level, the
     * key identifier mode, the key source, the key index, and a status of SUCCESS.
     */
    if (security_level == SECURITY_LEVEL_NONE) {
        return MAC_SUCCESS;
    }

    /***determine whether the frame to be unsecured meets the minimum security level***/
    u8 aux_len = mac_getAuxSecurityHeaderLen(key_id_mode);
    u8 frameType = (*pFrameCrtl) & (u8)MAC_FCF_FRAME_TYPE_MASK;
    u8* pMACPayload = NULL;
    if (MAC_FRAME_TYPE_COMMAND == frameType) {
        pMACPayload = pAuxSecHeader + aux_len;
    }
    mac_secStatus_t secLevlChkStatus = mac_checkIncomingSecLevel(security_level, frameType, *pMACPayload);
    if (MAC_SECURITY_FAILED == secLevlChkStatus) {
        return MAC_IMPROPER_SECURITY_LEVEL;
    }

    /*obtain the KeyDescriptor, DeviceDescriptor, and KeyDeviceDescriptor using the incoming 
     *frame security material retrieval procedure.
     */
    mac_keyDesc_t* pKeyDesc = NULL;
    mac_deviceDesc_t* pDevDesc = NULL;
    mac_keydevDesc_t* pKeyDevDesc = NULL;
    if (MAC_SECURITY_FAILED == mac_getIncomingFrameSecMaterial(pMacFrame, pFrameCrtl, pAuxSecHeader, &pKeyDesc, &pDevDesc, &pKeyDevDesc)) {
        return MAC_UNAVAILABLE_KEY;
    }

    /*determine whether the frame to be unsecured conforms to the key usage policy by passing 
     *the KeyDescriptor, the frame type, and, depending on whether the frame is a MAC command
     *frame, the first octet of the MAC payload (i.e., command frame identifier for a MAC command
     * frame) to the incoming key usage policy checking procedure.
     */
    if (MAC_SECURITY_FAILED == mac_checkIncomingKeyUsagePolicy(pKeyDesc, frameType, *pMACPayload)) {
        return MAC_IMPROPER_KEY_TYPE;
    }

    /*If the Exempt element of the DeviceDescriptor is set to FALSE and if the incoming security
     *level checking procedure had as output the 鈥渃onditionally passed鈥� status, the procedure
     *shall set the unsecured frame to be the frame to be unsecured and return with the unsecured
     * frame, the security level, the key identifier mode, the key source, the key index, and a 
     *status of IMPROPER_SECURITY_LEVEL.
     */
    if ((FALSE == pDevDesc->exempt) && (MAC_SECURITY_CONDITIONALLY_PASSED == secLevlChkStatus)) {
        return MAC_IMPROPER_SECURITY_LEVEL;
    }

    /*If the frame counter has the value 0xffffffff, the procedure shall set the unsecured frame 
     *to be the frame to be unsecured and return with the unsecured frame, the security level, 
     *the key identifier mode, the key source, the key index, and a status of COUNTER_ERROR.
     */
    if (frame_counter == 0xffffffff) {
        return MAC_COUNTER_ERROR; 
    }

    /*determine whether the frame counter is greater than or equal to the FrameCounter element 
     *of the DeviceDescriptor. If this check fails, the procedure shall set the unsecured frame
     *to be the frame to be unsecured and return with the unsecured frame, the security level, 
     *the key identifier mode, the key source, the key index, and a status of COUNTER_ERROR.
     */
    if (!(frame_counter >= pDevDesc->frame_counter)) {
        return MAC_COUNTER_ERROR; 
    }

    /***build nounce for CCM* mode security operation***/
    u8 nounce[13];
    nounce[0] = pDevDesc->long_address[7];
    nounce[1] = pDevDesc->long_address[6];
    nounce[2] = pDevDesc->long_address[5];
    nounce[3] = pDevDesc->long_address[4];
    nounce[4] = pDevDesc->long_address[3];
    nounce[5] = pDevDesc->long_address[2];
    nounce[6] = pDevDesc->long_address[1];
    nounce[7] = pDevDesc->long_address[0];
    nounce[8] = *(u8*)((u32)(&frame_counter)+3);
    nounce[9] = *(u8*)((u32)(&frame_counter)+2);
    nounce[10] = *(u8*)((u32)(&frame_counter)+1);
    nounce[11] = *(u8*)(&frame_counter);
    nounce[12] = security_level;

    /***determine the length M of the Authentication field in octets***/
    switch (security_level) {
    case SECURITY_LEVEL_MIC_32:
    case SECURITY_LEVEL_ENC_MIC_32:
        M = 4;
        break;
    case SECURITY_LEVEL_MIC_64:
    case SECURITY_LEVEL_ENC_MIC_64:
        M = 8;
        break;
    case SECURITY_LEVEL_MIC_128:
    case SECURITY_LEVEL_ENC_MIC_128:
        M = 16;
        break;
    default:
        M = 0;
    }

    /***determine the aStr, aStrLen, mStr and mStrLen according to the security level***/
    u8* aStr = NULL;
    u8 aStrLen = 0;
    u8 payload_pos = 0;
    u8* mStr;
    u8 mStrLen;
    u8* mic = NULL; 
    if (security_level < SECURITY_LEVEL_ENC) {
        aStr = pFrameCrtl;
        aStrLen = frameLen - M - MAC_FCS_FIELD_LEN;
        mStr = NULL;
        mStrLen = 0;
    } 
    else {
        payload_pos = mac_rx_getActualPayloadPos(pMacFrame, macHeaderLen, aux_len);
        if (payload_pos == 0) {
            return MAC_INVALID_PARAMETER;
        }
        mStr = (u8*)((u32)pMacFrame + payload_pos);
        mStrLen = frameLen - MAC_FCS_FIELD_LEN - M - (payload_pos-frameCrtlPos);

        if (security_level > SECURITY_LEVEL_ENC) {
            aStr = pFrameCrtl;
            aStrLen = payload_pos - frameCrtlPos;
        }
    }
    mic = pFrameCrtl + frameLen - M - MAC_FCS_FIELD_LEN;

    /***do CCM* decryption***/
    aes_ccmDecTran(M, pKeyDesc->key, nounce, mStr, mStrLen, aStr, aStrLen, mic);

    /***do CCM* authentication checking***/
    if (aStrLen > 0) {
        if (0 != aes_ccmDecAuthTran(M, pKeyDesc->key, nounce, mStr, mStrLen, aStr, aStrLen, mic)) {
            return MAC_SECURITY_ERROR;
        }
        /***remove the M tag field***/
        frameLen -= M;
    }

    /***remove the auxiliary security header***/
    u8 macPayloadLen = frameLen - MAC_FCS_FIELD_LEN - macHeaderLen - aux_len;
    my_memcpy(pAuxSecHeader, pAuxSecHeader+aux_len, macPayloadLen);
    frameLen -= aux_len;

    /***adjust the frame length field of the unsecured frame***/
    pRxEvt->len = frameLen;

    /***remove the security flags of the frame***/
    (*pFrameCrtl) &= ~(u8)MAC_FCF_SEC_ENABLED_MASK;
    (*(pFrameCrtl+1)) &= ~(u8)(MAC_FCF_FRAME_VERSION_MASK>>8);

    /*increment the frame counter by one and set the FrameCounter element of the
     *DeviceDescriptor to the resulting value.
     */
    frame_counter++;
    pDevDesc->frame_counter = frame_counter;

    /*If the FrameCounter element is equal to 0xffffffff, the procedure shall set the
     *Blacklisted element of the KeyDeviceDescriptor.
     */
    if (pDevDesc->frame_counter == 0xffffffff) {
        pKeyDevDesc->blacklisted = TRUE;
    }
    
    return MAC_SUCCESS;
}

//macKeyTable operating functions
int mac_keyInsert(mac_keyDesc_t *key)
{
    if (!key) {
        return MAC_INVALID_PARAMETER;
    }

    return mac_mlmeSetReq(MAC_KEY_TABLE, macPib.keyTableEntries, (void *)key);
}

int mac_keySet(mac_keyDesc_t *key, int index)
{
    if (!key) {
        return MAC_INVALID_PARAMETER;
    }

    return mac_mlmeSetReq(MAC_KEY_TABLE, index, (void *)key);
}
mac_keyDesc_t *mac_keyGet(int index)
{
    mac_keyDesc_t *pKeyDesc;
    u8 status = mac_mlmeGetReq(MAC_KEY_TABLE, index, (void *)&pKeyDesc);
    if (status == MAC_SUCCESS) {
        return pKeyDesc;
    }
    else {
        return NULL;
    }
}

int mac_keyDelete(int index)
{
    return mac_mlmeDelReq(MAC_KEY_TABLE, index);
}

//macDeviceTable operating functions
int mac_DevInsert(mac_deviceDesc_t *dev)
{
    if (!dev) {
        return MAC_INVALID_PARAMETER;
    }

    return mac_mlmeSetReq(MAC_DEVICE_TABLE, macPib.devTableEntries, (void *)dev);

}

int mac_DevSet(mac_deviceDesc_t *dev, int index)
{
    if (!dev) {
        return MAC_INVALID_PARAMETER;
    }

    return mac_mlmeSetReq(MAC_DEVICE_TABLE, index, (void *)dev);
}

mac_deviceDesc_t *mac_DevGet(int index)
{
    mac_deviceDesc_t *pDevDesc;
    u8 status = mac_mlmeGetReq(MAC_DEVICE_TABLE, index, (void *)&pDevDesc);
    if (status == MAC_SUCCESS) {
        return pDevDesc;
    }
    else {
        return NULL;
    }
}

int mac_DevDelete(int index)
{
    return mac_mlmeDelReq(MAC_DEVICE_TABLE, index);
}

//macsecLvlTable operating functions
int mac_secLvlInsert(mac_seclevelDesc_t *secLvl)
{
    if (!secLvl) {
        return MAC_INVALID_PARAMETER;
    }

    return mac_mlmeSetReq(MAC_SECURITY_LEVEL_TABLE, macPib.secLvlTableEntries, (void *)secLvl);
}

int mac_secLvlSet(mac_seclevelDesc_t *secLvl, int index)
{
    if (!secLvl) {
        return MAC_INVALID_PARAMETER;
    }

    return mac_mlmeSetReq(MAC_SECURITY_LEVEL_TABLE, index, (void *)secLvl);
}

mac_seclevelDesc_t *mac_secLvlGet(int index)
{
    mac_seclevelDesc_t *pSecLvl;
    u8 status = mac_mlmeGetReq(MAC_SECURITY_LEVEL_TABLE, index, (void *)&pSecLvl);
    if (status == MAC_SUCCESS) {
        return pSecLvl;
    }
    else {
        return NULL;
    }
}

int mac_secLvlDelete(int index)
{
    return mac_mlmeDelReq(MAC_SECURITY_LEVEL_TABLE, index);
}

//macSecurityDescs allocate operations
mac_keyid_lookup_desc_t *mac_keyidDesc_alloc(void)
{
    return (mac_keyid_lookup_desc_t *)mac_secMaterial_alloc(MAC_SECMATERIAL_KEYID_LOOKUP_DESC);
}

mac_deviceDesc_t *mac_deviceDesc_alloc(void)
{
    return (mac_deviceDesc_t *)mac_secMaterial_alloc(MAC_SECMATERIAL_DEVICE_DESC);
}

mac_keydevDesc_t *mac_keydevDesc_alloc(void)
{
    return (mac_keydevDesc_t *)mac_secMaterial_alloc(MAC_SECMATERIAL_KEYDEV_DESC);
}

mac_keyusageDesc_t *mac_keyusageDesc_alloc(void)
{
    return (mac_keyusageDesc_t *)mac_secMaterial_alloc(MAC_SECMATERIAL_KEYUSAGE_DESC);
}

mac_keyDesc_t *mac_keyDesc_alloc(void)
{
    return (mac_keyDesc_t *)mac_secMaterial_alloc(MAC_SECMATERIAL_KEY_DESC);
}

mac_seclevelDesc_t *mac_seclevelDesc_alloc(void)
{
    return (mac_seclevelDesc_t *)mac_secMaterial_alloc(MAC_SECMATERIAL_SECLEVEL_DESC);
}

//macSecurityDescs set operations
void mac_keyidDesc_set(mac_keyid_lookup_desc_t *desc, mac_lookupData_size_t size, u8 *data)
{
    memset(desc, 0, sizeof(mac_keyid_lookup_desc_t));
    desc->lookup_data_size = size;
    memcpy(desc->lookup_data, data, size ? 9 : 5);
}

void mac_deviceDesc_set(mac_deviceDesc_t *desc, u16 panID, u16 shortAddr, u8 *extAddr, u32 frameCnt, bool exempt)
{
    memset(desc, 0, sizeof(mac_deviceDesc_t));
    desc->pan_id = panID;
    desc->short_address = shortAddr;
    memcpy(desc->long_address, extAddr, 8);
    desc->frame_counter = frameCnt;
    desc->exempt = exempt;
}

void mac_keydevDesc_set(mac_keydevDesc_t *desc, mac_deviceDesc_t *devDesc, bool uniDev, bool blackLst)
{
    memset(desc, 0, sizeof(mac_keydevDesc_t));
    desc->device_descriptor = devDesc;
    desc->unique_device = uniDev;
    desc->blacklisted = blackLst;
}

void mac_keyusageDesc_set(mac_keyusageDesc_t *desc, u8 frameType, u8 cmdFrameID)
{
    memset(desc, 0, sizeof(mac_keyusageDesc_t));
    desc->frame_type = frameType;
    desc->command_frame_id = cmdFrameID;
}

void mac_keyDesc_set(mac_keyDesc_t *desc, 
                     mac_keyid_lookup_desc_t **keyIDLst, u8 keyIDLstLen,
                     mac_keydevDesc_t **keydevLst, u8 keydevLstLen,
                     mac_keyusageDesc_t **keyusageLst, u8 keyusageLstLen,
                     u8 *key)
{
    int i = 0;

    memset(desc, 0, sizeof(mac_keyDesc_t));

    LIST_STRUCT_INIT(desc, key_id_lookup_list);
    for (i = 0; i < keyIDLstLen; i++) {
        tn_list_add(desc->key_id_lookup_list, keyIDLst[i]);
    }
    desc->key_id_lookup_list_entries = keyIDLstLen;

    LIST_STRUCT_INIT(desc, key_device_list);
    for (i = 0; i < keydevLstLen; i++) {
        tn_list_add(desc->key_device_list, keydevLst[i]);
    }
    desc->key_device_list_entries = keydevLstLen;

    LIST_STRUCT_INIT(desc, key_usage_list);
    for (i = 0; i < keyusageLstLen; i++) {
        tn_list_add(desc->key_usage_list, keyusageLst[i]);
    }
    desc->key_usage_list_entries = keyusageLstLen;
    
    memcpy(desc->key, key, 16);
}

void mac_seclevelDesc_set(mac_seclevelDesc_t *desc, u8 frameType, u8 cmdFrameID, mac_secLevel_t min, bool override)
{
    desc->frame_type = frameType;
    desc->command_frame_id = cmdFrameID;
    desc->security_min = min;
    desc->device_override_security_min = override;
}



