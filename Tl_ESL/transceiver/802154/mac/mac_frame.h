#pragma once

#include "mac_include.h"

/**        
 *  @brief Telink Mac Command frame identifiers
 */
#define TL_MAC_DATA_FRAME                  0
#define TL_MAC_ASSOC_REQ_FRAME             1
#define TL_MAC_ASSOC_RSP_FRAME             2
#define TL_MAC_DISASSOC_NOTIF_FRAME        3
#define TL_MAC_DATA_REQ_FRAME              4
#define TL_MAC_PAN_CONFLICT_FRAME          5
#define TL_MAC_ORPHAN_NOTIF_FRAME          6
#define TL_MAC_BEACON_REQ_FRAME            7
#define TL_MAC_COORD_REALIGN_FRAME         8
#define TL_MAC_GTS_REQ_FRAME               9
#define TL_MAC_BEACON_FRAME                10

#define MAC_DATA_FRAME                      TL_MAC_DATA_FRAME                  
#define MAC_ASSOC_REQ_FRAME                 TL_MAC_ASSOC_REQ_FRAME             
#define MAC_ASSOC_RSP_FRAME                 TL_MAC_ASSOC_RSP_FRAME             
#define MAC_DISASSOC_NOTIF_FRAME            TL_MAC_DISASSOC_NOTIF_FRAME        
#define MAC_DATA_REQ_FRAME                  TL_MAC_DATA_REQ_FRAME              
#define MAC_PAN_CONFLICT_FRAME              TL_MAC_PAN_CONFLICT_FRAME          
#define MAC_ORPHAN_NOTIF_FRAME              TL_MAC_ORPHAN_NOTIF_FRAME          
#define MAC_BEACON_REQ_FRAME                TL_MAC_BEACON_REQ_FRAME            
#define MAC_COORD_REALIGN_FRAME             TL_MAC_COORD_REALIGN_FRAME         
#define MAC_GTS_REQ_FRAME                   TL_MAC_GTS_REQ_FRAME               
#define MAC_BEACON_FRAME                    TL_MAC_BEACON_FRAME                


/**        
 *  @brief Length of command frame payload (includes command ID byte)
 */
#define TL_MAC_ZERO_DATA_PAYLOAD           0
#define TL_MAC_ASSOC_REQ_PAYLOAD           2
#define TL_MAC_ASSOC_RSP_PAYLOAD           4
#define TL_MAC_DISASSOC_NOTIF_PAYLOAD      2
#define TL_MAC_DATA_REQ_PAYLOAD            1
#define TL_MAC_PAN_CONFLICT_PAYLOAD        1
#define TL_MAC_ORPHAN_NOTIF_PAYLOAD        1
#define TL_MAC_BEACON_REQ_PAYLOAD          1
#define TL_MAC_COORD_REALIGN_PAYLOAD       9
#define TL_MAC_GTS_REQ_PAYLOAD             2

#define TL_MAC_MIN_HDR_LEN                 3
#define TL_MAC_MAX_HDR_LEN                 23
#define TELINK_RF_TX_HDR_LEN               5
#define TELINK_RF_RX_HDR_LEN               13


#define MAC_ZERO_DATA_PAYLOAD           TL_MAC_ZERO_DATA_PAYLOAD        
#define MAC_ASSOC_REQ_PAYLOAD           TL_MAC_ASSOC_REQ_PAYLOAD        
#define MAC_ASSOC_RSP_PAYLOAD           TL_MAC_ASSOC_RSP_PAYLOAD        
#define MAC_DISASSOC_NOTIF_PAYLOAD      TL_MAC_DISASSOC_NOTIF_PAYLOAD   
#define MAC_DATA_REQ_PAYLOAD            TL_MAC_DATA_REQ_PAYLOAD         
#define MAC_PAN_CONFLICT_PAYLOAD        TL_MAC_PAN_CONFLICT_PAYLOAD     
#define MAC_ORPHAN_NOTIF_PAYLOAD        TL_MAC_ORPHAN_NOTIF_PAYLOAD     
#define MAC_BEACON_REQ_PAYLOAD          TL_MAC_BEACON_REQ_PAYLOAD       
#define MAC_COORD_REALIGN_PAYLOAD       TL_MAC_COORD_REALIGN_PAYLOAD    
#define MAC_GTS_REQ_PAYLOAD             TL_MAC_GTS_REQ_PAYLOAD          
#define MAC_MIN_HDR_LEN                 TL_MAC_MIN_HDR_LEN              
#define MAC_MAX_HDR_LEN                 TL_MAC_MAX_HDR_LEN              

