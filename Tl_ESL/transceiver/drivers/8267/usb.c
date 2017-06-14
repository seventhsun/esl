#include "bsp.h"
#include "usb_def.h"
#include "usb.h"
//#include "../../common/types.h"
typedef unsigned char u8 ;
typedef signed char s8;

typedef unsigned short u16;
typedef signed short s16;

typedef int s32;
typedef unsigned int u32;

typedef long long s64;
typedef unsigned long long u64;

#define		NULL    0
////////////////////////////////////////////////////////////////////////
#define	  	USB_PRINTER
#define		USB_KEYBOARD
#define		USB_MOUSE
#define		USB_SPEAKER
#define		USB_MICROPHONE



////////////////////////////////////////////////////////////////////////

unsigned char irq_edp0_setup = 0, irq_edp0_data = 0, irq_edp0_status = 0,
              irq_edps = 0;
unsigned char bm_request = 0, usb_request = 0, usb_valuel = 0, usb_valueh = 0,
              usb_indexl = 0, usb_indexh = 0;

unsigned short   usb_length = 0;
unsigned short   g_length = 0;
const unsigned char * g_table = 0;
unsigned char   g_stall = 0;
unsigned char   g_protocol = 0;		//default 1 for report protocol
unsigned char   g_rate = 0;		//default 0 for all report
unsigned char   aodat = 0;

#define IF_ID_PRINTER           0x00
#define IF_ID_AUDIO_CONTROL     0x01
#define IF_ID_MIC               0x02
#define IF_ID_SPEAKER           0x03
#define IF_ID_MOUSE             0x04
#define IF_ID_KEYBOARD          0x05

#define EDP_ID_PRINTER_IN       0x08
#define EDP_ID_PRINTER_OUT      0x07
#define EDP_ID_MIC              0x07
#define EDP_ID_SPEAKER          0x06
#define EDP_ID_MOUSE            0x01
#define EDP_ID_KEYBOARD_IN      0x02
#define EDP_ID_KEYBOARD_OUT     0x03


//------------------------------------------------------------------------------------
const unsigned char DeviceDescriptor[] = 
{
  //Device:USB1.10,Vid=0x0123,Pid=0x4567,bNumConfigurations = 0x01,
  0x12,      //Length
  0x01,      //DescriptorType
  0x10,0x01, //bcdUSB
  0x00,      //DeviceClass
  0x00,      //DeviceSubClass
  0x00,      //DeviceProtocol
  0x08,      //bMaxPacketSize 8
  0x8a,0x24, //idVendor........
  0x67,0x45, //idProduct........
  0x01,0x00, //bcdDevice
  1,         //iManufacturer
  2,         //iProduct
  3,         //iSerialNumber
  0x01       //bNumConfigurations
};

#define CONFIG_DESCRIPTOR  0x09, /* Length */ \
0x02,      /* DescriptorType : ConfigDescriptor */    \
0xfa,0x00, /* TotalLength: variable */  \
0x06,      /* NumInterfaces: variable */  \
0x01,      /* ConfigurationValue */  \
0x00,      /* Configuration String */  \
0x80,      /* Attributes:Bus Power */  \
0xfa       /* MaxPower = 0xfa*2ma */

#define PRINTER_STD_IF   0x09, /* Length */ \
0x04,  /* bDescriptorType */ \
0x00, /*IF_ID_PRINTER,*/  /* bInterfaceNumber */ \
0x00,  /* bAlternateSetting */ \
0x02,  /* bNumEndpoints */ \
0x07,  /* bInterfaceclass ->Printer */ \
0x01,  /* bInterfaceSubClass -> Control */ \
0x02,  /* bInterfaceProtocol maybe 0x03 */ \
0x00   /* iInterface */

#define PRINTER_IN_ENDPOINT 0x07, /* length */ \
0x05, /* bDescriptorType */ \
0x88, /* 0x80 | EDP_ID_PRINTER_IN,*/ /* bEndpointAddress -> Direction: IN - EndpointID: 8 */ \
0x02, /* bmAttributes  -> Transfer Type: Bulk */ \
0x40, 0x00, /* wMaxPacketSize */ \
0x00  /* bInterval */

#define PRINTER_OUT_ENDPOINT 0x07, /* length */ \
0x05, /* bDescriptorType */ \
0x07, /* EDP_ID_PRINTER_OUT,*/ /* bEndpointAddress -> Direction: OUT - EndpointID: 7 */ \
0x02, /* bmAttributes  -> Bulk  Transfer Typ */ \
0x40, 0x00, /* wMaxPacketSize */ \
0x00 /* bInterval */

/*Audio Control Interface Descriptors*/
/*Standard Audio Control Interface Descriptors*/
#define AUDIO_CONTROL_STD_IF 0x09, /* Length */ \
0x04, /* DescriptorType:Interface */ \
IF_ID_AUDIO_CONTROL, /* InterfaceNum:1 */ \
0x00, /* AlternateSetting:0 */ \
0x00, /* NumEndpoint:0 */ \
0x01, /* InterfaceClass:audio */ \
0x01, /* InterfaceSubClass:audio control */ \
0x00, /* InterfaceProtocol */ \
0x00  /* Interface String */

/*Class-specific Audio Control Interface Descriptors*/
/*1. Header*/
#if defined (USB_MICROPHONE) && defined (USB_SPEAKER)
#define  AUDIO_CONTROL_HEADER  0x0a, /* Length */ \
0x24, /* DescriptorType:audio interface descriptor */ \
0x01, /* DescriptorSubType:audio control header */ \
0x00,0x01, /* bcdADC:audio Device Class v1.00 */ \
0x34,0x00, /* TotalLength: variable */ \
0x02, /* InCollection:2 AudioStreaming interface */ \
0x02, /* Microphone AS id: 2 */ \
0x03  /* Speaker AS id: 3 */
#elif defined (USB_MICROPHONE)
#define  AUDIO_CONTROL_HEADER  0x09, /* Length */ \
0x24, /* DescriptorType:audio interface descriptor */ \
0x01, /* DescriptorSubType:audio control header */ \
0x00,0x01, /* bcdADC:audio Device Class v1.00 */ \
0x1e,0x00, /* TotalLength: variable */ \
0x01, /* InCollection:1 AudioStreaming interface */ \
0x02  /* Microphone AS id: 2 */ 
#elif defined (USB_SPEAKER)
#define  AUDIO_CONTROL_HEADER  0x09, /* Length */ \
0x24, /* DescriptorType:audio interface descriptor */ \
0x01, /* DescriptorSubType:audio control header */ \
0x00,0x01, /* bcdADC:audio Device Class v1.00 */ \
0x1e,0x00, /* TotalLength: variable */ \
0x01, /* InCollection:1 AudioStreaming interface */ \
0x03  /* Speaker AS id: 3 */
#endif

