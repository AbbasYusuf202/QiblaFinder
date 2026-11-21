#include "stm32f4xx.h"
#include "leds.h"

#define LED_GPIO_PINS ((1U << 12) | (1U << 13) | (1U << 14) | (1U << 15))

void leds_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;

    GPIOD->MODER = (GPIOD->MODER & ~(0xFFU << 24)) | (0x55U << 24);
    GPIOD->OTYPER &= ~LED_GPIO_PINS;
    GPIOD->OSPEEDR &= ~(0xFFU << 24);
    GPIOD->PUPDR &= ~(0xFFU << 24);

    GPIOD->BSRR = LED_GPIO_PINS << 16;
}

void leds_set(uint32_t mask)
{
    uint32_t pins = 0U;
    mask &= LED_ALL;

    if (mask & LED_LD3_ORANGE) pins |= 1U << 13;
    if (mask & LED_LD4_GREEN)  pins |= 1U << 12;
    if (mask & LED_LD5_RED)    pins |= 1U << 14;
    if (mask & LED_LD6_BLUE)   pins |= 1U << 15;

    GPIOD->BSRR = (LED_GPIO_PINS << 16) | pins;
}
