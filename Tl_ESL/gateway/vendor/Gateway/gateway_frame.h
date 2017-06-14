#ifndef GATEWAY_FRAME_H
#define GATEWAY_FRAME_H

extern int Gateway_ReadRssi(void);
extern int Gateway_SetMac(unsigned short mac);
extern int Gateway_SetPanID(unsigned short pan_id);
extern int Gateway_SetChannel(unsigned char channel);
extern int Gateway_SetPower(unsigned char power);
extern int Gateway_TxPacket(unsigned short dst_addr, unsigned short dst_pan_id, unsigned char *data, unsigned char len);

typedef void(*Gateway_ResultHandler)(unsigned char *data);

extern int Gateway_SetHandlers(Gateway_ResultHandler ReadRssiDoneHandler,
                               Gateway_ResultHandler SetMacDoneHandler,
                               Gateway_ResultHandler SetPanIDDoneHandler,
                               Gateway_ResultHandler SetChannelDoneHandler,
                               Gateway_ResultHandler SetPowerDoneHandler,
                               Gateway_ResultHandler TxPacketDoneHandler,
                               Gateway_ResultHandler RxPacketHandler);
extern int Gateway_IrqHandler(void);

#endif /*GATEWAY_FRAME_H*/