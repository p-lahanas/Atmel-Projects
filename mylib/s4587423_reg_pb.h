#ifndef S4587423_REG_PB_H
#define S4587423_REG_PB_H

#include "board.h"
#include "stm32f429xx.h"
#include "processor_hal.h"
#include "s4587423_reg_iss.h"

extern void s4587423_reg_pb_on_init(void);
extern void s4587423_reg_pb_on_isr(void);
extern void s4587423_reg_pb_iss_init(void);
extern void s4587423_reg_pb_iss_isr(void);

#endif