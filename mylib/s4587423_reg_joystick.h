#ifndef S4587423_REG_JOYSTICK_H
#define S4587423_REG_JOYSTICK_H

#include "board.h"
#include "processor_hal.h"
#include "stm32f429xx.h"
#include "s4587423_reg_iss.h"

void s4587423_reg_joystick_init(void);


/*------------------------- Globals for the ADC handles --------------------->*/
ADC_HandleTypeDef adcHandleX, adcHandleY;
ADC_ChannelConfTypeDef adcChanConfigX, adcChanConfigY;


/*-------------------------- Function prototypes ---------------------------->*/

int joystick_readxy(ADC_HandleTypeDef* handler);

/*--------------------------- MACRO Prototypes ------------------------------>*/
#define S4587423_REG_JOYSTICK_X_READ() joystick_readxy(&adcHandleX)
#define S4587423_REG_JOYSTICK_Y_READ() joystick_readxy(&adcHandleY)
#define S4587423_REG_JOYSTICK_Z_READ() s4587423_reg_iss_eventcounter_read(S4587423_REG_ISS_SOURCE3)

#endif