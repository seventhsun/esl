/********************************************************************************************************
 * @file     timer_event.c
 *
 * @brief    This file provides set of functions to manage software TIMER
 *
 * @author   xiaodong.zong@telink-semi.com
 * @date     Oct. 8, 2016
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *           The information contained herein is confidential property of Telink 
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *           Co., Ltd. and the licensee or the terms described here-in. This heading 
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this 
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#include "../../common.h"
#include "../../drivers.h"
#include "timer_event.h"

#define LengthOfArray(arr_name) (sizeof(arr_name)/sizeof(arr_name[0]))
#define TIMER_SAFE_MARGIN     (1<<31)

#define TIMER_LIST_LEN_MAX    (10)
ev_time_event_t timer_list[TIMER_LIST_LEN_MAX] = {0};

static int ev_is_timer_expired(ev_time_event_t *e, unsigned int now)
{
    assert(e);
    return ((unsigned int)(now - e->t) < TIMER_SAFE_MARGIN);
}

static void ev_start_timer(ev_time_event_t *e)
{  
    assert(e);

    unsigned char r = IRQ_Disable();

    e->t = ClockTime() + e->interval;
    e->valid = 1;
   
    IRQ_Restore(r);
}

static void ev_cancel_timer(ev_time_event_t * e)
{
    assert(e);

    unsigned char r = IRQ_Disable();
    e->valid = 0;
    e->busy = 0;
    IRQ_Restore(r);
}

ev_time_event_t *ev_on_timer(ev_timer_callback_t cb, void *data, unsigned int t_us)
{
    int i;
    ev_time_event_t *e = NULL;

    assert(cb);

    for (i = 0; i < LengthOfArray(timer_list); i++) {
        if (timer_list[i].busy == 0) {
            timer_list[i].busy = 1;
            e = timer_list + i;
            break;
        }
    }
    if (e == NULL) {
        while(1);
    }
    e->interval = t_us * TickPerUs;
    e->cb = cb;
    e->data = data;
    ev_start_timer(e);
    return e;
}

void ev_unon_timer(ev_time_event_t ** e)
{
    assert(e && (*e));
    
    ev_cancel_timer(*e);
    *e = NULL;
}


/* Process time events */
void ev_process_timer()
{
    unsigned int now = ClockTime();
    ev_time_event_t *te;
    for (te = timer_list; te < timer_list + LengthOfArray(timer_list); te++) {
        if ((!is_timer_expired(te)) && ev_is_timer_expired(te, now)) {
            int t;
            if ( te->cb < 0x100 || te->cb > 0x20000 ) {
                while(1);
            }
            t = te->cb(te->data);
            if(t < 0){
                ev_cancel_timer(te);        // delete timer
            }
            else if(0 == t) {
                te->t = now + te->interval;    // becare of overflow
            }
            else {
                te->interval = t * TickPerUs;
                te->t = now + te->interval;    // becare of overflow
            }
        }
    }
}

int is_timer_expired(ev_time_event_t *e)
{
    if ( e == NULL ) {
        return TRUE;
    }
    if (e->valid == 1) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}
