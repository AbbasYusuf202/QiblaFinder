
#ifndef LSM303AGR_H
#define LSM303AGR_H

#include <stdint.h>
#include "i2c.h"

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} lsm303agr_axes_t;

i2c_status_t lsm303agr_init(void);

i2c_status_t lsm303agr_read_accel(volatile lsm303agr_axes_t *data);

i2c_status_t lsm303agr_read_mag(volatile lsm303agr_axes_t *data);

#endif