/*2. Terminal or Unit*/
#define MIC_INPUT_TERMINAL 0x0c, /* Length */ \
0x24,  /* DescriptorType:audio interface descriptor */ \
0x02,  /* DescriptorSubType:Input Terminal */ \
0x01,  /* TerminalID:0x01 */ \
0x01,0x02, /* TerminalType:USB Microphone */ \
0x00,  /* AssocTerminal */ \
0x01,  /* NrChannels:mono 1 channel */ \
0x00,0x00, /* ChannelConfig:Left Front,Right Front */ \
0x00,  /* ChannelName String */ \
0x00   /* Terminal String */

//USB Streaming OT:audio interface descriptor,audio control output terminal(0x03),terminal id 0x03,
//USB Streaming(0x0101),Output Terminal(0x03),SourceId 0x02,
#define MIC_OUTPUT_TERMINAL 0x09, /* Length */ \
0x24,  /* DescriptorType:audio interface descriptor */ \
0x03,  /* DescriptorSubTYpe:Output Terminal */ \
0x03,  /* TerminalID:0x03 */ \
0x01,0x01, /* TerminalType:USB Streaming */ \
0x00,  /* AssocTerminal:ID 0 */ \
0x01,  /* SourceID:1 from input terminal */ \
0x00   /* Terminal String */

#define SPEAKER_INPUT_TERMINAL 0x0c, /* Length */ \
0x24,  /* DescriptorType:audio interface descriptor */ \
0x02,  /* DescriptorSubType:Input Terminal */ \
0x04,  /* TerminalID:0x04 */ \
0x01,0x01, /* TerminalType:USB Streaming */ \
0x00,  /* AssocTerminal */ \
0x01,  /* NrChannels:2 channel */ \
0x00,0x00, /* ChannelConfig:Left Front,Right Front */ \
0x00,  /* ChannelName String */ \
0x00   /* Terminal String */

#define SPEAKER_OUTPUT_TERMINAL 0x09, /* Length */ \
0x24,  /* DescriptorType:audio interface descriptor */ \
0x03,  /* DescriptorSubTYpe:Output Terminal */ \
0x06,  /* TerminalID:0x06 */ \
0x01,0x03, /* TerminalType:Speaker */ \
0x00,  /* AssocTerminal: 0 */ \
0x04,  /* SourceID:4  SPEAKER_INPUT_TERMINAL */ \
0x00   /* Terminal String */

/*Audio Control Endpoint Descriptors*/
/*Standard Audio Control Endpoint Descriptors*/
/*Endpoint 0 is taken as Standard Audio Control Endpoint*/

/*Class-specific Audio Control Endpoint Descriptors*/
/*None*/

/*Microphone AudioStreaming Interface Descriptors*/
/*Standard AS Interface Descriptors*/
/*1. Zero-bandwidth Alternate Setting 0*/
#define MIC_STD_IF_SET0 0x09, /* Length */ \
0x04,  /* DescriptorType:Interface */ \
IF_ID_MIC,  /* InterfaceNum(id): 2 */ \
0x00,  /* AlternateSetting:0 */ \
0x00,  /* NumEndpoint:0 */ \
0x01,  /* InterfaceClass:audio */ \
0x02,  /* InterfaceSubClass:audio streaming */ \
0x00,  /* InterfaceProtocol */ \
0x00   /* Interface String */

/*2. Operational Alternate Setting 1*/
#define MIC_STD_IF_SET1 0x09, /* Length */ \
0x04, /* DescriptorType:Interface */ \
IF_ID_MIC, /* InterfaceNum(id): 2 */ \
0x01, /* AlternateSetting:1 */ \
0x01, /* NumEndpoint:1 */ \
0x01, /* InterfaceClass:audio */ \
0x02, /* InterfaceSubClass:audio streaming */ \
0x00, /* InterfaceProtocol */ \
0x00  /* Interface String */

/*Class-specific AS Interface Descriptors*/
/*1. AS_General Interface Descriptors*/
#define MIC_AS_GENERAL 0x07,  /* Length */ \
0x24,   /* DescriptorType:audio interface descriptor */ \
0x01,   /* DescriptorSubType:AS_GENERAL */ \
0x03,   /* TerminalLink:#3USB USB Streaming OT */ \
0x01,   /* Delay:1 */ \
0x01,0x00   /* FormatTag:PCM */

/*2. Format_Type Interface Descriptors*/
#define MIC_AS_FORMAT_TYPE 0x0b,  /* Length */ \
0x24,  /* DescriptorType:audio interface descriptor */ \
0x02,  /* DescriptorSubType:Format_type */ \
0x01,  /* FormatType:Format type 1 */ \
0x01,  /* NumberOfChanne:1 */ \
0x02,  /* SubframeSize:2byte */ \
0x10,  /* BitsResolution:16bit */ \
0x01,  /* SampleFreqType:One sampling frequency. */ \
/* 0x40,0x1f,0x00  //8k */ \
/* 0x80,0x3e,0x00  //16k */ \
/* 0x22,0x56,0x00  //22.05k */ \
/* 0x00,0x7d,0x00  // 32k */ \
/* 0x44,0xac,0x00  //44k */ \
0x80,0x3e,0x00//16K// 0x80,0xbb,0x00   // 48k 

/*Standard AS Isochronous Audio Data Endpoint Descriptors*/
#define MIC_ENDPOINT  0x07, /* Length */ \
0x05,  /* DescriptorType:endpoint descriptor */ \
0x80 | EDP_ID_MIC,  /* EndpointAddress:Input endpoint 7 */ \
0x05,  /* Attributes:0x05,Isochronous,Synchronization Type(Asynchronous).. */ \
0x00,0x01, /* MaxPacketSize: 256 */ \
0x01   /* Interval */

/*Class-specific AS Isochronous Audio Data Endpoint Descriptors*/
#define MIC_AUDIO_ENDPOINT  0x07,  /* Length */ \
0x25,  /* DescriptorType:audio endpoint descriptor */ \
0x01,  /* DescriptorSubType:audio endpiont general */ \
0x00,  /* Attributes:0x00........ */ \
0x00,  /* LockDelayUnits */ \
0x00,0x00  /* LockDelay */

