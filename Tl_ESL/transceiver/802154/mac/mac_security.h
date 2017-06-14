#ifndef MAC_SECURITY_H
#define MAC_SECURITY_H
/**************************************************************************************
  Filename:       mac_security.h
  Revised:        $Date: 2015-15-04 $

  Description:    Interface file for the MAC security module
**************************************************************************************/

#include "../../common.h"
#include "../common/tn_list.h"
#include "../common/tn_mm.h"

/* MAC security levels (see IEEE 802.15.4-2006 p. 211)*/
typedef enum {
    SECURITY_LEVEL_NONE = 0x00,
    SECURITY_LEVEL_MIC_32 = 0x01,
    SECURITY_LEVEL_MIC_64 = 0x02,
    SECURITY_LEVEL_MIC_128 = 0x03,
    SECURITY_LEVEL_ENC = 0x04,
    SECURITY_LEVEL_ENC_MIC_32 = 0x05,
    SECURITY_LEVEL_ENC_MIC_64 = 0x06,
    SECURITY_LEVEL_ENC_MIC_128 = 0x07
} mac_secLevel_t;

/*Key identifier modes (see IEEE 802.15.4-2006 p. 212)*/
typedef enum {
    KEY_ID_MODE_IMPLICIT = 0x00,
    KEY_ID_MODE_KEY_INDEX = 0x01,
    KEY_ID_MODE_KEY_EXPLICIT_4 = 0x02,
    KEY_ID_MODE_KEY_EXPLICIT_8 = 0x03
} mac_keyidMode_t;

/*Key source length definition in Mac security*/
#define MAC_KEY_SOURCE_MAX_LEN          8

typedef u8 mac_keySource_t[MAC_KEY_SOURCE_MAX_LEN];
typedef u8 mac_key_t[16];

/*Definition of Security related information data structure.*/
typedef struct {
    mac_secLevel_t securityLevel;
    mac_keySource_t key_source;
    mac_keyidMode_t key_id_mode;
    u8 key_index; 
} mac_sec_t;


/*Definition of KeyIdLookupDescriptor(see IEEE 802.15.4-2006 p. 210)*/
typedef enum {
    LOOKUP_DATA_SIZE_5 = 0x00,
    LOOKUP_DATA_SIZE_9 = 0x01
} mac_lookupData_size_t;

typedef struct {
    struct mac_keyid_lookup_desc_t *next;
    u8 lookup_data[9];
    mac_lookupData_size_t lookup_data_size;
    u8 reserved[2];
} mac_keyid_lookup_desc_t;

/*Definition of Device Descriptor(see IEEE 802.15.4-2006 p. 209)*/
typedef struct {
    struct mac_deviceDesc_t *next;
    u32 frame_counter;
    u16 pan_id;
    u16 short_address;
    u8 long_address[8];
    bool exempt;
    u8 reserved[3];
} mac_deviceDesc_t;

/*Definition of KeyDeviceDescriptor(see IEEE 802.15.4-2006 p. 208)*/
typedef struct {
    struct mac_keydevDesc_t *next;
    mac_deviceDesc_t *device_descriptor;
    bool unique_device;
    bool blacklisted;
    u8 reserved[2];
} mac_keydevDesc_t;

/*Definition of KeyUsageDescriptor(see IEEE 802.15.4-2006 p. 208)*/
typedef struct {
    struct mac_keyusageDesc_t *next;
    u8 frame_type;
    u8 command_frame_id;
    u8 reserved[2];
} mac_keyusageDesc_t;

/*Definition of KeyDescriptor(see IEEE 802.15.4-2006 p. 208)*/
typedef struct {
    struct mac_keyDesc_t *next;
    LIST_STRUCT_DEF(key_id_lookup_list);
    LIST_STRUCT_DEF(key_device_list);
    LIST_STRUCT_DEF(key_usage_list);
    mac_key_t key;
    u8 key_id_lookup_list_entries;
    u8 key_device_list_entries;
    u8 key_usage_list_entries;
    u8 reserved;
} mac_keyDesc_t;

/*Definition of SecurityLevelDescriptor(see IEEE 802.15.4-2006 p. 209)*/
typedef struct {
    struct mac_seclevelDesc_t *next;
    u8 frame_type;
    u8 command_frame_id;
    mac_secLevel_t security_min;
    bool device_override_security_min;
} mac_seclevelDesc_t;

