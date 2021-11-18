#include "main.h"

/**
***************************************************************
* @file     stages/s2/main.c
* @author   Peter Lahanas - 45874239
* @date     19032021
* @brief    Main file for stage 2
***************************************************************
*/

/* --------------------------- ISS Source definitions ----------------------->*/
#define PB_SOURCE S4587423_REG_ISS_SOURCE1
#define JOYSTICKZ_SOURCE S4587423_REG_ISS_SOURCE3

#define PAN_CHANGE 10 // move 10 degrees on joystick Z press

/**
 * @brief Initialises the program state variable
 * @param ProgramState a pointer to the current program state variable
 * @return void
*/
void init_program_state(ProgramState* state) {
    
    state->pbCount = s4587423_reg_iss_eventcounter_read(PB_SOURCE);
    state->joystickZCount = 
            s4587423_reg_iss_eventcounter_read(JOYSTICKZ_SOURCE);    
    state->dir = LEFT;
    state->panAngle = 0;
    state->tiltAngle = 0;
    state->panOffset = 0;
}

/**
 * @brief Initialise the board for the program to run
 * @param void
 * @return void
*/
void hardware_init(void) {

    HAL_Init();
    s4587423_reg_iss_init();
    s4587423_reg_pb_on_init();
    s4587423_reg_lta1000g_init();
    s4587423_reg_joystick_init();
    s4587423_reg_pantilt_init();
    s4587423_reg_iss_delaytimer_ctrl(JOYSTICKZ_SOURCE, 100);
    s4587423_reg_iss_delaytimer_ctrl(PB_SOURCE, 100);
}

/**
 * @brief Modifies pan offset
 * @param ProgramState The current program state
 * @return void
*/
void pan_z_move(ProgramState* state) {
    
    if (state->dir == RIGHT) {

        if ((state->panOffset + PAN_CHANGE) <= 70) {
            state->panOffset += PAN_CHANGE;
        } else {
            state->panOffset = 70;
        }

    } else if (state->dir == LEFT) { // Same but for right

        if ((state->panOffset - PAN_CHANGE) >= -70) {
            state->panOffset -= PAN_CHANGE;
        } else {
            state->panOffset = -70;
        }
    }
}

/**
 * @brief Updates the LED display according to the program state
 * @param ProgramState The current program state
 * @return void
*/
void update_led_display(ProgramState* state) {
   
    int panAngle = S4587423_REG_PANTILT_PAN_READ() + 85;
    int tiltAngle = S4587423_REG_PANTILT_TILT_READ() + 85;
   
    int panShift = (int) ((panAngle / 170.0) * 5);
    int tiltShift = (int) ((tiltAngle / 170.0) * 5);

    state->displayVal = (unsigned short) ((0x01 << (panShift + 7)) 
            | (0x01 << (tiltShift + 2)));
    s4587423_reg_lta1000g_write(state->displayVal);
}

/**
 * @brief Convert joystick value to a pan rate
 * @param ProgramState a pointer to the program state
 * @return void
*/
void joystick_val_to_rate(ProgramState* state) {
    int pan = S4587423_REG_JOYSTICK_X_READ();
    int tilt = S4587423_REG_JOYSTICK_Y_READ();

    state->panAngle = (int) (((pan / 4095.0) * 170.0) - 85 
            + state->panOffset);

    if (state->panAngle > 85) {
        state->panAngle = 85;
    } else if (state->panAngle < -85) {
        state->panAngle = -85;
    }
    
    state->tiltAngle = (int) ((tilt / 4095.0) * 170.0) - 85;
}

/**
 * @brief Main program loop (checks polls input and modifies state)
 * @param ProgramState the current program state
 * @return void
*/
void loop(ProgramState* state) { // main program loop

    /* CHECK FOR PUSH BUTTON EVENT*/
    uint32_t newPbCount = s4587423_reg_iss_eventcounter_read(PB_SOURCE);
    
    if (newPbCount != state->pbCount) { //button has been pressed
       
        state->dir = !state->dir; // alternate the direction
        state->pbCount = newPbCount; // update the counter
    }

    ///* CHECK FOR JOYSTICK Z EVENT*/
    uint32_t newJoyZCount = s4587423_reg_iss_eventcounter_read(JOYSTICKZ_SOURCE);
    
    if (newJoyZCount != state->joystickZCount) { //joystick has been pressed

        pan_z_move(state);
        state->joystickZCount = newJoyZCount;
    }

    /* Update Joystick X and Y movements*/
    joystick_val_to_rate(state);
   
    /* Set servo positions*/
    S4587423_REG_PANTILT_PAN_WRITE(-1 * state->panAngle); //flip direction
    S4587423_REG_PANTILT_TILT_WRITE(state->tiltAngle);

    update_led_display(state);
} 


int main(void) {

    hardware_init();
    ProgramState state; // make program state in main scope
    init_program_state(&state);

    while(1) {
        loop(&state);
    }

    return 0;
}