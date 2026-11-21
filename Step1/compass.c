#include <math.h>
#include "compass.h"

#define COMPASS_PI 3.14159265358979323846f
#define RAD_TO_DEG (180.0f / COMPASS_PI)

// Calibration for combined hard and soft iron interference on a specific spot on my desk
#define MAG_X_CENTER (-122.0f)
#define MAG_Y_CENTER (-181.75f)
#define MAG_FORWARD_X (-0.0068779f)
#define MAG_FORWARD_Y (-0.0076707f)
#define MAG_RIGHT_X    (0.0058723f)
#define MAG_RIGHT_Y   (-0.0088351f)

compass_side_t compass_north_side(const lsm303agr_axes_t *mag,
                                  float declination_degrees)
{
    if (mag == 0) return COMPASS_SIDE_INVALID;

    float x = (float)mag->x - MAG_X_CENTER;
    float y = (float)mag->y - MAG_Y_CENTER;
    if (x == 0.0f && y == 0.0f) return COMPASS_SIDE_INVALID;

    // Correct the measured sensor offset, axis orientation, and scale.
    float forward = MAG_FORWARD_X * x + MAG_FORWARD_Y * y;
    float right = -(MAG_RIGHT_X * x + MAG_RIGHT_Y * y);
    float magnetic_bearing = atan2f(right, forward) * RAD_TO_DEG;

    // East declination means magnetic north lies east of true north.
    float true_bearing = magnetic_bearing - declination_degrees;
    while (true_bearing < 0.0f) true_bearing += 360.0f;
    while (true_bearing >= 360.0f) true_bearing -= 360.0f;

    uint32_t sector = (uint32_t)((true_bearing + 45.0f) / 90.0f);
    if (sector >= 4U) sector = 0U;
    return (compass_side_t)sector;
}
