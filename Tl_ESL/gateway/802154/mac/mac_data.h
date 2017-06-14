#pragma once

#include "mac_include.h"

#define INDIR_DATA 1
#define MAC_MAX_INDIR_DATA_NUM       3

typedef struct {
    struct mac_indirItem_t *next;
    u8 *pData;
    ev_time_event_t *timer;
    addr_t devAddr;
    u8 msduHandle;
    u8 txOptions;
} mac_indirItem_t;

void mac_dataInit(void);
void mac_dataReset(void);

void mac_doSendData(u8 *pData);
void mac_dataCnfHandler(u8 *pData);
void mac_dataIndHandler(u8 *pData);

void mac_doPoll(u8 *pData);
void mac_pollCnfHandler(u8 *pData);

mac_sts_t mac_indirectTx(u8 *pData, addr_t *pDstAddr);
void mac_dataReqHandler(u8 *pData);

void mac_doPurge(u8 *pData);

