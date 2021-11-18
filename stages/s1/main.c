/**
***************************************************************
* @file     stages/s1/main.c
* @author   Peter Lahanas - 45874239
* @date     05032021
* @brief    Main file for stage 1
***************************************************************
*/

#include "main.h"

/* Source index for ISS ----------------------------------------------------->*/
#define PB_SOURCE S4587423_REG_ISS_SOURCE1
#define XENON_SOURCE S4587423_REG_ISS_SOURCE2

/* Delay for Xenon and Push Button ------------------------------------------>*/
#define ISS_DELAY 50 

/* MPD direction constants -------------------------------------------------->*/
#define RIGHT 0
#define LEFT 1

/**
 * @brief Initialise pins and interrupts
 * @param None
 * @return None
*/
void init(void) {

    BRD_init();
    s4587423_reg_lta1000g_init(); // init LED bar
    s4587423_reg_pb_on_init(); // set pb interrupt
    s4587423_reg_pb_iss_init(); // set Xenon input source
    s4587423_reg_iss_init(); // initialise ISS
    s4587423_reg_iss_delaytimer_ctrl(PB_SOURCE, ISS_DELAY); // set delay values
    s4587423_reg_iss_delaytimer_ctrl(XENON_SOURCE, ISS_DELAY); 
}

/**
 * @brief Initialise the program state
 * @param ProgramState* a pointer to the struct to hold the program state
 * @return None
*/
void init_state(ProgramState* state) {

    state->dispDir = RIGHT;
    state->displayVal = 0x300;
    state->xenonCount = s4587423_reg_iss_eventcounter_read(XENON_SOURCE);
    state->pbCount = s4587423_reg_iss_eventcounter_read(PB_SOURCE);
    state->lastPbPress = 0;
    state->firstPress = true;
}

/**
 * @brief Shifts the MPD depending on program state direction
 * @param ProgramState* a pointer to the current program state
 * @return None
*/
void move_pattern_display(ProgramState* state) {

    if (state->dispDir == RIGHT) {
        
        if (state->displayVal == 0x03) { // check if at end of the display
            state->displayVal = 0x300; // reset pattern
        } else {
            state->displayVal = (state->displayVal >> 1);
        }
    } else { // Moving left
       
        if (state->displayVal == 0x300) {
            state->displayVal = 0x003;
        } else {
            state->displayVal = (state->displayVal << 1);
        }
    }
}

/**
 * @brief Displays the event count on the LED display
 * @param ProgramState* a pointer to the current program state
 * @return None
*/
void show_display_counts(ProgramState* state) {

    uint32_t pbCount = s4587423_reg_iss_eventcounter_read(PB_SOURCE);
    uint32_t xenCount = s4587423_reg_iss_eventcounter_read(XENON_SOURCE);

    // display xenon count on upper 5 bits
    unsigned short xenWrite = (unsigned short) (xenCount << 5);
    unsigned short pbWrite = (unsigned short) (pbCount);
    unsigned short write = xenWrite |= pbWrite;
    s4587423_reg_lta1000g_write(write);

    HAL_Delay(5000); // delay 5 seconds
}

/**
 * @brief The main program loop called by the main function
 * @param ProgramState* a pointer to the current program state
 * @return None
*/
void loop(ProgramState* state) {

    // check if the xenon monitor has been activated
    uint32_t newXenCount = s4587423_reg_iss_eventcounter_read(XENON_SOURCE);

    if (newXenCount > state->xenonCount) { // xenon event has occured
        
        state->xenonCount = newXenCount; // update counter
        move_pattern_display(state);
        
        if (newXenCount > 10) { // Check if event counter exceeds 10 (reset)
            s4587423_reg_iss_eventcounter_reset(XENON_SOURCE);
            state->xenonCount = 0;
        }
    }
    uint32_t newPbCount = s4587423_reg_iss_eventcounter_read(PB_SOURCE);
    
    if (newPbCount > state->pbCount) { // check if button was pressed
        
        state->pbCount = newPbCount; // update pb count

        if (HAL_GetTick() - state->lastPbPress < 2000 
                && !state->firstPress) {
            show_display_counts(state);
        } else {

            state->firstPress = false;
            state->lastPbPress = HAL_GetTick();
            // button has been pressed so reverse direction
            state->dispDir ^= 0x01; // alternate between 0 and 1
        }
        
        if (newPbCount > 10) { // Check if even counter exceeds 10 (reset)

            s4587423_reg_iss_eventcounter_reset(PB_SOURCE);
            state->pbCount = 0;
        }
    }
    s4587423_reg_lta1000g_write(state->displayVal); // update display
}

int main(void) {
    
    init(); // Init pins and hardware

    ProgramState state; // Make and initialise program state
    init_state(&state);
    int displayVal = 0x01; 
    while(1) { // loop indefinitely
        //loop(&state);
        uint32_t newPbCount = s4587423_reg_iss_eventcounter_read(PB_SOURCE);
    
        if (newPbCount > state.pbCount) { // check if button was pressed
        
            state.pbCount++; // update pb count
            displayVal ^= 1;
        }

        s4587423_reg_lta1000g_write(displayVal);
    }

    return 0;
}