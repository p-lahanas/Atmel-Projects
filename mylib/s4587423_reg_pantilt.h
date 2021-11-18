#ifndef S4587423_REG_PANTILT_H
#define S4587423_REG_PANTILT_H

#include "board.h"
#include "processor_hal.h"

/* --------------------- Constants for servo mapping ------------------------>*/
#define PAN 0
#define TILT 1

extern void s4587423_reg_pantilt_init(void);
extern void s4587423_pantilt_angle_write(int type, int angle);
extern int s4587423_pantilt_angle_read(int type);

#define S4587423_REG_PANTILT_PAN_ZERO_CAL_OFFSET -5
#define S4587423_REG_PANTILT_TILT_ZERO_CAL_OFFSET 0

#define S4587423_REG_PANTILT_PAN_WRITE(angle) s4587423_pantilt_angle_write(PAN, angle)
#define S4587423_REG_PANTILT_PAN_READ() s4587423_pantilt_angle_read(PAN);
#define S4587423_REG_PANTILT_TILT_WRITE(angle) s4587423_pantilt_angle_write(TILT, angle)
#define S4587423_REG_PANTILT_TILT_READ() s4587423_pantilt_angle_read(TILT);

#endif