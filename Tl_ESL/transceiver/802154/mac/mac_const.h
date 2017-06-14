/**************************************************************************************************
  Filename:       mac_const.h
  Revised:        $Date: 2013-05-27 $

  Description:    Constant definition in MAC layer 
**************************************************************************************************/

#pragma once

/** @addtogroup  TELINK_802_15_4_STACK TELINK 802.15.4 Stack
 *  @{
 */

/** @addtogroup  MAC_Module MAC
 *  @{
 */
 
/** @addtogroup  MAC_Constant MAC Constants
 *  @{
 */

 
/** @addtogroup mac_channel MAC Channels
 * Definition of MAC Channels
 * @{
 */
 
/** @addtogroup mac_channel_id MAC Channels Index
 * @{
 */
#define MAC_CHAN_11                 11
#define MAC_CHAN_12                 12
#define MAC_CHAN_13                 13
#define MAC_CHAN_14                 14
#define MAC_CHAN_15                 15
#define MAC_CHAN_16                 16
#define MAC_CHAN_17                 17
#define MAC_CHAN_18                 18
#define MAC_CHAN_19                 19
#define MAC_CHAN_20                 20
#define MAC_CHAN_21                 21
#define MAC_CHAN_22                 22
#define MAC_CHAN_23                 23
#define MAC_CHAN_24                 24
#define MAC_CHAN_25                 25
#define MAC_CHAN_26                 26
#define MAC_CHAN_27                 27
#define MAC_CHAN_28                 28
#define MAC_CHAN_BEG                MAC_CHAN_11
#define MAC_CHAN_END                MAC_CHAN_28

/** @} end of group mac_channel_id */

/**        
 *  @brief This macro converts a channel to a mask
 */
#define TL_MAC_CHANNEL_MASK(chan)         ((u32) 1 << (chan))


/** @addtogroup mac_channel_mask MAC Channels Mask
 *  Channel Masks
 * @{
 */
#define MAC_CHAN_11_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_11)
#define MAC_CHAN_12_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_12)
#define MAC_CHAN_13_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_13)
#define MAC_CHAN_14_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_14)
#define MAC_CHAN_15_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_15)
#define MAC_CHAN_16_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_16)
#define MAC_CHAN_17_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_17)
#define MAC_CHAN_18_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_18)
#define MAC_CHAN_19_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_19)
#define MAC_CHAN_20_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_20)
#define MAC_CHAN_21_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_21)
#define MAC_CHAN_22_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_22)
#define MAC_CHAN_23_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_23)
#define MAC_CHAN_24_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_24)
#define MAC_CHAN_25_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_25)
#define MAC_CHAN_26_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_26)
#define MAC_CHAN_27_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_27)
#define MAC_CHAN_28_MASK            TL_MAC_CHANNEL_MASK(MAC_CHAN_28)

/** @} end of group mac_channel_mask */


/** @addtogroup mac_channel_page MAC Channels Page
 *  Channel Page
 * @{
 */
#define MAC_CHANNEL_PAGE_0          0     //!< 2.4 GHz band using O-QPSK
#define MAC_CHANNEL_PAGE_1          1     //!< 868 and 915 MHz bands using ASK
#define MAC_CHANNEL_PAGE_2          2     //!< 868 and 915 MHz bands using O-QPSK

/** @} end of group mac_channel_page */

/** @} end of group mac_channel */


/**        
 *  @brief Maximum application data length without security
 */
#define MAC_MAX_FRAME_SIZE          102

/**        
 *  @brief Maximum auxiliary security header
 */
#define MAC_MAX_AUX_SEC_HDR_SIZE    14


/** @addtogroup mac_capability_info MAC Capability Information
 *  Bit mask of Capability Information
 * @{
 */
#define MAC_CAPABLE_PAN_COORD       0x01  //!< Device is capable of becoming a PAN coordinator
#define MAC_CAPABLE_FFD             0x02  //!< Device is an FFD
#define MAC_CAPABLE_MAINS_POWER     0x04  //!< Device is mains powered rather than battery powered
#define MAC_CAPABLE_RX_ON_IDLE      0x08  //!< Device has its receiver on when idle
#define MAC_CAPABLE_SECURITY        0x40  //!< Device is capable of sending and receiving secured frames
#define MAC_CAPABLE_ALLOC_ADDR      0x80  //!< Request allocation of a short address in the associate procedure

