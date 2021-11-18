#include "main.h"

/*
 * Interrupt handler for EXTI 3 IRQ Handler
 */ 
void EXTI3_IRQHandler(void) {

    s4587423_reg_iss_synchroniser(S4587423_REG_ISS_SOURCE3);
    // Clear interrupt flag
    EXTI->PR |= EXTI_PR_PR3;
}