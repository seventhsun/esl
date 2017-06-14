#include "../../common.h"
#include "utility.h"
#include "ev_buffer.h"

#define LengthOfArray(arr_name) (sizeof(arr_name)/sizeof(arr_name[0]))

ev_bufItem_t ev_bufItem[BUFFER_NUM] = {0};

/*********************************************************************
 * @fn      ev_buf_reset
 *
 * @brief   Reset the EV Buffer module
 *
 * @param   None
 *
 * @return  None
 */
static void ev_buf_reset(void)
{
    int i;
    for (i = 0; i < LengthOfArray(ev_bufItem); i++) {
        memset(ev_bufItem[i].buf_item_state_tbl, 0x00, sizeof(ev_bufItem[i].buf_item_state_tbl));
    }
}

/*********************************************************************
 * @fn      ev_buf_init
 *
 * @brief   Initialize the EV Buffer module
 *
 * @param   None
 *
 * @return  None
 */
void ev_buf_init(void)
{
    ev_buf_reset();
}

/*********************************************************************
 * @fn      ev_buf_allocate
 *
 * @brief   Allocate an available buffer according to the requested size
 *          The allocated buffer will have only three kind of size, defined
 *          in @ref EV_BUFFER_CONSTANT
 *
 * @param   size - requested size
 *
 * @return  Pointer to an allocated buffer.
 *          NULL means the there is no available buffer.
 */
unsigned char *ev_buf_allocate(int size)
{
    int i, j, k;
    unsigned char r;

    r = IRQ_Disable();

    for (i = 0; i < LengthOfArray(ev_bufItem); i++) {
        if (ev_bufItem[i].buf_item_state_tbl[BUF_ITEM_STATE_BUSY] == 0) {
            ev_bufItem[i].buf_item_state_tbl[BUF_ITEM_STATE_BUSY] = 1;
            IRQ_Restore(r);
            return ev_bufItem[i].data;
        }
    }
    IRQ_Restore(r);

    return NULL;
}

/*********************************************************************
 * @fn      ev_buf_free
 *
 * @brief   Free the specified buffer
 *
 * @param   pBuf - the pointer to the specified buffer to free.
 *
 * @return  status
 */
buf_sts_t ev_buf_free(unsigned char *pBuf)
{
    int j, k;
    unsigned char r;
    ev_bufItem_t *pDelBuf;

    if (NULL == pBuf) {
        while(1);
    }

    r = IRQ_Disable();

    pDelBuf = ev_buf_getHead(pBuf);
    // CHECK_ARRAY_ELEM_POINTER(ev_bufItem,pDelBuf,step,p_value);
    pDelBuf->buf_item_state_tbl[BUF_ITEM_STATE_BUSY] = 0;
    IRQ_Restore(r);
    return BUFFER_SUCC;
}

unsigned char ev_isTaskDone(void)
{
    buf_item_state_t state;
    int i;
    for (state = BUF_ITEM_STATE_PHY2MAC; state < MAX_BUF_ITEM_STATE; state++) {
        for (i = 0; i < LengthOfArray(ev_bufItem); i++) {
            if (ev_bufItem[i].buf_item_state_tbl[state] == 1) {
                return 0;
            }
        }
    }
    return 1;
}

unsigned char buf_message_post(unsigned char *ptr, buf_item_state_t state)
{
        unsigned char r;
        ev_bufItem_t *buffer_item_ptr;

        r = IRQ_Disable();
        
        buffer_item_ptr = (ev_bufItem_t*)ev_buf_getHead(ptr);
        buffer_item_ptr->buf_item_state_tbl[state] = 1;
        IRQ_Restore(r);
        return 0;   
}

unsigned char *buf_message_poll(buf_item_state_t state)
{
    int i;
    unsigned char r;

    r = IRQ_Disable();
    for (i = 0; i < LengthOfArray(ev_bufItem); i++) {
        if (ev_bufItem[i].buf_item_state_tbl[state] == 1) {
            ev_bufItem[i].buf_item_state_tbl[state] = 0;
            IRQ_Restore(r);
            return ev_bufItem[i].data;
        }
    }
    
    IRQ_Restore(r);
    return NULL;
}

/*********************************************************************
 * @fn      ev_buf_getHead
 *
 * @brief   Get the header pointer of a buffer item
 *
 * @param   pd - the pointer of a data, which is previously allocated
 *
 * @return  Pointer of bufferItem
 */
ev_bufItem_t* ev_buf_getHead(unsigned char* pd)
{
    return (ev_bufItem_t*)(pd - OFFSETOF(ev_bufItem_t, data));
}

/*********************************************************************
 * @fn      ev_buf_getTail
 *
 * @brief   Get the pointer from a EV BUFFER tail.
 *
 * @param   pd - the pointer of a data, which is previously allocated
 * @param   offsetToTail - The offset to Tail
 *
 * @return  Pointer of the specified memory
 */
unsigned char* ev_buf_getTail(unsigned char* pd, int offsetToTail)
{
    return (unsigned char*)(pd + BUFFER_SIZE - offsetToTail);
}