/*Speaker AudioStreaming Interface Descriptors*/
/*Standard AS Interface Descriptors*/
/*1. Zero-bandwidth Alternate Setting 0*/
#define SPEAKRE_STD_IF_SET0  0x09, /* Length */ \
0x04,  /* DescriptorType:Interface */ \
IF_ID_SPEAKER,  /* InterfaceNum(id): 3 */ \
0x00,  /* AlternateSetting:0 */ \
0x00,  /* NumEndpoint:0 */ \
0x01,  /* InterfaceClass:audio */ \
0x02,  /* InterfaceSubClass:audio streaming */ \
0x00,  /* InterfaceProtocol */ \
0x00   /* Interface String */

/*2. Operational Alternate Setting 1*/
#define SPEAKER_STD_IF_SET1  0x09, /* Length */ \
0x04,  /* DescriptorType:Interface */ \
IF_ID_SPEAKER,  /* InterfaceNum(id): 3 */ \
0x01,  /* AlternateSetting:1 */ \
0x01,  /* NumEndpoint:1 */ \
0x01,  /* InterfaceClass:audio */ \
0x02,  /* InterfaceSubClass:audio streaming */ \
0x00,  /* InterfaceProtocol */ \
0x00   /* Interface String */

/*Class-specific AS Interface Descriptors*/
/*1. AS_General Interface Descriptors*/
#define SPEAKER_AS_GENERAL  0x07,  /* Length */ \
0x24,  /* DescriptorType:audio interface descriptor */ \
0x01,  /* DescriptorSubType:AS_GENERAL */ \
0x04,  /* TerminalLink:#4 USB Streaming IT */ \
0x01,  /* Delay:1 */ \
0x01,0x00  /* FormatTag:PCM */

/*2. Format_Type Interface Descriptors*/
#define SPEAKER_AS_FORMAT_TYPE    0x0b,  /* Length */ \
0x24,  /* DescriptorType:audio interface descriptor */ \
0x02,  /* DescriptorSubType:Format_type */ \
0x01,  /* FormatType:Format type 1 */ \
0x01,  /* NumberOfChanne:1 */ \
0x02,  /* SubframeSize:2byte */ \
0x10,  /* BitsResolution:16bit */ \
0x01,  /* SampleFreqType:One sampling frequency. */ \
/* 0x40,0x1f,0x00  //8k */ \
/* 0x80,0x3e,0x00  //16k */ \
/* 0x22,0x56,0x00, //22.05k */ \
/* 0x00,0x7d,0x00  // 32k */ \
/* 0x44,0xac,0x00, //44.1k */ \
0x80,0x3e,0x00//16K //0x80,0xbb,0x00   //48k 

/*Standard AS Isochronous Audio Data Endpoint Descriptors*/
#define SPEAKER_ENDPOINT    0x07,   /* Length */ \
0x05,  /* DescriptorType:endpoint descriptor */ \
EDP_ID_SPEAKER,  /* EndpointAddress */ \
0x09,  /* Attributes:0x05,Isochronous,Synchronization Type(Asynchronous).... */ \
0x00,0x01,  /* MaxPacketSize: 256 */ \
0x01   /* Interval */

/*Class-specific AS Isochronous Audio Data Endpoint Descriptors*/
#define SPEAKER_AUDIO_ENDPOINT    0x07,   /* Length */ \
0x25,  /* DescriptorType:audio endpoint descriptor */ \
0x01,  /* DescriptorSubType:audio endpiont general */ \
0x00,  /* Attributes:0x00............. */ \
0x00,  /* LockDelayUnits */ \
0x00,0x00  /* LockDelay */

#define MOUSE_STD_IF 0x09, /* length */ \
0x04, /* bDescriptorType字段。接口描述符的编号为0x04 */ \
IF_ID_MOUSE, /* bInterfaceNumber(id): 4 */ \
0x00, /* bAlternateSetting字段。该接口的备用编号，为0 */ \
0x01, /* bNumEndpoints字段。非0端点的数目。由于USB鼠标只需要一个中断输入端点，因此该值为1 */ \
0x03, /* bInterfaceClass字段。该接口所使用的类。USB鼠标是HID类，HID类的编码为0x03 */ \
0x01, /* bInterfaceSubClass字段。该接口所使用的子类。USB键盘、鼠标属于该子类，子类代码为0x01。*/ \
0x02, /* bInterfaceProtocol字段。键盘代码为0x01，鼠标代码为0x02。*/ \
0x00  /* iConfiguration字段。该接口的字符串索引值。这里没有，为0 */

#define MOUSE_HID  0x09, /* length */ \
0x21, /* bDescriptorType字段。HID描述符的编号为0x21。*/ \
0x10, 0x01, /* bcdHID字段。本协议使用的HID1.1协议。注意低字节在先 */ \
0x21, /* bCountyCode字段。设备适用的国家代码，这里选择为美国，代码0x21 */ \
0x01, /* bNumDescriptors字段。下级描述符的数目。我们只有一个报告描述符 */ \
0x22, /* bDescriptorType字段。下级描述符的类型，为报告描述符，编号为0x22 */ \
0x34, 0x00 /* bDescriptorLength字段。下级描述符的长度。下级描述符为报告描述符 */

#define MOUSE_ENDPOINT  0x07, /* length */ \
0x05, /* bDescriptorType字段。端点描述符编号为0x05 */ \
0x80 | EDP_ID_MOUSE, /* bEndpointAddress字段。端点的地址。D7位表示数据方向，D3-D0表示端点号 */ \
0x03, /* bmAttributes字段。D1~D0为端点传输类型选择。该端点为中断端点。中断端点的编号为3。其它位保留为0。*/ \
0x10, 0x00, /* wMaxPacketSize字段。该端点的最大包长。端点1的最大包长为16字节。注意低字节在先。*/ \
0x0A  /* bInterval字段。端点查询的时间，我们设置为10个帧时间，即10ms */

#define KEYBOARD_STD_IF  0x09, /* length */ \
0x04,  /* bDescriptorType字段。接口描述符的编号为0x04 */ \
IF_ID_KEYBOARD,  /* bInterfaceNumber(id): 5 */ \
0x00,  /* bAlternateSetting字段。该接口的备用编号，为0。*/ \
0x02,  /* bNumEndpoints字段。非0端点的数目。由于USB键盘需要二个, 中断端点（一个输入一个输出），因此该值为2 */ \
0x03,  /* bInterfaceClass字段。该接口所使用的类。USB键盘是HID类，HID类的编码为0x03。*/ \
0x01,  /* bInterfaceSubClass字段。只规定了一种子类：支持BIOS引导启动的子类。USB键盘、鼠标属于该子类，子类代码为0x01。*/ \
0x01,  /* bInterfaceProtocol字段。键盘代码为0x01，鼠标代码为0x02。*/ \
0x00   /* iConfiguration字段。该接口的字符串索引值。这里没有，为0。*/

