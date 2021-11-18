/**
***************************************************************
* @file     mylib/s4587423_reg_iss.c
* @author   Peter Lahanas - 45874239
* @date     28022021
* @brief    Input Signal Synchroniser Register Driver
* REFERENCE: csse3010_mylib_reg_iss.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_iss_init() - initialise ISS global variables
* s4587423_reg_ss_synchroniser() - debounces input signal and sets event counter
* s4587423_reg_iss_eventcounter_read() - return event counter value
* s4587423_reg_iss_lasttimer_read() - return lasttimer value
* s4587423_reg_iss_eventcounter_reset() - resets the event counter to 0
* s4587423_reg_iss_lasttimer_reset() - reset the lasttimer value to 0
* s4587423_reg_iss_delaytimer_control() - sets delay value for debouncing
***************************************************************
*/

#include "s4587423_reg_iss.h"

/*Sets the input signal synchronisation counters*/
void s4587423_reg_iss_init(void) {

    for (int i = 0; i < ISS_SOURCE_NUM; i++) {

        issEventcounterVal[i] = 0;
        issLasttimerVal[i] = 0;
        issDelayVal[i] = 10; // default value
        issIsRisingEdge[i] = true; //assume first press is a rising edge
    }
}

/**
 * @brief Synchronises input signal to debounce
 * @param unsigned char the input source index
 * @returns None 
*/
void s4587423_reg_iss_synchroniser(unsigned char signalSourceIndex) { 
    if ((HAL_GetTick() - s4587423_reg_iss_lasttimer_read(signalSourceIndex)) 
                 > issDelayVal[signalSourceIndex]) {

             issEventcounterVal[signalSourceIndex]++; // valid press
    }
    issLasttimerVal[signalSourceIndex] = HAL_GetTick();
}

/*Returns the event counter value for a given source index*/
uint32_t s4587423_reg_iss_eventcounter_read(unsigned char signalSourceIndex) {

    return issEventcounterVal[signalSourceIndex];
}

/*Returns the last timer value for a given source index*/
uint32_t s4587423_reg_iss_lasttimer_read(unsigned char signalSourceIndex) {

    return issLasttimerVal[signalSourceIndex];
} 
/*Resets the event counter for a given source index*/
void s4587423_reg_iss_eventcounter_reset(unsigned char signalSourceIndex) {

    issEventcounterVal[signalSourceIndex] = 0; // reset value to 0
}

/*Resets the last timer value for a given index*/
void s4587423_reg_iss_lasttimer_reset(unsigned char signalSourceIndex) {

    issLasttimerVal[signalSourceIndex] = 0;
}

/*Sets the delay value for a given index*/
void s4587423_reg_iss_delaytimer_ctrl(unsigned char signalSourceIndex, 
        int delayValue) {
    
    issDelayVal[signalSourceIndex] = (uint32_t) (delayValue);
}