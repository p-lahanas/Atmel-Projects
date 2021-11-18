#include "main.h"

/*
 * Interrupt handler for EXTI 15 IRQ Handler Pin D2, A5
 */ 
void EXTI15_10_IRQHandler(void) {

    uint32_t pending = EXTI->PR;
    if (pending & (1 << 10)) {
        // this was row 3
        s4587423_reg_keypad_iss_isr(S4587423_REG_ISS_SOURCE6);
        EXTI->PR |= EXTI_PR_PR10;
    } 

    if (pending & (1 << 15)) {
        // this was row 4
        s4587423_reg_keypad_iss_isr(S4587423_REG_ISS_SOURCE7);
        EXTI->PR |= EXTI_PR_PR15;
    }

    if (pending & (1 << 13)) {
        s4587423_reg_pb_on_isr();
    }
   
}

/*
 * Interrupt handler for EXTI 3 IRQ Handler
 */ 
void EXTI3_IRQHandler(void) {
    
    s4587423_reg_keypad_iss_isr(S4587423_REG_ISS_SOURCE4);
    EXTI->PR |= EXTI_PR_PR3;
}


/*
 * Interrupt handler for EXTI 5 IRQ Handler
 */ 
void EXTI9_5_IRQHandler(void) {

    s4587423_reg_keypad_iss_isr(S4587423_REG_ISS_SOURCE5);
    EXTI->PR |= EXTI_PR_PR5;
}