#define KEYBOARD_HID  0x09,  /* length */ \
0x21,  /* bDescriptorType字段。HID描述符的编号为0x21 */ \
0x10, 0x01,  /* bcdHID字段。本协议使用的HID1.1协议。注意低字节在先。*/ \
0x21,  /* bCountyCode字段。设备适用的国家代码，这里选择为美国，代码0x21。*/ \
0x01,  /* bNumDescriptors字段。下级描述符的数目。我们只有一个报告描述符。*/ \
0x22,  /* bDescriptorType字段。下级描述符的类型，为报告描述符，编号为0x22。*/ \
0x41, 0x00 /* bDescriptorLength字段。下级描述符的长度。下级描述符为键盘报告描述符。*/

#define KEYBOARD_ENDPOINT_IN  0x07, /* length */ \
0x05,  /* bDescriptorType字段。端点描述符编号为0x05。*/ \
0x80 | EDP_ID_KEYBOARD_IN,  /* bEndpointAddress字段。端点的地址。D7位表示数据方向，D3-D0表示端点号*/ \
0x03,  /* bmAttributes字段。D1~D0为端点传输类型选择。该端点为中断端点。中断端点的编号为3。其它位保留为0。*/ \
0x10, 0x00,  /* wMaxPacketSize字段。该端点的最大包长。端点2的最大包长为16字节。注意低字节在先。*/ \
0x0A   /* bInterval字段。端点查询的时间，我们设置为10个帧时间，即10ms。*/

#define KEYBOARD_ENDPOINT_OUT  0x07,  /* length */ \
0x05,  /* bDescriptorType字段。端点描述符编号为0x05。*/ \
EDP_ID_KEYBOARD_OUT,  /* bEndpointAddress字段。端点的地址。D7位表示数据方向，D3-D0表示端点号*/ \
0x03,  /* bmAttributes字段。D1~D0为端点传输类型选择。该端点为中断端点。中断端点的编号为3。其它位保留为0。*/ \
0x10, 0x00,  /* wMaxPacketSize字段。该端点的最大包长。端点1的最大包长为16字节。注意低字节在先。*/ \
0x0A   /* bInterval字段。端点查询的时间，我们设置为10个帧时间，即10ms。*/


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
unsigned char ConfigDescriptor[] = 
{
  CONFIG_DESCRIPTOR,
#ifdef USB_PRINTER
  PRINTER_STD_IF,
  PRINTER_IN_ENDPOINT,
  PRINTER_OUT_ENDPOINT,
#endif
#if defined (USB_MICROPHONE) || defined (USB_SPEAKER)
  AUDIO_CONTROL_STD_IF,
  AUDIO_CONTROL_HEADER,
#endif
#ifdef USB_MICROPHONE 
  MIC_INPUT_TERMINAL,
  MIC_OUTPUT_TERMINAL,
#endif
#ifdef USB_SPEAKER
  SPEAKER_INPUT_TERMINAL,
  SPEAKER_OUTPUT_TERMINAL,
#endif
#ifdef USB_MICROPHONE
  MIC_STD_IF_SET0,
  MIC_STD_IF_SET1,
  MIC_AS_GENERAL,
  MIC_AS_FORMAT_TYPE,
  MIC_ENDPOINT,
  MIC_AUDIO_ENDPOINT,
#endif
#ifdef USB_SPEAKER
  SPEAKRE_STD_IF_SET0,
  SPEAKER_STD_IF_SET1,
  SPEAKER_AS_GENERAL,
  SPEAKER_AS_FORMAT_TYPE,
  SPEAKER_ENDPOINT,
  SPEAKER_AUDIO_ENDPOINT,
#endif
#ifdef USB_MOUSE
  MOUSE_STD_IF,
  MOUSE_HID,
  MOUSE_ENDPOINT,
#endif
#ifdef USB_KEYBOARD
  KEYBOARD_STD_IF,
  KEYBOARD_HID,
  KEYBOARD_ENDPOINT_IN,
  KEYBOARD_ENDPOINT_OUT,
#endif
};

/* USB String Descriptor (optional) */
const unsigned char StringLangID[] = 
{
  0x04,
  0x03,
  0x09,
  0x04
};

const unsigned char StringVendor[] = 
{
  0x26, //Length
  0x03, //DescriptorType
  'D', 0, 'e', 0, 'm', 0, 'o', 0, '-', 0, 's', 0, 'p', 0, 'e', 0,
  'r', 0, 'k', 0, 'e', 0, 'r', 0, 'p', 0, 'h', 0, 'o', 0, 'n', 0,
  'e', 0, '1', 0
};

const unsigned char StringProduct[] = 
{
  0x1c,  //Length
  0x03,  //DescriptorType
  'T', 0, 'e', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0,
  'A', 0, 'u', 0, 'd', 0, 'i', 0, 'o', 0, '1', 0, '6', 0
};


const unsigned char StringSerial[] = 
{
  0x1c,  //Length
  0x03,  //DescriptorType
  'T', 0, 'e', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0,
  'A', 0, 'u', 0, 'd', 0, 'i', 0, 'o', 0, '1', 0, '6', 0
};

#ifdef USB_MOUSE
//鼠标的HID描述符，和MOUSE_HID宏的内容保持一致
const unsigned char MouseHidDesc[] = 
{
	0x09, 0x10, 0x01, 0x21, 0x01, 0x22, 0x34, 0x00
};