/** @} end of group mac_capability_info */

/**        
 *  @brief Mask this value with the srcAddrMode when source PAN ID is broadcast
 */
#define MAC_SRC_PAN_ID_BROADCAST    0x80


/** @addtogroup mac_special_address MAC Special Address
 *  Special address values
 * @{
 */
#define MAC_ADDR_USE_EXT            0xFFFE  //!< Short address value indicating extended address is used
#define MAC_SHORT_ADDR_BROADCAST    0xFFFF  //!< Broadcast short address
#define MAC_SHORT_ADDR_NONE         0xFFFF  //!< Short address when there is no short address

/** @} end of group mac_special_address */

/**        
 *  @brief The default reset value of MAC table
 */
#define MAC_RESET_VALUE             0xFF

/**        
 *  @brief Maximum number of pending addresses in a beacon
 */
#define MAC_PEND_ADDR_MAX               7


/**        
 *  @brief Beacon order maximum values
 */
#define MAC_BO_NON_BEACON               15

/**        
 *  @brief Superframe order maximum values
 */
#define MAC_SO_NONE                     15

/**        
 *  @brief Broadcast PAN ID
 */
#define MAC_PAN_ID_BROADCAST            0xFFFF
#define MAC_SYMBOLS_PER_OCTET           2
#define MAC_A_MAX_PHY_PACKET_SIZE       127
#define MAC_A_TURNAROUND_TIME           12
#define MAC_A_BASE_SLOT_DURATION        60

#ifdef WIN32
#define MAC_A_BASE_SUPERFRAME_DURATION  (MAC_A_BASE_SLOT_DURATION * MAC_A_NUM_SUPERFRAME_SLOTS)
#else
#define MAC_A_BASE_SUPERFRAME_DURATION  (MAC_A_BASE_SLOT_DURATION * MAC_A_NUM_SUPERFRAME_SLOTS)
#endif

#define MAC_A_MAX_BEACON_OVERHEAD       75
#define MAC_A_MAX_BEACON_PAYLOAD_LENGTH (MAC_A_MAX_PHY_PACKET_SIZE - MAC_A_MAX_BEACON_OVERHEAD)
#define MAC_A_GTS_DESC_PERSISTENCE_TIME 4
#define MAC_A_MAX_FRAME_OVERHEAD        25
#define MAC_A_MAX_LOST_BEACONS          4


#define MAC_A_MAX_FRAME_SIZE            (MAC_A_MAX_PHY_PACKET_SIZE - MAC_A_MAX_FRAME_OVERHEAD)


#define MAC_A_MAX_SIFS_FRAME_SIZE       18

#define MAC_A_MIN_CAP_LENGTH            440


#define MAC_A_MIN_LIFS_PERIOD           40


#define MAC_A_MIN_SIFS_PERIOD           12


#define MAC_A_NUM_SUPERFRAME_SLOTS      16


#define MAC_A_UNIT_BACKOFF_PERIOD       20

#define MAC_SPEC_ED_MAX                 0xFF


#define MAC_SPEC_ED_MIN_DBM_ABOVE_RECEIVER_SENSITIVITY    10




#define MAC_SPEC_MIN_RECEIVER_SENSITIVITY   -85


#define MAC_SPEC_PREAMBLE_FIELD_LENGTH      8


#define MAC_SPEC_SFD_FIELD_LENGTH           2


#define MAC_SPEC_USECS_PER_SYMBOL           16


#define MAC_SPEC_USECS_PER_BACKOFF          (MAC_SPEC_USECS_PER_SYMBOL * MAC_A_UNIT_BACKOFF_PERIOD)

/**        
 *  @brief octets (or bytes) per symbol
 */
#define MAC_SPEC_OCTETS_PER_SYMBOL          2

/**
 *  @brief MAC Security ON/OFF macro
 */
#define USE_MAC_SECURITY                    1


#define CCA_THRESHOLD       -78

/**  @} end of group MAC_Constant */

/**  @} end of group MAC_Module */

/**  @} end of group TELINK_802_15_4_STACK */


