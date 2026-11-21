#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

#define LED_LD3_ORANGE (1U << 0)
#define LED_LD4_GREEN  (1U << 1)
#define LED_LD5_RED    (1U << 2)
#define LED_LD6_BLUE   (1U << 3)
#define LED_ALL        (LED_LD3_ORANGE | LED_LD4_GREEN | LED_LD5_RED | LED_LD6_BLUE)

void leds_init(void);
void leds_set(uint32_t mask);

#endif
