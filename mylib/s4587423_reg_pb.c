/**
***************************************************************
* @file     mylib/s4587423_reg_pb.c
* @author   Peter Lahanas - 45874239
* @date     01032021
* @brief    Pushbutton Register Driver
* REFERENCE: csse3010_mylib_reg_pushbutton.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_pb_on_init() - initialise pb interrupt
* s4587423_reg_pb_on_isr() - interrupt service routine for push button
* s4587423_reg_pb_iss_init() - initialise ISS interrupt
* s4587423_reg_pb_iss_isr() - interrupt service routine for ISS
***************************************************************
*/

#include "s4587423_reg_pb.h"

/* Push button and ISS ports and pin definitions----------------------------->*/
#define BRD_PB_PORT GPIOC
#define ISS_PORT GPIOA
#define BRD_PB_PIN 13
#define ISS_PIN 3

/**
 * @brief Initialise push button as input and configure external interrupt
 * @param None
 * @return None
*/
void s4587423_reg_pb_on_init(void) {

    BRD_PB_PORT->OSPEEDR &= ~(0x03 << (2 * BRD_PB_PIN)); // Clear bits
    BRD_PB_PORT->OSPEEDR |= (GPIO_SPEED_FAST << (2 * BRD_PB_PIN)); //Fast speed
    BRD_PB_PORT->PUPDR &= ~(0x03 << (2 * BRD_PB_PIN)); //No push/pull
    BRD_PB_PORT->MODER &= ~(0x03 << (2 * BRD_PB_PIN)); //Input mode

    // Enable EXTI clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    //select trigger source (port c, pin 13) on EXTICR4.
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;
    SYSCFG->EXTICR[3] &= ~(0x000F);

    EXTI->RTSR |= EXTI_RTSR_TR13;	//enable rising dedge
    //EXTI->FTSR |= EXTI_FTSR_TR13;	//enable falling edge
    EXTI->IMR |= EXTI_IMR_IM13;		//Enable external interrupt

    //Enable priority and interrupt callback.
    NVIC_SetPriority(EXTI15_10_IRQn, 10);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/**
 * @brief Interrupt service routine for push button
 * @param None
 * @return None
*/
void s4587423_reg_pb_on_isr(void) {

    s4587423_reg_iss_synchroniser(S4587423_REG_ISS_SOURCE1);
    
    EXTI->PR |= EXTI_PR_PR13; //Clear interrupt flag.

}

/**
 * @brief Initialise PA3 as input and external interrupt
 * @param None
 * @return None
*/
void s4587423_reg_pb_iss_init(void) {

    ISS_PORT->OSPEEDR &= ~(0x03 << (2 * ISS_PIN)); // Clear
    ISS_PORT->OSPEEDR |= (GPIO_SPEED_FAST << (2 * ISS_PIN)); // Set
    ISS_PORT->PUPDR &= ~(0x03 << (2 * ISS_PIN)); // Clear for no push/pull
    ISS_PORT->MODER &= ~(0x03 << (2 * ISS_PIN)); // Clear for input mode

    // Enable EXTI clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Set PA3 as interrupt
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI3;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PA;
   
    // can reset stuff here if you would like should work without
    EXTI->RTSR |= EXTI_RTSR_TR3;	//enable rising dedge
    EXTI->FTSR |= EXTI_FTSR_TR3;	//enable falling edge
    EXTI->IMR |= EXTI_IMR_IM3;		//Enable external interrupt

    //Enable priority and interrupt callback.
    NVIC_SetPriority(EXTI3_IRQn, 2);
    NVIC_EnableIRQ(EXTI3_IRQn);
}

/**
 * @brief Interrupt service routine for PA3
 * @param None
 * @return None
*/
void s4587423_reg_pb_iss_isr(void) {

    s4587423_reg_iss_synchroniser(S4587423_REG_ISS_SOURCE2);
   
    // Clear interrupt flag
    EXTI->PR |= EXTI_PR_PR3;
}
