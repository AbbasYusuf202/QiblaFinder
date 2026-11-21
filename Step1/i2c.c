
#include "stm32f4xx.h"
#include "i2c.h"

#define I2C_ERROR_FLAGS (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR)

static void i2c_wait_bus_free(void)
{
    // START is only valid after the previous master releases the bus.
    while (I2C1->SR2 & I2C_SR2_BUSY);
}


static void i2c_abort(void)
{
    // End the incomplete transfer and restore the defaults used by reads.
    I2C1->CR1 |= I2C_CR1_STOP;

    I2C1->CR1 &= ~I2C_CR1_POS;
    I2C1->CR1 |= I2C_CR1_ACK;

    I2C1->SR1 &= ~I2C_ERROR_FLAGS;
}

// Wait for hardware progress; abort and report any I2C error flag.
static i2c_status_t i2c_wait_flag(uint32_t flag)
{
    while ((I2C1->SR1 & flag) == 0U)
    {
        if (I2C1->SR1 & I2C_ERROR_FLAGS)
        {
            i2c_status_t status = (I2C1->SR1 & I2C_SR1_AF) ? I2C_NACK : I2C_ERROR;
            i2c_abort();
            return status;
        }
    }

    return I2C_OK;
}


void i2c_init(void)
{
    // I2C1 uses PB6/PB9 on AF4; external pull-ups are expected.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~((3U << 12) | (3U << 18));
    GPIOB->MODER |=  ((2U << 12) | (2U << 18));
    GPIOB->OTYPER |= (1U << 6) | (1U << 9);
    GPIOB->OSPEEDR &= ~((3U << 12) | (3U << 18));
    GPIOB->OSPEEDR |=  ((2U << 12) | (2U << 18));
    GPIOB->PUPDR &= ~((3U << 12) | (3U << 18));
    GPIOB->AFR[0] &= ~(0xFU << 24);
    GPIOB->AFR[0] |=  (4U << 24);
    GPIOB->AFR[1] &= ~(0xFU << 4);
    GPIOB->AFR[1] |=  (4U << 4);

    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    I2C1->CR1 = 0U;
    I2C1->CR2 = 16U;
    I2C1->CCR = 80U;
    I2C1->TRISE = 17U;
    I2C1->CR1 = I2C_CR1_PE | I2C_CR1_ACK;
}


i2c_status_t i2c_write(uint8_t address,
                       uint8_t reg,
                       const uint8_t *data,
                       size_t length)
{
    i2c_status_t status;

    if (data == NULL && length != 0U) return I2C_ERROR;

    i2c_wait_bus_free();

    I2C1->CR1 |= I2C_CR1_START;

    status = i2c_wait_flag(I2C_SR1_SB);

    if (status != I2C_OK) return status;

    I2C1->DR = (uint8_t)(address << 1);

    status = i2c_wait_flag(I2C_SR1_ADDR);

    if (status != I2C_OK) return status;

    // Reading SR1 then SR2 clears ADDR on this STM32 peripheral.
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = reg;

    status = i2c_wait_flag(I2C_SR1_BTF);

    if (status != I2C_OK) return status;

    for (size_t i = 0U; i < length; i++)
    {
        status = i2c_wait_flag(I2C_SR1_TXE);

        if (status != I2C_OK) return status;

        I2C1->DR = data[i];
    }

    status = i2c_wait_flag(I2C_SR1_BTF);

    if (status != I2C_OK) return status;

    // Generate STOP
    I2C1->CR1 |= I2C_CR1_STOP;

    return I2C_OK;

}


i2c_status_t i2c_read(uint8_t address,
                      uint8_t reg,
                      uint8_t *data,
                      size_t length)
{
    i2c_status_t status;
    size_t index = 0U;

    if (data == NULL || length == 0U) return I2C_ERROR;

    i2c_wait_bus_free();

    // Reads start with ACK enabled and POS clear; the final bytes adjust this.
    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR1 &= ~I2C_CR1_POS;

    I2C1->CR1 |= I2C_CR1_START;

    status = i2c_wait_flag(I2C_SR1_SB);

    if (status != I2C_OK) return status;

    I2C1->DR = (uint8_t)(address << 1);

    status = i2c_wait_flag(I2C_SR1_ADDR);

    if (status != I2C_OK) return status;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = reg;

    status = i2c_wait_flag(I2C_SR1_BTF);

    if (status != I2C_OK) return status;

    // Switch from writing the register index to reading its contents.
    I2C1->CR1 |= I2C_CR1_START;

    status = i2c_wait_flag(I2C_SR1_SB);

    if (status != I2C_OK) return status;

    I2C1->DR = (uint8_t)((address << 1) | 1U);

    status = i2c_wait_flag(I2C_SR1_ADDR);

    if (status != I2C_OK) return status;

    if (length == 1U)
    {
        // Disable ACK before clearing ADDR
        I2C1->CR1 &= ~I2C_CR1_ACK;

        (void)I2C1->SR1;
        (void)I2C1->SR2;

        // Generate STOP
        I2C1->CR1 |= I2C_CR1_STOP;

        status = i2c_wait_flag(I2C_SR1_RXNE);

        if (status != I2C_OK) return status;

        data[index++] = (uint8_t)I2C1->DR;
    }
    else if (length == 2U)
    {
        // POS defers the disabled ACK so the final byte is NACKed.
        I2C1->CR1 |= I2C_CR1_POS;
        I2C1->CR1 &= ~I2C_CR1_ACK;

        (void)I2C1->SR1;
        (void)I2C1->SR2;

        status = i2c_wait_flag(I2C_SR1_BTF);

        if (status != I2C_OK) return status;

        I2C1->CR1 |= I2C_CR1_STOP;

        data[index++] = (uint8_t)I2C1->DR;
        data[index++] = (uint8_t)I2C1->DR;
    }
    else
    {
        // Clear ADDR before the receive pipeline starts.
        (void)I2C1->SR1;
        (void)I2C1->SR2;

        // Leave the last three bytes for the BTF-controlled shutdown sequence.
        while ((length - index) > 3U)
        {
            status = i2c_wait_flag(I2C_SR1_RXNE);

            if (status != I2C_OK) return status;

            data[index++] = (uint8_t)I2C1->DR;
        }

        // Wait for the final three bytes
        status = i2c_wait_flag(I2C_SR1_BTF);

        if (status != I2C_OK) return status;

        // NACK before draining the last three bytes, then request STOP.
        I2C1->CR1 &= ~I2C_CR1_ACK;

        data[index++] = (uint8_t)I2C1->DR;

        status = i2c_wait_flag(I2C_SR1_BTF);

        if (status != I2C_OK) return status;

        // Generate STOP
        I2C1->CR1 |= I2C_CR1_STOP;

        data[index++] = (uint8_t)I2C1->DR;
        data[index++] = (uint8_t)I2C1->DR;
    }

    // Leave the peripheral ready for the next transaction.
    I2C1->CR1 &= ~I2C_CR1_POS;
    I2C1->CR1 |= I2C_CR1_ACK;

    return I2C_OK;

}
