#pragma once

#define BUFFER_NUM                       (9)
#define SMALL_BUFFER                     (24)
#define MIDDLE_BUFFER                    (48)
#define LARGE_BUFFER                     (152)
#define BUFFER_SIZE                      (200)

enum {
    NWK_2_MAC_TYPE,
    PHY_2_MAC_TYPE,
};

typedef enum {
    BUF_ITEM_STATE_BUSY = 0,
    BUF_ITEM_STATE_PHY2MAC,
    BUF_ITEM_STATE_CSMA2MAC,
    BUF_ITEM_STATE_MAC2NWK,
    BUF_ITEM_STATE_NWK2PROFILE,
    BUF_ITEM_STATE_PROFILE2NWk,
    BUF_ITEM_STATE_NWK2MAC,
    BUF_ITEM_STATE_MAC2CSMA,
    MAX_BUF_ITEM_STATE,
} buf_item_state_t;


typedef struct {
    struct ev_bufItem_t *next;
    unsigned char buf_item_state_tbl[MAX_BUF_ITEM_STATE];
    unsigned char data[BUFFER_SIZE];
} ev_bufItem_t;

typedef enum {
    BUFFER_SUCC,                   // SUCCESS always be ZERO
    BUFFER_INVALID_PARAMETER = 1,  // Invalid parameter passed to the buffer API
    BUFFER_DUPLICATE_FREE          // The same buffer is freed more than once
} buf_sts_t;

extern void ev_buf_init(void);
extern unsigned char ev_isTaskDone(void);
extern unsigned char *ev_buf_allocate(int size);
extern buf_sts_t ev_buf_free(unsigned char *pBuf);

extern ev_bufItem_t *ev_buf_getHead(unsigned char *pd);
extern unsigned char *ev_buf_getTail(unsigned char *pd, int offsetToTail);
extern unsigned char buf_message_post(unsigned char *ptr, buf_item_state_t state);
extern unsigned char *buf_message_poll(buf_item_state_t state);


