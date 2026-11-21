#ifndef COMPASS_H
#define COMPASS_H

#include "lsm303agr.h"

typedef enum
{
    COMPASS_SIDE_USB = 0,
    COMPASS_SIDE_RIGHT,
    COMPASS_SIDE_STLINK,
    COMPASS_SIDE_LEFT,
    COMPASS_SIDE_INVALID = -1
} compass_side_t;

// Returns the board side facing closest to north; east declination is positive.
compass_side_t compass_north_side(const lsm303agr_axes_t *mag,
                                  float declination_degrees);

#endif
