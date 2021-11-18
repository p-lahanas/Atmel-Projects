/**
***************************************************************
* @file     stages/s3/main.c
* @author   Peter Lahanas - 45874239
* @date     04042021
* @brief    Main file for stage 3
***************************************************************
*/

#include "main.h"

/* PB ISS Source*/
#define PB_ISS_SIGNAL_SOURCE S4587423_REG_ISS_SOURCE1

/* FSM states for the CODEC*/
#define IDLE_STATE 0
#define ENCODE_STATE 1
#define DECODE_STATE 2

/* Global values for FSM*/
uint8_t currentState;
unsigned char key; // most recent key press
unsigned char keyPrev; // previous key press

/* Macros for key press value*/
#define PB_E 0x0E
#define PB_F 0x0F
#define PB_D 0x0D

void init() {

    HAL_Init();
    s4587423_reg_iss_init();
    s4587423_reg_lta1000g_init();
    s4587423_reg_keypad_init();
    s4587423_reg_iss_delaytimer_ctrl(S4587423_REG_ISS_SOURCE4, 10);
    s4587423_reg_iss_delaytimer_ctrl(S4587423_REG_ISS_SOURCE5, 10);
    s4587423_reg_iss_delaytimer_ctrl(S4587423_REG_ISS_SOURCE6, 10);
    s4587423_reg_iss_delaytimer_ctrl(S4587423_REG_ISS_SOURCE7, 10);
    s4587423_reg_iss_delaytimer_ctrl(PB_ISS_SIGNAL_SOURCE, 100);
    s4587423_reg_pb_on_init();
    currentState = IDLE_STATE;
}

void codec_fsm() {

    if (key == PB_E && keyPrev == PB_E) {
        currentState = ENCODE_STATE;
    } else if (key == PB_F && keyPrev == PB_F) {
        currentState = IDLE_STATE;
    } else if (key == PB_D && keyPrev == PB_D) {
        currentState = DECODE_STATE;
    }

    switch(currentState) {
        
        case IDLE_STATE:
            // display any key presses
            s4587423_reg_lta1000g_write(key);
            break;

        case ENCODE_STATE:
            /* Display what key has just been pressed*/ 
            if (keyPrev == PB_E) {
                s4587423_reg_lta1000g_write(key);
            } else { // display the encoded value

                uint8_t toEncode = (keyPrev << 4) | key;
                uint16_t encoded = s4587423_lib_hamming_byte_encode(toEncode);
                unsigned short displayValue;

                if (s4587423_reg_iss_eventcounter_read(PB_ISS_SIGNAL_SOURCE) 
                        % 2) {
                    displayValue = (unsigned short) (encoded);
                } else { // start with upper bits
                    displayValue = (unsigned short) (encoded >> 8);
                }
                
                s4587423_reg_lta1000g_write(displayValue);
            }
            break;

        case DECODE_STATE:
            // decode the input value and display on LED
            if (keyPrev == PB_D) {
                s4587423_reg_lta1000g_write(key);
            } else { // display the decoded value

                uint8_t toDecode = (keyPrev << 4) | key;
                uint8_t decoded = s4587423_lib_hamming_byte_decode(toDecode);
                unsigned short displayValue = 0;
                displayValue |= (decoded >> 4);

                /* Display parity on bits 8 and 9*/ 
                int parityErr = (int) 
                        s4587423_lib_hamming_parity_error(toDecode);
                displayValue |= parityErr << 9;
                
                // remove parity bit
                if ((decoded & 0xFE) != (toDecode & 0xFE)) { 
                    displayValue |= 0x100;
                }
                
                s4587423_reg_lta1000g_write(displayValue);
            }
            break;
    }


}

int main(void) {

    init();
    
    uint32_t lastFsmCall = HAL_GetTick();
    uint32_t btnCount = 0;

    while (1) {
        
        /* Update state machine for Keypad*/
        if (HAL_GetTick() - lastFsmCall >= 50) {
            s4587423_reg_keypad_fsmprocessing();
            lastFsmCall = HAL_GetTick();
        }

        /* A key has been pressed update key storage variables*/
        if (s4587423_reg_keypad_read_status()) {

            KeypadStatus = 0;
            if (btnCount % 2) {
                keyPrev = key;
            }

            key = s4587423_reg_keypad_read_key();
            btnCount++;
        }
        codec_fsm();
    }

    return 0;
}