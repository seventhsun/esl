/********************************************************************************************************
 * @file     timer_event.h
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
#ifndef _TIMER_EVENT_H_
#define _TIMER_EVENT_H_

typedef int (*ev_timer_callback_t)(void *data);

/**
 *  @brief Definition for timer event
 */
typedef struct {
    ev_timer_callback_t cb; //!< Callback function when expire, this must be specified
    unsigned int t;         //!< Used internal
    unsigned int interval;  //!< Used internal
    void *data;             //!< Callback function arguments.
    unsigned int valid;
    unsigned int busy;
} ev_time_event_t;

extern ev_time_event_t *ev_on_timer(ev_timer_callback_t cb, void *data, unsigned int t_us);
extern void ev_unon_timer(ev_time_event_t **e);//ok
extern void ev_process_timer();
extern int is_timer_expired(ev_time_event_t *e);//ok

#endif /* _TIMER_EVENT_H_ */