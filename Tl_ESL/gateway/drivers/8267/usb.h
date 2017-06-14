#ifndef	_USB_H_
#define _USB_H_ 

void UsbInitDescriptor();
void UsbInit();
void UsbTask();
void WriteEndPoint(int nEnpNum, unsigned char * pData, int nDataLen);
					
#endif
