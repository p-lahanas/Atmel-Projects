#include "main.h"

/*
 * Interrupt handler for EXTI 3 IRQ Handler
 */ 
void EXTI3_IRQHandler(void) {
    
    joystick_callback();
}