/*Used by blacklist_check*/
typedef struct {
    const mac_deviceDesc_t* dev_descriptor;
    const mac_keydevDesc_t* key_dev_descriptor;
} blacklist_check_result_t;

/*Definition of Auxiliary Security HDR(see IEEE 802.15.4-2006 p. 210)*/
#define    SEC_CRTL_SEC_LEVEL_MASK     0x07   //bit:0-2
#define    SEC_CRTL_KEYID_MODE_MASK    0x18   //bit:3-4
#define    SEC_CRTL_SEC_LEVEL_POS      0   //bit:0-2
#define    SEC_CRTL_KEYID_MODE_POS     3   //bit:3-4
typedef struct {
    u32 frame_counter;
    mac_keySource_t key_source;
    u8 security_control;
    u8 key_index;
} mac_auxSecHeader_t;


u8 mac_getAuxSecurityHeaderLen(mac_keyidMode_t key_id_mode);
u8 mac_secureOutgoingFrame(u8* pData, mac_sec_t* pSecurity);
u8 mac_unsecureIncomingFrame(u8* pData);
u8* my_memcpy(u8* dest, const u8* src, unsigned int count);

//macKeyTable operating functions
int mac_keyInsert(mac_keyDesc_t *key); //add a key entry to the tail of macKeyTable
int mac_keySet(mac_keyDesc_t *key, int index); //set the key entry specified by index in macKeyTable 
mac_keyDesc_t *mac_keyGet(int index); //get the key entry specified by index in macKeyTable 
int mac_keyDelete(int index); //delete the key entry specified by index from macKeyTable 

//macDeviceTable operating functions
int mac_DevInsert(mac_deviceDesc_t *dev); //add a device entry to the tail of macDeviceTable
int mac_DevSet(mac_deviceDesc_t *dev, int index); //set the device entry specified by index in macDeviceTable 
mac_deviceDesc_t *mac_DevGet(int index); //get the device entry specified by index in macDeviceTable 
int mac_DevDelete(int index); //delete the device entry specified by index from macDeviceTable 

//macsecLvlTable operating functions
int mac_secLvlInsert(mac_seclevelDesc_t *secLvl); //add a secLvl entry to the tail of macsecLvlTable
int mac_secLvlSet(mac_seclevelDesc_t *secLvl, int index); //set the secLvl entry specified by index in macsecLvlTable 
mac_seclevelDesc_t *mac_secLvlGet(int index); //get the secLvl entry specified by index in macsecLvlTable 
int mac_secLvlDelete(int index); //delete the secLvl entry specified by index from macsecLvlTable 

//macSecurityDescs allocate operations
mac_keyid_lookup_desc_t *mac_keyidDesc_alloc(void);
mac_deviceDesc_t *mac_deviceDesc_alloc(void);
mac_keydevDesc_t *mac_keydevDesc_alloc(void);
mac_keyusageDesc_t *mac_keyusageDesc_alloc(void);
mac_keyDesc_t *mac_keyDesc_alloc(void);
mac_seclevelDesc_t *mac_seclevelDesc_alloc(void);

//macSecurityDescs set operations
void mac_keyidDesc_set(mac_keyid_lookup_desc_t *desc, mac_lookupData_size_t size, u8 *data);
void mac_deviceDesc_set(mac_deviceDesc_t *desc, u16 panID, u16 shortAddr, u8 *extAddr, u32 frameCnt, bool exempt);
void mac_keydevDesc_set(mac_keydevDesc_t *desc, mac_deviceDesc_t *devDesc, bool uniDev, bool blackLst);
void mac_keyusageDesc_set(mac_keyusageDesc_t *desc, u8 frameType, u8 cmdFrameID);
void mac_keyDesc_set(mac_keyDesc_t *desc, 
                     mac_keyid_lookup_desc_t **keyIDLst, u8 keyIDLstLen,
                     mac_keydevDesc_t **keydevLst, u8 keydevLstLen,
                     mac_keyusageDesc_t **keyusageLst, u8 keyusageLstLen,
                     u8 *key);
void mac_seclevelDesc_set(mac_seclevelDesc_t *desc, u8 frameType, u8 cmdFrameID, mac_secLevel_t min, bool override);

#endif /* MAC_SECURITY_H */