//USB报告描述符的定义
const unsigned char MouseReportDesc[] = 
{
 //每行开始的第一字节为该条目的前缀，前缀的格式为：
 //D7~D4：bTag。D3~D2：bType；D1~D0：bSize。以下分别对每个条目注释。
 
 //这是一个全局（bType为1）条目，选择用途页为普通桌面Generic Desktop Page(0x01)
 //后面跟一字节数据（bSize为1），后面的字节数就不注释了，
 //自己根据bSize来判断。
 0x05, 0x01, // USAGE_PAGE (Generic Desktop)
 
 //这是一个局部（bType为2）条目，说明接下来的应用集合用途用于鼠标
 0x09, 0x02, // USAGE (Mouse)
 
 //这是一个主条目（bType为0）条目，开集合，后面跟的数据0x01表示
 //该集合是一个应用集合。它的性质在前面由用途页和用途定义为
 //普通桌面用的鼠标。
 0xa1, 0x01, // COLLECTION (Application)
 
 //这是一个局部条目。说明用途为指针集合
 0x09, 0x01, //   USAGE (Pointer)
 
 //这是一个主条目，开集合，后面跟的数据0x00表示该集合是一个
 //物理集合，用途由前面的局部条目定义为指针集合。
 0xa1, 0x00, //   COLLECTION (Physical)
 
 //这是一个全局条目，选择用途页为按键（Button Page(0x09)）
 0x05, 0x09, //     USAGE_PAGE (Button)
 
 //这是一个局部条目，说明用途的最小值为1。实际上是鼠标左键。
 0x19, 0x01, //     USAGE_MINIMUM (Button 1)
 
 //这是一个局部条目，说明用途的最大值为3。实际上是鼠标中键。
 0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
 
 //这是一个全局条目，说明返回的数据的逻辑值（就是我们返回的数据域的值啦）
 //最小为0。因为我们这里用Bit来表示一个数据域，因此最小为0，最大为1。
 0x15, 0x00, //     LOGICAL_MINIMUM (0)
 
 //这是一个全局条目，说明逻辑值最大为1。
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)
 
 //这是一个全局条目，说明数据域的数量为三个。
 0x95, 0x03, //     REPORT_COUNT (3)
 
 //这是一个全局条目，说明每个数据域的长度为1个bit。
 0x75, 0x01, //     REPORT_SIZE (1)
 
 //这是一个主条目，说明有3个长度为1bit的数据域（数量和长度
 //由前面的两个全局条目所定义）用来做为输入，
 //属性为：Data,Var,Abs。Data表示这些数据可以变动，Var表示
 //这些数据域是独立的，每个域表示一个意思。Abs表示绝对值。
 //这样定义的结果就是，第一个数据域bit0表示按键1（左键）是否按下，
 //第二个数据域bit1表示按键2（右键）是否按下，第三个数据域bit2表示
 //按键3（中键）是否按下。
 0x81, 0x02, //     INPUT (Data,Var,Abs)
 
 //这是一个全局条目，说明数据域数量为1个
 0x95, 0x01, //     REPORT_COUNT (1)
 
 //这是一个全局条目，说明每个数据域的长度为5bit。
 0x75, 0x05, //     REPORT_SIZE (5)
 
 //这是一个主条目，输入用，由前面两个全局条目可知，长度为5bit，
 //数量为1个。它的属性为常量（即返回的数据一直是0）。
 //这个只是为了凑齐一个字节（前面用了3个bit）而填充的一些数据
 //而已，所以它是没有实际用途的。
 0x81, 0x03, //     INPUT (Cnst,Var,Abs)
 
 //这是一个全局条目，选择用途页为普通桌面Generic Desktop Page(0x01)
 0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
 
 //这是一个局部条目，说明用途为X轴
 0x09, 0x30, //     USAGE (X)
 
 //这是一个局部条目，说明用途为Y轴
 0x09, 0x31, //     USAGE (Y)
 
 //这是一个局部条目，说明用途为滚轮
 0x09, 0x38, //     USAGE (Wheel)
 
 //下面两个为全局条目，说明返回的逻辑最小和最大值。
 //因为鼠标指针移动时，通常是用相对值来表示的，
 //相对值的意思就是，当指针移动时，只发送移动量。
 //往右移动时，X值为正；往下移动时，Y值为正。
 //对于滚轮，当滚轮往上滚时，值为正。
 0x15, 0x81, //     LOGICAL_MINIMUM (-127)
 0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
 
 //这是一个全局条目，说明数据域的长度为8bit。
 0x75, 0x08, //     REPORT_SIZE (8)
 
 //这是一个全局条目，说明数据域的个数为3个。
 0x95, 0x03, //     REPORT_COUNT (3)
 
 //这是一个主条目。它说明这三个8bit的数据域是输入用的，
 //属性为：Data,Var,Rel。Data说明数据是可以变的，Var说明
 //这些数据域是独立的，即第一个8bit表示X轴，第二个8bit表示
 //Y轴，第三个8bit表示滚轮。Rel表示这些值是相对值。
 0x81, 0x06, //     INPUT (Data,Var,Rel)
 
 //下面这两个主条目用来关闭前面的集合用。
 //我们开了两个集合，所以要关两次。bSize为0，所以后面没数据。
 0xc0,       //   END_COLLECTION
 0xc0        // END_COLLECTION
};
#endif 

#ifdef USB_KEYBOARD
//键盘的HID描述符，和KEYBOARD_HID宏的内容保持一致
const unsigned char KeyboardHidDesc[] = 
{
	0x09, 0x21, 0x10, 0x01, 0x21, 0x01, 0x22, 0x41, 0x00 
};


