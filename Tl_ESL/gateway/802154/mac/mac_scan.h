#pragma once

#include "mac_include.h"

/* Maximum scan duration */
#define MAC_SCAN_DURATION_MAX           14

/* Initialize scan to first possible channel minus 1 */
#define MAC_SCAN_CHAN_INIT              (MAC_CHAN_11 - 1)

/* Value to determine last channel for end of scan */
#define MAC_SCAN_CHAN_END               (MAC_CHAN_28 + 1)


void mac_scanReset(void);
void mac_scanInit(void);
void mac_doScan(u8 *pData);
void mac_scanRecvBeacon(u8* pData);
