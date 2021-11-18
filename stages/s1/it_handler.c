#include "main.h"

/*
 * Interrupt handler for EXTI 15 to 10 IRQ Handler
 */ 
void EXTI15_10_IRQHandler(void) {
    s4587423_reg_pb_on_isr();
}

/*
 * Interrupt handler for EXTI 3 IRQ Handler
 */ 
void EXTI3_IRQHandler(void) {
    s4587423_reg_pb_iss_isr();
}