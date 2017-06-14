#ifndef GATEWAY_INTERFACE_H
#define GATEWAY_INTERFACE_H

extern void Interface_Init(void);
extern unsigned int ResBuf_Read(unsigned char *pDestBuf, unsigned char len);
extern unsigned int ParaBuf_Write(unsigned char *pSrcBuf, unsigned char len);

#endif /*GATEWAY_INTERFACE_H*/