//USB键盘报告描述符的定义
const unsigned char KeyboardReportDesc[] = 
{
 //每行开始的第一字节为该条目的前缀，前缀的格式为：
 //D7~D4：bTag。D3~D2：bType；D1~D0：bSize。以下分别对每个条目注释。
 
 //这是一个全局（bType为1）条目，将用途页选择为普通桌面Generic Desktop Page(0x01)
 //后面跟一字节数据（bSize为1），后面的字节数就不注释了，
 //自己根据bSize来判断。
 0x05, 0x01, // USAGE_PAGE (Generic Desktop)
 
 //这是一个局部（bType为2）条目，说明接下来的集合用途用于键盘
 0x09, 0x06, // USAGE (Keyboard)
 
 //这是一个主条目（bType为0）条目，开集合，后面跟的数据0x01表示
 //该集合是一个应用集合。它的性质在前面由用途页和用途定义为
 //普通桌面用的键盘。
 0xa1, 0x01, // COLLECTION (Application)
 
 //报告ID，这里定义键盘报告的ID为1（报告ID 0是保留的）
 //为了减少程序的修改量，这里依然定义一个报告ID。
 //0x85, 0x01, //Report ID (1)
 
 //这是一个全局条目，选择用途页为键盘（Keyboard/Keypad(0x07)）
 0x05, 0x07, //     USAGE_PAGE (Keyboard/Keypad)

 //这是一个局部条目，说明用途的最小值为0xe0。实际上是键盘左Ctrl键。
 //具体的用途值可在HID用途表中查看。
 0x19, 0xe0, //     USAGE_MINIMUM (Keyboard LeftControl)
 
 //这是一个局部条目，说明用途的最大值为0xe7。实际上是键盘右GUI键。
 0x29, 0xe7, //     USAGE_MAXIMUM (Keyboard Right GUI)
 
 //这是一个全局条目，说明返回的数据的逻辑值（就是我们返回的数据域的值）
 //最小为0。因为我们这里用Bit来表示一个数据域，因此最小为0，最大为1。
 0x15, 0x00, //     LOGICAL_MINIMUM (0)
 
 //这是一个全局条目，说明逻辑值最大为1。
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)
 
 //这是一个全局条目，说明数据域的数量为八个。
 0x95, 0x08, //     REPORT_COUNT (8)
 
 //这是一个全局条目，说明每个数据域的长度为1个bit。
 0x75, 0x01, //     REPORT_SIZE (1)
 
 //这是一个主条目，说明有8个长度为1bit的数据域（数量和长度
 //由前面的两个全局条目所定义）用来做为输入，
 //属性为：Data,Var,Abs。Data表示这些数据可以变动，Var表示
 //这些数据域是独立的，每个域表示一个意思。Abs表示绝对值。
 //这样定义的结果就是，当某个域的值为1时，就表示对应的键按下。
 //bit0就对应着用途最小值0xe0，bit7对应着用途最大值0xe7。
 0x81, 0x02, //     INPUT (Data,Var,Abs)
 
 //这是一个全局条目，说明数据域数量为1个
 0x95, 0x01, //     REPORT_COUNT (1)
 
 //这是一个全局条目，说明每个数据域的长度为8bit。
 0x75, 0x08, //     REPORT_SIZE (8)
 
 //这是一个主条目，输入用，由前面两个全局条目可知，长度为8bit，
 //数量为1个。它的属性为常量（即返回的数据一直是0）。
 //该字节是保留字节（保留给OEM使用）。
 0x81, 0x03, //     INPUT (Cnst,Var,Abs)
 
 //这是一个全局条目。定义位域数量为6个。
 0x95, 0x06, //   REPORT_COUNT (6)
 
 //这是一个全局条目。定义每个位域长度为8bit。
 //其实这里这个条目不要也是可以的，因为在前面已经有一个定义
 //长度为8bit的全局条目了。
 0x75, 0x08, //   REPORT_SIZE (8)
 
 //这是一个全局条目，定义逻辑最小值为0。
 //同上，这里这个全局条目也是可以不要的，因为前面已经有一个
 //定义逻辑最小值为0的全局条目了。
 0x15, 0x00, //   LOGICAL_MINIMUM (0)
 
 //这是一个全局条目，定义逻辑最大值为255。
 0x25, 0xFF, //   LOGICAL_MAXIMUM (255)
 
 //这是一个全局条目，选择用途页为键盘。
 //前面已经选择过用途页为键盘了，所以该条目不要也可以。
 0x05, 0x07, //   USAGE_PAGE (Keyboard/Keypad)
 
 //这是一个局部条目，定义用途最小值为0（0表示没有键按下）
 0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))
 
 //这是一个局部条目，定义用途最大值为0x65
 0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)
 
 //这是一个主条目。它说明这六个8bit的数据域是输入用的，
 //属性为：Data,Ary,Abs。Data说明数据是可以变的，Ary说明
 //这些数据域是一个数组，即每个8bit都可以表示某个键值，
 //如果按下的键太多（例如超过这里定义的长度或者键盘本身无法
 //扫描出按键情况时），则这些数据返回全1（二进制），表示按键无效。
 //Abs表示这些值是绝对值。
 0x81, 0x00, //     INPUT (Data,Ary,Abs)

 //以下为输出报告的描述
 //逻辑最小值前面已经有定义为0了，这里可以省略。 
 //这是一个全局条目，说明逻辑值最大为1。
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)
 
 //这是一个全局条目，说明数据域数量为5个。 
 0x95, 0x05, //   REPORT_COUNT (5)
 
 //这是一个全局条目，说明数据域的长度为1bit。
 0x75, 0x01, //   REPORT_SIZE (1)
 
 //这是一个全局条目，说明使用的用途页为指示灯（LED）
 0x05, 0x08, //   USAGE_PAGE (LEDs)
 
 //这是一个局部条目，说明用途最小值为数字键盘灯。
 0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
 
 //这是一个局部条目，说明用途最大值为Kana灯。
 0x29, 0x05, //   USAGE_MAXIMUM (Kana)
 
 //这是一个主条目。定义输出数据，即前面定义的5个LED。
 0x91, 0x02, //   OUTPUT (Data,Var,Abs)
 
 //这是一个全局条目。定义位域数量为1个。
 0x95, 0x01, //   REPORT_COUNT (1)
 
 //这是一个全局条目。定义位域长度为3bit。
 0x75, 0x03, //   REPORT_SIZE (3)
 
 //这是一个主条目，定义输出常量，前面用了5bit，所以这里需要
 //3个bit来凑成一字节。
 0x91, 0x03, //   OUTPUT (Cnst,Var,Abs)
 
 //下面这个主条目用来关闭前面的集合。bSize为0，所以后面没数据。
 0xc0,        // END_COLLECTION
//以下注释不包括第一字节报告ID。
//通过上面的报告描述符的定义，我们知道返回的输入报告具有8字节。
//第一字节的8个bit用来表示特殊键是否按下（例如Shift、Alt等键）。
//第二字节为保留值，值为常量0。第三到第八字节是一个普通键键值的
//数组，当没有键按下时，全部6个字节值都为0。当只有一个普通键按下时，
//这六个字节中的第一字节值即为该按键的键值（具体的键值请看HID的
//用途表文档），当有多个普通键同时按下时，则同时返回这些键的键值。
//如果按下的键太多，则这六个字节都为0xFF（不能返回0x00，这样会让
//操作系统认为所有键都已经释放）。至于键值在数组中的先后顺序是
//无所谓的，操作系统会负责检查是否有新键按下。我们应该在中断端点1
//中按照上面的格式返回实际的键盘数据。另外，报告中还定义了一个字节
//的输出报告，是用来控制LED情况的。只使用了低7位，高1位是保留值0。
//当某位的值为1时，则表示对应的LED要点亮。操作系统会负责同步各个
//键盘之间的LED，例如你有两块键盘，一块的数字键盘灯亮时，另一块
//也会跟着亮。键盘本身不需要判断各种LED应该何时亮，它只是等待主机
//发送报告给它，然后根据报告值来点亮相应的LED。我们在端点1输出中断
//中读出这1字节的输出报告，然后对它取反（因为学习板上的LED是低电平时
//亮），直接发送到LED上。这样main函数中按键点亮LED的代码就不需要了。
};
#endif

