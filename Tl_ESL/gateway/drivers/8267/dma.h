#ifndef _DMA_H_
#define _DMA_H_

#define REG_DMA_CHANNEL_EN    REG_ADDR8(0x520)
enum {
    FLD_DMA_CHANNEL_UART_RX  = BIT(0),
    FLD_DMA_CHANNEL_UART_TX  = BIT(1),
    FLD_DMA_CHANNEL_RF_RX    = BIT(2),
    FLD_DMA_CHANNEL_RF_TX    = BIT(3),
    FLD_DMA_CHANNEL_AES_DECO = BIT(4),
    FLD_DMA_CHANNEL_AES_CODE = BIT(5),
    FLD_DMA_CHANNEL_ALL = 0xff,
};

static inline void DMA_ChannelEnable(unsigned char ChannelMask)
{
    REG_DMA_CHANNEL_EN |= ChannelMask;
}

static inline void DMA_ChannelDisable(unsigned char ChannelMask)
{
    REG_DMA_CHANNEL_EN &= ~ChannelMask;
}

#endif /* _DMA_H_ */
