#pragma once

typedef enum {
    MAC_STATE_IDLE,
    MAC_STATE_SCAN,
    MAC_STATE_ASSOCIATING,
    MAC_STATE_POLLING,
    MAC_STATE_STARTING,
    MAC_STATE_DEV,
    MAC_STATE_COORD,
} mac_state_t;

typedef struct {
    u8 primitive;
    u8 request[1];
} mac_generalReq_t;

typedef struct {
    u8 primitive;
    u8 status;
    u8 data[1];
} mac_generalCnf_t;

typedef void (*mac_evHandler_t)(u8* pData);

/** @brief  MAC state machine */
typedef struct {
    mac_state_t curState;             /**< The MAC State in which the event handler can be used */
    u8 primitive;                    /**< The primitive id for which the event handler is to be invoked */
    mac_state_t nxtState;             /**< The MAC State in which the event handler can be used */
    mac_evHandler_t evHandlerFunc;    /**< The corresponding event handler */
} mac_stateMachine_t;


void mac_sendConfirm(u8* pData);
void mac_sendIndication(u8* pData);
void mac_restoreState(void);
void mac_setState(mac_state_t status);
mac_state_t mac_getState(void);
void mac_setPanCoord(u8 fCoord);
u8 mac_getPanCoord(void);
void mac_getCoordAddr(addr_t *pAddr);
u8 mac_getSelfAddrMode(void);
void mac_genCommStatusInd(u8* pData, u8 status, addr_t* pSrcAddr, addr_t*pDstAddr);
