#pragma once
#include "mac_include.h"

//for assocating device
void mac_doAssociate(u8 *pData);
void mac_associateCnfHandler(u8 *pData);
void mac_associatePollCnfHandler(u8 *pData);
void mac_associateRspHandler(u8 *pData);
//for coordinator
void mac_associateReqHandler(u8 *pData);
void mac_doAssociateRspSend(u8 *pData);
void mac_assoRspCnfHandler(u8 *pData);

void mac_doDisassociate(u8* pData);
void mac_disassociateCnfHandler(u8* pData);
void mac_disassociateIndHandler(u8* pData);
