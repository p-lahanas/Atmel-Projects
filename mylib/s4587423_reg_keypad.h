#ifndef S4587423_REG_KEYPAD_H
#define S4587423_REG_KEYPAD_H

#include "board.h"
#include "processor_hal.h"
#include "stm32f429xx.h"
#include "s4587423_reg_iss.h"

/* States for the keypad FSM */
#define RSCAN1_STATE 1
#define RSCAN2_STATE 2
#define RSCAN3_STATE 3
#define RSCAN4_STATE 4

/* The current value of the keypad*/
unsigned char KeypadValue;

/* Status of the keypad (1 if key was pressed, 0 otherwise) */
int KeypadStatus;

extern void s4587423_reg_keypad_iss_isr(int gpioPin);
extern int s4587423_reg_keypad_read_status();
extern uint8_t s4587423_reg_keypad_read_key();
extern char s4587423_reg_keypad_read_ascii();
extern void s4587423_reg_keypad_init();
extern void s4587423_reg_keypad_fsmprocessing();

#endif