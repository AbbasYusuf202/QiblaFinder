#include "stm32f4xx.h"
#include "dma.h"

// DMA1 Stream 0 / Channel 1 = I2C1_RX.
#define DMA1_S0_FLAGS   ((1U << 0) | (1U << 2) | (1U << 3) | (1U << 4) | (1U << 5))
#define DMA1_S0_ERRORS  ((1U << 0) | (1U << 2) | (1U << 3))
#define DMA1_S0_TCIF    (1U << 5)

// DMA1 Stream 6 / Channel 1 = I2C1_TX.
#define DMA1_S6_FLAGS   ((1U << 16) | (1U << 18) | (1U << 19) | (1U << 20) | (1U << 21))
#define DMA1_S6_ERRORS  ((1U << 16) | (1U << 18) | (1U << 19))
#define DMA1_S6_TCIF    (1U << 21)

static void dma_disable_stream(DMA_Stream_TypeDef *stream)
{
    stream->CR &= ~DMA_SxCR_EN;
    while ((stream->CR & DMA_SxCR_EN) != 0U) { }
}

static int dma_wait_complete(DMA_Stream_TypeDef *stream,
                             volatile uint32_t *status,
                             volatile uint32_t *clear,
                             uint32_t all_flags,
                             uint32_t error_flags,
                             uint32_t complete_flag)
{
    while (((*status & complete_flag) == 0U) &&
           ((*status & error_flags) == 0U)) { }

    int success = ((*status & complete_flag) != 0U) &&
                  ((*status & error_flags) == 0U);
    dma_disable_stream(stream);
    *clear = all_flags;
    return success;
}

static int dma_prepare(DMA_Stream_TypeDef *stream,
                       uint32_t channel,
                       uint32_t direction,
                       volatile uint32_t *clear,
                       uint32_t flags,
                       uint32_t peripheral_address,
                       uint32_t memory_address,
                       size_t length)
{
    if (length == 0U || length > 65535U) return 0;

    dma_disable_stream(stream);
    *clear = flags;
    stream->PAR = peripheral_address;
    stream->M0AR = memory_address;
    stream->NDTR = (uint32_t)length;
    stream->FCR = 0U;
    stream->CR = (channel << DMA_SxCR_CHSEL_Pos) |
                 direction | DMA_SxCR_MINC | DMA_SxCR_PL_1;
    stream->CR |= DMA_SxCR_EN;
    return 1;
}

void dma_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;
}

int dma_i2c1_rx_prepare(uint8_t *data, size_t length)
{
    if (data == 0) return 0;
    return dma_prepare(DMA1_Stream0, 1U, 0U, &DMA1->LIFCR, DMA1_S0_FLAGS,
                       (uint32_t)&I2C1->DR, (uint32_t)data, length);
}

int dma_i2c1_rx_wait(void)
{
    return dma_wait_complete(DMA1_Stream0, &DMA1->LISR, &DMA1->LIFCR,
                             DMA1_S0_FLAGS, DMA1_S0_ERRORS, DMA1_S0_TCIF);
}

int dma_i2c1_tx_prepare(const uint8_t *data, size_t length)
{
    if (data == 0) return 0;
    return dma_prepare(DMA1_Stream6, 1U, DMA_SxCR_DIR_0,
                       &DMA1->HIFCR, DMA1_S6_FLAGS,
                       (uint32_t)&I2C1->DR, (uint32_t)data, length);
}

int dma_i2c1_tx_wait(void)
{
    return dma_wait_complete(DMA1_Stream6, &DMA1->HISR, &DMA1->HIFCR,
                             DMA1_S6_FLAGS, DMA1_S6_ERRORS, DMA1_S6_TCIF);
}

void dma_i2c1_abort(void)
{
    dma_disable_stream(DMA1_Stream0);
    dma_disable_stream(DMA1_Stream6);
    DMA1->LIFCR = DMA1_S0_FLAGS;
    DMA1->HIFCR = DMA1_S6_FLAGS;
}
