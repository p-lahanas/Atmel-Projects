#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "board.h"
#include "processor_hal.h"
#include "s4587423_reg_lta1000g.h"
#include "s4587423_reg_iss.h"
#include "s4587423_reg_joystick.h"
#include "s4587423_reg_pantilt.h"
#include "s4587423_reg_pb.h"


/* --------------------- Constants for pan direction ------------------------>*/
#define LEFT 0
#define RIGHT 1

/* ------------------ Hold information about the current state -------------->*/
typedef struct {

    uint32_t pbCount;
    uint32_t joystickZCount;
    char dir;
    int panAngle;
    int tiltAngle;
    int panOffset;
    unsigned short displayVal;

} ProgramState;


#endif