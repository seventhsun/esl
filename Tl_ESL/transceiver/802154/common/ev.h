#pragma once

typedef void (*ev_poll_callback_t)(void);

extern ev_poll_callback_t rf_edDetect_ptr;

void stack_mainloop(void);

