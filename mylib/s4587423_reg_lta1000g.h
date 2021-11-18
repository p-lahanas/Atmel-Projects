#ifndef S4587423_REG_LTA1000G_H
#define S4587423_REG_LTA1000G_H

#include "board.h"
#include "processor_hal.h"
#include "stm32f4xx_hal_gpio.h"

#define NUM_LEDS 10

extern void s4587423_reg_lta1000g_init(void);
extern void s4587423_reg_lta1000g_write(unsigned short value);

#endif