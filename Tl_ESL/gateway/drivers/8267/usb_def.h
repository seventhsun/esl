#ifndef _USB_DEF_H_

#define _USB_DEF_H_




#define EDP0_PTR                       0x100
#define EDP0_DAT                       0x101
#define EDP0_CTL                       0x102
#define EDP0_STA                       0x103
#define USB_MODE                       0x104

#define EDPS_PTR                       0x110
#define EDPS_DAT                       0x118
#define EDPS_CTL                       0x120


#define EDP1_PTR                       0x111
#define EDP1_DAT                       0x119
#define EDP1_CTL                       0x121
#define EDP2_PTR                       0x112
#define EDP2_DAT			           0x11a
#define EDP2_CTL			           0x122
#define EDP3_PTR                       0x113
#define EDP3_DAT			           0x11b
#define EDP3_CTL			           0x123


#define USB_ISO                        0x138
#define EDPS_IRQ                       0x139
#define USBMASK                        __REG_B(0x80013a)
#define USBFIFO                        0x13d
#define USBFIFOTH                      0x13c
#define EP6BUFADDR                     __REG_B(0x80012e)
#define EP7BUFADDR                     __REG_B(0x80012f)
#define EPBUFSIZE                      __REG_B(0x80013e)


#define BIT0                            0x1
#define BIT1                            0x2
#define BIT2                            0x4
#define BIT3                            0x8
#define BIT4                            0x10
#define BIT5                            0x20
#define BIT6                            0x40
#define BIT7                            0x80
#define BIT8                            0x100
#define BIT9                            0x200
#define BIT10                           0x400
#define BIT11                           0x800
#define BIT12                           0x1000
#define BIT13                           0x2000
#define BIT14                           0x4000
#define BIT15                           0x8000
#define BIT16                           0x10000
#define BIT17                           0x20000
#define BIT18                           0x40000
#define BIT19                           0x80000
#define BIT20                           0x100000
#define BIT21                           0x200000
#define BIT22                           0x400000
#define BIT23                           0x800000
#define BIT24                           0x1000000
#define BIT25                           0x2000000
#define BIT26                           0x4000000
#define BIT27                           0x8000000
#define BIT28                           0x10000000
#define BIT29                           0x20000000
#define BIT30                           0x40000000
#define BIT31                           0x80000000

#define __REG(x)			            (*((volatile long *)(x))) 
#define __REG_W(x)			            (*((volatile short *)(x))) 
#define __REG_B(x)			            (*((volatile char *)(x)))

//IRQ
#define IRQMSK                          __REG(0x800640)
#define IRQIP                           __REG(0x800644)
#define IRQSRC                          __REG(0x800648)
#define IRQTMR0                         BIT0
#define IRQTMR1                         BIT1
#define IRQTMR2                         BIT2
#define IRQUSBPWDN                      BIT3
#define IRQDMA                          BIT4
#define IRQDFIFO                        BIT5
#define IRQSBC                          BIT6
#define IRQHOSTCMD                      BIT7
#define IRQEDP0SETUP                    BIT8
#define IRQEDP0DATA                     BIT9
#define IRQEDP0STATUS                   BIT10
#define IRQEDP0SETINTf                  BIT11
#define IRQEDPS                         BIT12
#define IRQSW                           BIT14
#define IRQGPIO                         BIT18
#define IRQGPIO2RISC0                   BIT22
#define IRQGPIO2RISC2                   BIT23

#define TMR0IRQ                         (0)
#define TMR1IRQ                         (1)
#define TMR2IRQ                         (2)
#define DMAIRQ                          (4)
#define DFIFOIRQ                        (5)
#define CODECIRQ                        (6)
#define HOSTCMDIRQ                      (7)
#define EDP0SETUPIRQ                    (8)
#define EDP0DATAIRQ                     (9)
#define EDP0STATUSIRQ                   (10)
#define EDP0SETINTfIRQ                  (11)
#define EDPSIRQ                         (12)
#define SWIRQ                           (14)
#define GPIOIRQ                         (18)
#define GPIO2RISC0IRQ                   (22)
#define GPIO2RISC2IRQ                   (23) 



#define USERREG                        0x6c

#endif
