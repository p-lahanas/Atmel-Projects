#ifndef S4587423_TSK_JOYSTICK_H
#define S4587423_TSK_JOYSTICK_H

#include "board.h"
#include "processor_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "s4587423_reg_joystick.h"
#include "s4587423_reg_iss.h"
#include "semphr.h"
#include "s4587423_cli_includes.h"

/* Creates the joystick controlling task*/
extern void s4587423_tsk_joystick_init(void);
extern void s4587423_tsk_joystick_deinit(void);

/* Interrupt callback for joystick*/
void joystick_callback();

/* Interrupt callback for pb*/
void pb_callback();

#endif