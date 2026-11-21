
#include "stm32f4xx.h"
#include "i2c.h"
#include "lsm303agr.h"
#include "leds.h"
#include "compass.h"

// Set the local east-positive declination to correct magnetic north to true north.
#define MAGNETIC_DECLINATION_DEGREES 0.0f

// Global sensor readings for Cortex-Debug Live Watch
volatile lsm303agr_axes_t accel;
volatile lsm303agr_axes_t mag;

int main(void)
{
    leds_init();
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

        compass_side_t north_side = compass_north_side((const lsm303agr_axes_t *)&mag, MAGNETIC_DECLINATION_DEGREES);
        uint32_t led = 0U;

        // Viewed from the component side with the USB connector at the bottom.
        switch (north_side)
        {
            case COMPASS_SIDE_USB:    led = LED_LD6_BLUE;   break;
            case COMPASS_SIDE_RIGHT:  led = LED_LD5_RED;    break;
            case COMPASS_SIDE_STLINK: led = LED_LD3_ORANGE; break;
            case COMPASS_SIDE_LEFT:   led = LED_LD4_GREEN;  break;
            default: break;
        }

        leds_set(led);
    }
}
