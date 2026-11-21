#ifndef DMA_H
#define DMA_H

#include <stddef.h>
#include <stdint.h>

void dma_init(void);
int dma_i2c1_rx_prepare(uint8_t *data, size_t length);
int dma_i2c1_rx_wait(void);
int dma_i2c1_tx_prepare(const uint8_t *data, size_t length);
int dma_i2c1_tx_wait(void);
void dma_i2c1_abort(void);

#endif