void UsbInitDescriptor()
{
	u16 wConfigLen = 0;
	u16 *pConfigLen = NULL;
	u8 byIntfNum = 0;
	
	u16 wACDescOffset = 0x09;  //sizeof CONFIG_DESCRIPTOR
	u16 wAudioCtrlLen = 0;
	u16 *pAudioCtrlLen = NULL;

	wConfigLen = sizeof(ConfigDescriptor);
	pConfigLen = (u16*)(ConfigDescriptor + 2);
	*pConfigLen = wConfigLen;
	
#ifdef USB_PRINTER
	byIntfNum += 1;
	wACDescOffset += 0x09 + 0x07 + 0x07;  //sizeof printer interface and endpoint descriptor
#endif

#if defined (USB_MICROPHONE) || defined (USB_SPEAKER)
	byIntfNum += 1;
	wACDescOffset += 0x09;  //sizeof AUDIO_CONTROL_STD_IF
#endif

#if defined (USB_MICROPHONE) && defined (USB_SPEAKER)
	wAudioCtrlLen += 0x0a;
#elif defined (USB_MICROPHONE) || defined (USB_SPEAKER)
	wAudioCtrlLen += 0x09;
#endif

#ifdef USB_MICROPHONE
	byIntfNum += 1;
	wAudioCtrlLen += 0x0c + 0x09; //size of microphone input and output descriptor
#endif

#ifdef USB_SPEAKER
	byIntfNum += 1;
	wAudioCtrlLen += 0x0c + 0x09; //size of speaker input and output descriptor
#endif

#ifdef USB_MOUSE
	byIntfNum += 1;
#endif

#ifdef USB_KEYBOARD
	byIntfNum += 1;
#endif
	
	*(ConfigDescriptor + 4) = byIntfNum;

#if defined (USB_MICROPHONE) || defined (USB_SPEAKER)
	pAudioCtrlLen = (u16*)(ConfigDescriptor + wACDescOffset + 5);
	*pAudioCtrlLen = wAudioCtrlLen;
#endif
	return;
}

///------------------------------------------------------------------------
void SendDescriptorData(void) 
{
  short i = 0;
  short n = 0;
  if(g_length < 8)
  {
	  n = g_length;
  }
  else
  {
	  n = 8;
  }

  g_length = g_length - n;
  WRITE_REG8(EDP0_PTR, 0);
  for (i=0; i<n; i++) 
  {
	  WRITE_REG8(EDP0_DAT, *g_table);
	  g_table++;
  }
}

void HandleGetDescriptor(void)
{
	short i, n;
	if(g_length < 8)
	{
		n = g_length;
	}
	else
	{
		n = 8;
	}
	g_length = g_length - n;
	WRITE_REG8(EDP0_PTR, 0);
	for (i=0; i<n; i++) 
	{
		WRITE_REG8(EDP0_DAT, *g_table);
	    g_table++;
	}
}

void ProcStdDevRequest()
{
	if (usb_valueh==1) 
	{
        g_table = DeviceDescriptor;
        g_length = sizeof (DeviceDescriptor);
    }
    else if (usb_valueh==2) 
	{
        g_table = ConfigDescriptor;
        g_length = sizeof (ConfigDescriptor);
    }
    else if (usb_valueh==3 && usb_valuel==0) 
	{
        g_table = StringLangID;
        g_length = sizeof (StringLangID);
    }
    else if (usb_valueh==3 && usb_valuel==0x1) 
	{
        g_table = StringVendor;
        g_length = sizeof (StringVendor);
    }
    else if (usb_valueh==3 && usb_valuel==0x2) 
	{
        g_table = StringProduct;
        g_length = sizeof (StringProduct);
    }
    else if (usb_valueh==3 && usb_valuel==0x3) 
	{
        g_table = StringSerial;
        g_length = sizeof (StringSerial);
    }
    else
	{
		g_stall = 1;
	}

    if (usb_length < g_length)
	{
		g_length = usb_length;
	}
	return;
}

void ProcStdIntfRequest()
{
	switch(usb_valueh)
	{
		// HID Descriptor
	case 0x21:
		if(usb_indexl == 4)          //usb_indexl is the interface number
		{//mouse
#ifdef USB_MOUSE
			g_table = MouseHidDesc;
			g_length = sizeof (MouseHidDesc);
#endif
		}
		else if(usb_indexl == 5)
		{//keyboard
#ifdef USB_KEYBOARD
			g_table = KeyboardHidDesc;
			g_length = sizeof(KeyboardHidDesc);
#endif
		}
		break;
		
		//Report Descriptor
	case 0x22:
		if(usb_indexl == 4)
		{//mouse
#ifdef USB_MOUSE
			g_table = MouseReportDesc;
			g_length = sizeof (MouseReportDesc);
			//printf("Send mouse report descriptor. \n");
#endif
		}
		else if(usb_indexl == 5)
		{//keyboard
#ifdef USB_KEYBOARD
			g_table = KeyboardReportDesc;
			g_length = sizeof(KeyboardReportDesc);
			//printf("Send keyboard report descriptor. \n");
#endif
		}
		break;			
		// Phisical Descriptor
	case 0x23:
		// TODO
		break;			
		// other condition
	default:
		break;
	}
	if (usb_length < g_length)
	{
		g_length = usb_length;
	}
	return;
}

void ProcOutClassIntfRequest(int nDataStage)
{
	switch(usb_request)
	{
		// set_report
	case 0x09:
		switch(usb_valueh)
		{
			// report_type_input
		case 0x01:
			// TODO
			break;
			
			// report_type_output
		case 0x02:
			// usb_hid_set_report_ouput();
			break;
			
			// report_type_feature
		case 0x03:
			// TODO
			break;
			
		default:
			break;
		}
		break;
		
		// set_idle (used for idle test)              //??
		case 0x0a:
			if(nDataStage)
			{
				WRITE_REG8(EDP0_PTR, 0);
				g_rate = READ_REG8(EDP0_DAT); //??
			}
			g_rate = usb_valueh; //??
			break;
			
			// set_protocol                              //??
		case 0x0b:
			if(nDataStage)
			{
				WRITE_REG8(EDP0_PTR, 0);
				g_protocol = READ_REG8(EDP0_DAT);
			}
			g_protocol = usb_valuel;
			break;
	}
}

