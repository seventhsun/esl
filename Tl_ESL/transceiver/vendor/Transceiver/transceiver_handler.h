#ifndef _TRANSCEIVER_HANDLER_H_
#define _TRANSCEIVER_HANDLER_H_

extern void Transceiver_MACInit(void);
extern void Transceiver_DataIndCb(unsigned char *pData);
extern void Transceiver_Mainloop(void);

extern void SendTestTimerStart();

#endif /* _TRANSCEIVER_HANDLER_H_ */