/**        
 *  @brief MAC frame field lengths in bytes
 */
#define MAC_FCF_FIELD_LEN               0x02       
#define MAC_SEQ_NUM_FIELD_LEN           0x01       
#define MAC_PAN_ID_FIELD_LEN            0x02       
#define MAC_EXT_ADDR_FIELD_LEN          0x08       
#define MAC_SHORT_ADDR_FIELD_LEN        0x02       
#define MAC_FCS_FIELD_LEN               0x02       

/**        
 *  @brief Frame offsets in bytes
 */
#define MAC_FCF_OFFSET                  0x00       
#define MAC_SEQ_NUM_OFFSET              0x02       
#define MAC_DEST_PAN_ID_OFFSET          0x03       
#define MAC_DEST_ADDR_OFFSET            0x05       

/**        
 *  @brief Frame control field bit masks
 */
#define TL_MAC_FCF_FRAME_TYPE_MASK         0x0007
#define TL_MAC_FCF_SEC_ENABLED_MASK        0x0008
#define TL_MAC_FCF_FRAME_PENDING_MASK      0x0010
#define TL_MAC_FCF_ACK_REQUEST_MASK        0x0020
#define TL_MAC_FCF_INTRA_PAN_MASK          0x0040
#define TL_MAC_FCF_DST_ADDR_MODE_MASK      0x0C00
#define TL_MAC_FCF_FRAME_VERSION_MASK      0x3000
#define TL_MAC_FCF_SRC_ADDR_MODE_MASK      0xC000

#define MAC_FCF_FRAME_TYPE_MASK           TL_MAC_FCF_FRAME_TYPE_MASK      
#define MAC_FCF_SEC_ENABLED_MASK          TL_MAC_FCF_SEC_ENABLED_MASK     
#define MAC_FCF_FRAME_PENDING_MASK        TL_MAC_FCF_FRAME_PENDING_MASK   
#define MAC_FCF_ACK_REQUEST_MASK          TL_MAC_FCF_ACK_REQUEST_MASK     
#define MAC_FCF_INTRA_PAN_MASK            TL_MAC_FCF_INTRA_PAN_MASK       
#define MAC_FCF_DST_ADDR_MODE_MASK        TL_MAC_FCF_DST_ADDR_MODE_MASK   
#define MAC_FCF_FRAME_VERSION_MASK        TL_MAC_FCF_FRAME_VERSION_MASK   
#define MAC_FCF_SRC_ADDR_MODE_MASK        TL_MAC_FCF_SRC_ADDR_MODE_MASK   

/**        
 *  @brief Frame control field bit positions
 */
#define MAC_FCF_FRAME_TYPE_POS          0x00
#define MAC_FCF_SEC_ENABLED_POS         0x03
#define MAC_FCF_FRAME_PENDING_POS       0x04
#define MAC_FCF_ACK_REQUEST_POS         0x05
#define MAC_FCF_INTRA_PAN_POS           0x06
#define MAC_FCF_DST_ADDR_MODE_POS       0x0a
#define MAC_FCF_FRAME_VERSION_POS       0x0c
#define MAC_FCF_SRC_ADDR_MODE_POS       0x0e


/**        
 *  @brief Beacon superframe spec bit positions
 */
#define MAC_SFS_BEACON_ORDER_POS        0
#define MAC_SFS_SUPERFRAME_ORDER_POS    4
#define MAC_SFS_FINAL_CAP_SLOT_POS      8
#define MAC_SFS_BATT_LIFE_EXT_POS       12
#define MAC_SFS_PAN_COORD_POS           14
#define MAC_SFS_ASSOC_PERMIT_POS        15

/**        
 *  @brief Frame type
 */
#define MAC_FRAME_TYPE_BEACON           0
#define MAC_FRAME_TYPE_DATA             1
#define MAC_FRAME_TYPE_ACK              2
#define MAC_FRAME_TYPE_COMMAND          3
#define MAC_FRAME_TYPE_MAX_VALID        MAC_FRAME_TYPE_COMMAND

/**        
 *  @brief Length of command frames (max header plus payload)
 */
