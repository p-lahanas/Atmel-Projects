#include "main.h"

/*
 * Interrupt handler for USART3 IRQ Handler
 */ 
void USART3_IRQHandler(void) {

    Uart3_callback();
}

/*
 * Interrupt handler for USART6 IRQ Handler
 */ 
void USART6_IRQHandler(void) {

    Uart6_callback();
}

/*
 * Interrupt handler for EXTI 3 IRQ Handler (Joystick Z)
 */ 
void EXTI3_IRQHandler(void) {

    joystick_callback();
}

/*
 * Interrupt handler for EXTI 15 to 10 IRQ Handler
 */ 
void EXTI15_10_IRQHandler(void) {
    
    pb_callback();
}