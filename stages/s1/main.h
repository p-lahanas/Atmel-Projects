#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include "board.h"
#include "processor_hal.h"
#include "s4587423_reg_lta1000g.h"
#include "s4587423_reg_iss.h"
#include "s4587423_reg_pb.h"

/* A structure to hold the program state*/
typedef struct {
    uint32_t xenonCount;
    uint32_t pbCount;
    unsigned short displayVal;
    int dispDir;
    uint32_t lastPbPress; // used to check for double press
    bool firstPress; // check if first button press
} ProgramState;

#endif