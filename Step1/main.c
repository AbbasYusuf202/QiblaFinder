
#include "stm32f4xx.h"

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    GPIOD->MODER &= ~(3U << 28);
    GPIOD->MODER |=  (1U << 28);

    while (1)
    {
        GPIOD->ODR |= (1U << 14U);
        for (volatile uint32_t i = 0; i < 1000000U; i++);
        GPIOD->ODR &= ~(1U << 14U);
        for (volatile uint32_t i = 0; i < 1000000U; i++);
    }
}