#define TL_MAC_ZERO_DATA_FRAME_LEN         (21 + MAC_ZERO_DATA_PAYLOAD)
#define TL_MAC_ASSOC_REQ_FRAME_LEN         (17 + MAC_ASSOC_REQ_PAYLOAD)//(23 + MAC_ASSOC_REQ_PAYLOAD)
#define TL_MAC_ASSOC_RSP_FRAME_LEN         (21 + MAC_ASSOC_RSP_PAYLOAD)
#define TL_MAC_DISASSOC_NOTIF_FRAME_LEN    (17 + MAC_DISASSOC_NOTIF_PAYLOAD)
#define TL_MAC_DATA_REQ_FRAME_LEN          (9 + MAC_DATA_REQ_PAYLOAD)
#define TL_MAC_PAN_CONFLICT_FRAME_LEN      (23 + MAC_PAN_CONFLICT_PAYLOAD)
#define TL_MAC_ORPHAN_NOTIF_FRAME_LEN      (17 + MAC_ORPHAN_NOTIF_PAYLOAD)
#define TL_MAC_BEACON_REQ_FRAME_LEN        (7 + MAC_BEACON_REQ_PAYLOAD)
#define TL_MAC_COORD_REALIGN_FRAME_LEN     (23 + MAC_COORD_REALIGN_PAYLOAD)
#define TL_MAC_GTS_REQ_FRAME_LEN           (7 + MAC_GTS_REQ_PAYLOAD)

#define MAC_ZERO_DATA_FRAME_LEN        TL_MAC_ZERO_DATA_FRAME_LEN        
#define MAC_ASSOC_REQ_FRAME_LEN        TL_MAC_ASSOC_REQ_FRAME_LEN        
#define MAC_ASSOC_RSP_FRAME_LEN        TL_MAC_ASSOC_RSP_FRAME_LEN        
#define MAC_DISASSOC_NOTIF_FRAME_LEN   TL_MAC_DISASSOC_NOTIF_FRAME_LEN   
#define MAC_DATA_REQ_FRAME_LEN         TL_MAC_DATA_REQ_FRAME_LEN         
#define MAC_PAN_CONFLICT_FRAME_LEN     TL_MAC_PAN_CONFLICT_FRAME_LEN     
#define MAC_ORPHAN_NOTIF_FRAME_LEN     TL_MAC_ORPHAN_NOTIF_FRAME_LEN     
#define MAC_BEACON_REQ_FRAME_LEN       TL_MAC_BEACON_REQ_FRAME_LEN       
#define MAC_COORD_REALIGN_FRAME_LEN    TL_MAC_COORD_REALIGN_FRAME_LEN    
#define MAC_GTS_REQ_FRAME_LEN          TL_MAC_GTS_REQ_FRAME_LEN          

/**        
 *  @brief Beacon frame base length (max header plus minimum payload)
 */
#define MAC_BEACON_FRAME_BASE_LEN       (13 + 4)


#define MAC_TX_OFFSET_LEN   OFFSETOF(mac_genericFrame_t, psdu)

#define MAC_NO_GTS_FIELD                0
#define MAC_NO_PENDING_ADDR_FIELD       0
#define MAC_POLL_CNF_PENDING_MASK       0x80

mac_sts_t mac_sendBeacon(u8* pData);
mac_sts_t mac_sendDataFrame(mac_genericFrame_t* pData);
mac_sts_t mac_buildCommonReq(u8* pBuff, u8 cmdID, u8 srcAddrMode, addr_t* pDstAddr, u16 dstPanID, u8 txOption, u8* payload);

 //added by zhangjian for 802.15.4 security 2015/10/20
 u8* mac_getDstPANIDPos(u8* pFrameCtrl);
 u8* mac_getSrcPANIDPos(u8* pFrameCtrl);
 u8* mac_getDstAddrPos(u8* pFrameCtrl);
 u8* mac_getSrcAddrPos(u8* pFrameCtrl);

 u8 mac_tx_getFrameControlPos(mac_genericFrame_t* pData);
 u8 mac_tx_getMacPayloadPos(mac_genericFrame_t* pData);
 u8 mac_tx_getMacPayloadLen(mac_genericFrame_t* pData);
 u8 mac_tx_getActualPayloadPos(mac_genericFrame_t* pData);

 u8 mac_rx_getFrameControlPos(mac_genericFrame_t* pData);
 u8 mac_rx_getMACHeaderLen(mac_genericFrame_t* pData);
 u8 mac_rx_getActualPayloadPos(mac_genericFrame_t* pData, u8 macHDRLen, u8 auxLen);
