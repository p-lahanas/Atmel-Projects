#ifndef S4587423_REG_ISS_H
#define S4587423_REG_ISS_H

/* ---------------------- Signal Source Index --------------------------------*/
#define S4587423_REG_ISS_SOURCE1 0
#define S4587423_REG_ISS_SOURCE2 1
#define S4587423_REG_ISS_SOURCE3 2
#define S4587423_REG_ISS_SOURCE4 3
#define S4587423_REG_ISS_SOURCE5 4
#define S4587423_REG_ISS_SOURCE6 5
#define S4587423_REG_ISS_SOURCE7 6

// Number of input signals
#define ISS_SOURCE_NUM 7

#include "board.h"
#include "processor_hal.h"
#include "stdbool.h"

uint32_t issEventcounterVal[ISS_SOURCE_NUM];
uint32_t issLasttimerVal[ISS_SOURCE_NUM];
uint32_t issDelayVal[ISS_SOURCE_NUM];
bool issIsRisingEdge[ISS_SOURCE_NUM]; //used to store if rising or falling edge

extern void s4587423_reg_iss_init(void);
extern void s4587423_reg_iss_synchroniser(unsigned char signalSourceIndex);
extern uint32_t s4587423_reg_iss_eventcounter_read(unsigned char signalSourceIndex);
extern uint32_t s4587423_reg_iss_lasttimer_read(unsigned char signalSourceIndex);
extern void s4587423_reg_iss_eventcounter_reset(unsigned char signalSourceIndex);
extern void s4587423_reg_iss_lasttimer_reset(unsigned char signalSourceIndex);
extern void s4587423_reg_iss_delaytimer_ctrl(unsigned char signalSourceIndex, 
        int delayValue);
    

#endif