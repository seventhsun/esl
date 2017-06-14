#include "ev.h"

void process_Phy2Mac(void);
void process_CSMA2Mac(void);
void process_Mac2Nwk(void);
void process_Nwk2Profile(void);
void process_Profile2Nwk(void);
void process_Nwk2Mac(void);
void process_Mac2CSMA(void);

ev_poll_callback_t rf_edDetect_ptr = 0;

void stack_mainloop(void)
{
    ev_process_timer();

    if (rf_edDetect_ptr) {
        rf_edDetect_ptr();
    }

    process_Phy2Mac();
    process_CSMA2Mac();
    process_Nwk2Mac();
    process_Mac2CSMA();
}

