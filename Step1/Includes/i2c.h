
#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    I2C_OK,
    I2C_ERROR,
    I2C_NACK
} i2c_status_t;

void i2c_init(void);

// Calls remain blocking; multi-byte payloads are moved by DMA.
i2c_status_t i2c_write(uint8_t address,
                       uint8_t reg,
                       const uint8_t *data,
                       size_t length);

i2c_status_t i2c_read(uint8_t address,
                      uint8_t reg,
                      uint8_t *data,
                      size_t length);

#endif