void ProcInClassIntfRequest()
{
	switch(usb_request)
	{
		// get_hid_input
	case 0x01:
		WRITE_REG8(EDP0_PTR, 0);
		WRITE_REG8(EDP0_DAT, 0x81);
		WRITE_REG8(EDP0_DAT, 0x02);
		WRITE_REG8(EDP0_DAT, 0x55);
		WRITE_REG8(EDP0_DAT, 0x55);
		break;
		
		// get_idle (used for idle test)
	case 0x02:
		WRITE_REG8(EDP0_PTR, 0);
		WRITE_REG8(EDP0_DAT, g_rate);
		break;
		
		// get_protocol
	case 0x03:
		WRITE_REG8(EDP0_PTR, 0);
		WRITE_REG8(EDP0_DAT, g_protocol);
		break;
		
	default:
		break;
	}
	return;
}

void ProcSetInterface()
{
	WRITE_REG8(EDP0_PTR,0);   //address
	WRITE_REG8 (EDP0_CTL, BIT0);	//Ack Data
	return;
}

void PerformRequest(int nDataStage) 
{
	if ( (bm_request==0x80) && (usb_request==0x06) ) {
		if (nDataStage == 0) 
		{
			ProcStdDevRequest();
		}
		HandleGetDescriptor();
	}
	else if ( (bm_request==0x81) && (usb_request==0x06))
	{
		if(nDataStage == 0)
		{
			ProcStdIntfRequest();
		}
		HandleGetDescriptor ();
	}
	else if (bm_request==0x21)  
	{
		ProcOutClassIntfRequest(nDataStage);
	}
	else if (bm_request==0xa1)  
	{
		ProcInClassIntfRequest();
	}
	else if( (bm_request == 0x01) && (usb_request == 0x0b) )
	{
		ProcSetInterface();
	}
	else
	{
		g_stall = 1;
	}
}

void Edp0Setup() 
{
	WRITE_REG8(EDP0_PTR,0);   //address
	bm_request	= READ_REG8(EDP0_DAT);
	usb_request	= READ_REG8(EDP0_DAT);
	usb_valuel	= READ_REG8(EDP0_DAT);
	usb_valueh	= READ_REG8(EDP0_DAT);
	usb_indexl	= READ_REG8(EDP0_DAT);
	usb_indexh	= READ_REG8(EDP0_DAT);
	usb_length	= READ_REG8(EDP0_DAT);
	usb_length	+= READ_REG8(EDP0_DAT) << 8;
	g_stall = 0;
	PerformRequest(0);
	if ( g_stall )
		WRITE_REG8(EDP0_CTL, BIT1);	//Stall Data
	else
		WRITE_REG8(EDP0_CTL, BIT0);	//Ack Data
}

void Edp0Data(void) 
{
	PerformRequest(1);
	if ( g_stall )
		WRITE_REG8(EDP0_CTL, BIT1);	//Stall Data
	else
		WRITE_REG8(EDP0_CTL, BIT0);	//Ack Data
}

void Edp0Status() 
{
	if ( g_stall )
		WRITE_REG8(EDP0_CTL, BIT3);	//Stall status
	else
		WRITE_REG8(EDP0_CTL, BIT2);	//Ack status
}

#if 0
void SendTestData()
{
	volatile static int s_nTimer = 0;
	volatile static char s_chInput = 0x59;
	int nDataIndx = 0;
	char achInput[8] = {0};

	s_nTimer = ( s_nTimer + 1 ) % 1000000;
	if(s_nTimer == 1000000 - 1)
	{
//		printf("Send keyboard data 0x%0x \n", s_chInput);
#if 0
		achInput[2] = s_chInput++;
		if(s_chInput > 0x61)
		{
			s_chInput = 0x59;
		}

		write_reg8(EDP2_PTR, 0);
		for(nDataIndx = 0; nDataIndx < 8; nDataIndx++)
		{
			write_reg8(EDP2_DAT, achInput[nDataIndx]);
		}
		write_reg16(EDP2_CTL, 0x01);
#endif
	}
	else if(s_nTimer == 500000)
	{
//		printf("Send mouse data. \n");
#if 1
		achInput[1] = 10;
		write_reg8(EDP1_PTR, 0);
		for(nDataIndx = 0; nDataIndx < 4; nDataIndx++)
		{
			write_reg8(EDP1_DAT, achInput[nDataIndx]);
		}
		write_reg16(EDP1_CTL, 0x01);
#endif
	}
	return;
}
#endif
void UsbTask()
{	  
    unsigned int irq_src = IRQSRC; //get interrupt source
  
    if (irq_src & IRQEDP0SETUP) {
    	WRITE_REG8(EDP0_STA, BIT4);    //clear setup irq
        Edp0Setup();
    }
    if (irq_src & IRQEDP0DATA) {
    	WRITE_REG8(EDP0_STA, BIT5);    //clear data irq
        Edp0Data( );
    }
    if (irq_src & IRQEDP0STATUS) {
    	WRITE_REG8(EDP0_STA, BIT6);    //clear status irq
        Edp0Status();
    }

}

void UsbIrqHandle() 
{
    unsigned int irq_src = IRQSRC; //get interrupt source
    if (irq_src & IRQEDPS) {
		    irq_edps = READ_REG8(EDPS_IRQ);
		    WRITE_REG8(EDPS_IRQ, irq_edps);
    }
	return;
}

void UsbInitInterrupt()
{
	WRITE_REG8(0x13c, 0x40);  //set Minimum threshold  to ACK endpoint 8 transfer
	
	WRITE_REG8(USB_MODE, 0xff & ~BIT7 & ~BIT5);// enable manuual mode of standard and get descriptor request
	USBMASK |= BIT7|BIT6; //enable endpoint7/endpoint6's interrupt(endpoint7/endpoint6 is taken as Audio input/output)
	IRQMSK |= IRQEDPS; //enable IRQEDPS interrupt
	return;
}

void UsbInit() 
{
	UsbInitInterrupt();

	/****** set buf address and size of endpoint7******/
	EP6BUFADDR = 0x80;
	EP7BUFADDR  = 0xC0;
	EPBUFSIZE = (256 >> 2);

	UsbInitDescriptor();

	return;
}
#define   N   7

#if 0
void WriteEndPoint(int nEnpNum, unsigned char * pData, int nDataLen)
{
	int nIndx = 0;
	write_reg8(EDPS_PTR + nEnpNum, 0);
	for(nIndx = 0; nIndx < nDataLen; nIndx++)
	{
		write_reg8(EDPS_DAT + nEnpNum, pData[nIndx]);
	}
	write_reg8(EDPS_CTL + nEnpNum, BIT0);
}
#endif

