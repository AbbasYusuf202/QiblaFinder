
#include "lsm303agr.h"

// I2C addresses
#define LSM303AGR_ACC_ADDR       0x19U
#define LSM303AGR_MAG_ADDR       0x1EU

// Identity registers
#define ACC_WHO_AM_I             0x0FU
#define MAG_WHO_AM_I             0x4FU

#define ACC_ID                   0x33U
#define MAG_ID                   0x40U

// Accelerometer registers
#define CTRL_REG1_A              0x20U
#define CTRL_REG4_A              0x23U
#define OUT_X_L_A                0x28U

// Magnetometer registers
#define CFG_REG_A_M              0x60U
#define CFG_REG_C_M              0x62U
#define OUTX_L_REG_M             0x68U

static const struct
{
    uint8_t address, reg, value;
} sensor_config[] = {
    {LSM303AGR_ACC_ADDR, CTRL_REG4_A, 0x88U}, // BDU, high-resolution, +/-2g
    {LSM303AGR_ACC_ADDR, CTRL_REG1_A, 0x47U}, // 50 Hz, all accelerometer axes
    {LSM303AGR_MAG_ADDR, CFG_REG_A_M, 0x80U}, // Temperature compensation, continuous
    {LSM303AGR_MAG_ADDR, CFG_REG_C_M, 0x10U}  // Block magnetometer data updates
};

i2c_status_t lsm303agr_init(void)
{
    uint8_t id;
    i2c_status_t status;

    // Confirm both I2C addresses identify the expected sensor.
    status = i2c_read(LSM303AGR_ACC_ADDR, ACC_WHO_AM_I, &id, 1U);

    if (status != I2C_OK)
        return status;

    if (id != ACC_ID)
        return I2C_ERROR;

    status = i2c_read(LSM303AGR_MAG_ADDR, MAG_WHO_AM_I, &id, 1U);

    if (status != I2C_OK)
        return status;

    if (id != MAG_ID)
        return I2C_ERROR;


    // Keep sensor setup data together and stop if any register write fails.
    for (size_t i = 0; i < sizeof sensor_config / sizeof sensor_config[0]; i++)
    {
        status = i2c_write(sensor_config[i].address, sensor_config[i].reg,
                           &sensor_config[i].value, 1U);
        if (status != I2C_OK) return status;
    }

    return I2C_OK;
}


static i2c_status_t lsm303agr_read_axes(uint8_t address, uint8_t reg,
                                         volatile lsm303agr_axes_t *data,
                                         uint8_t shift)
{
    uint8_t buffer[6];

    if (data == 0)
        return I2C_ERROR;

    // Bit 7 enables automatic increment across the six axis registers.
    i2c_status_t status = i2c_read(address, (uint8_t)(reg | 0x80U), buffer, 6U);

    if (status != I2C_OK)
        return status;

    data->x = (int16_t)((uint16_t)buffer[1] << 8 | buffer[0]) >> shift;
    data->y = (int16_t)((uint16_t)buffer[3] << 8 | buffer[2]) >> shift;
    data->z = (int16_t)((uint16_t)buffer[5] << 8 | buffer[4]) >> shift;

    return I2C_OK;
}

i2c_status_t lsm303agr_read_accel(volatile lsm303agr_axes_t *data)
{
    // Accelerometer values are 12-bit and left-justified in each word.
    return lsm303agr_read_axes(LSM303AGR_ACC_ADDR, OUT_X_L_A, data, 4U);
}


i2c_status_t lsm303agr_read_mag(volatile lsm303agr_axes_t *data)
{
    return lsm303agr_read_axes(LSM303AGR_MAG_ADDR, OUTX_L_REG_M, data, 0U);
}
