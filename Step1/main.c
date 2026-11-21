
#include "stm32f4xx.h"
#include "i2c.h"
#include "lsm303agr.h"

// Global sensor readings for Cortex-Debug Live Watch
volatile lsm303agr_axes_t accel;
volatile lsm303agr_axes_t mag;

int main(void)
{
    i2c_init();

    // A failed identity/configuration check means the sensor cannot be used.
    if (lsm303agr_init() != I2C_OK)
    {
        while (1)
        {
            // Sensor initialisation failed
        }
    }

    while (1)
    {
        // Skip this sample if the bus read failed; keep the last good values.
        if (lsm303agr_read_accel(&accel) != I2C_OK)
        {
            continue;
        }

        if (lsm303agr_read_mag(&mag) != I2C_OK)
        {
            continue;
        }
    }
}
