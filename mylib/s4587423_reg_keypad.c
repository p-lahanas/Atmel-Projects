/**
***************************************************************
* @file     mylib/s4587423_reg_keypad.c
* @author   Peter Lahanas - 45874239
* @date     01042021
* @brief    Keypad Register Driver
* REFERENCE: csse3010_mylib_reg_keypad.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_keypad_iss_isr(int gpioPin) - interrupt service routine handler
* s4587423_reg_keypad_read_status() - reads the current keypad status
* s4587423_reg_keypad_read_key() - returns the current keypad value
* s4587423_reg_keypad_read_ascii() - returns the current ascii keypad value
* s4587423_reg_keypad_init() - initialises the keypad status
* s4587423_reg_keypad_fsmprocessing() - FSM for row scanning 
***************************************************************
*/

#include "s4587423_reg_keypad.h"

#define ROW1_ISS_SOURCE S4587423_REG_ISS_SOURCE4
#define ROW2_ISS_SOURCE S4587423_REG_ISS_SOURCE5
#define ROW3_ISS_SOURCE S4587423_REG_ISS_SOURCE6
#define ROW4_ISS_SOURCE S4587423_REG_ISS_SOURCE7

#define keypad_col1() keypad_writecol(0x0E) // [1110]
#define keypad_col2() keypad_writecol(0x0D) // [1101]
#define keypad_col3() keypad_writecol(0x0B) // [1011]
#define keypad_col4() keypad_writecol(0x07) // [0111]

#define INIT_STATE 0
#define MIN_STATE_TIME 10


static int KeypadFsmCurrentstate;



/* Holds old event counter values for each row*/
typedef struct {

    uint32_t row1EventCount;
    uint32_t row2EventCount;
    uint32_t row3EventCount;
    uint32_t row4EventCount;
    uint32_t lastStateTime;

} RowEventCount;

RowEventCount count;

/**
 * @brief Configures a GPIO pin as output
 * @param pinNum the pin to configure
 * @param gpioPort a pointer to the gpio port for the corresponding pin
 * 
*/
void pin_set_output(unsigned int pinNum, GPIO_TypeDef* gpioPort) {

    gpioPort->MODER &= ~(0x03 << (2 * pinNum)); // Clear the mode bits
    gpioPort->MODER |= (GPIO_MODE_OUTPUT_PP << (2 * pinNum)); // Set as output

    gpioPort->OTYPER &= ~(0x01 << (pinNum)); // Clear
    gpioPort->OTYPER |= (0x00 << (pinNum));  // Set as push-pull

    gpioPort->OSPEEDR &= ~(0x03 << (2 * pinNum)); //Clear
    gpioPort->OSPEEDR |= (GPIO_SPEED_FREQ_HIGH << (2 * pinNum));

    gpioPort->PUPDR &= ~(0x03 << (2 * pinNum));
    gpioPort->PUPDR |= (GPIO_PULLDOWN << (2 * pinNum));
}

/**
 * @brief Configures a GPIO pin as input
 * @param pinNum the pin to configure
 * @param gpioPort a pointer to the gpio port for the corresponding pin
 * 
*/
void pin_set_input(unsigned int pinNum, GPIO_TypeDef* gpioPort) {

    gpioPort->MODER &= ~(0x03 << (2 * pinNum)); // Clear the mode bits

    gpioPort->OSPEEDR &= ~(0x03 << (2 * pinNum)); //Clear
    gpioPort->OSPEEDR |= (GPIO_SPEED_FREQ_HIGH << (2 * pinNum));

    gpioPort->PUPDR &= ~(0x03 << (2 * pinNum));
    gpioPort->PUPDR |= (GPIO_PULLUP << (2 * pinNum));

}

/**
 * @brief Initialises the GPIO pins for the keypad
*/
void keypad_gpio_init() {

    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();

    /* --------------- Configure pin D8, D7, D4, D3 as output ---------------*/
    pin_set_output(BRD_D8, BRD_D8_GPIO_PORT);
    pin_set_output(BRD_D7, BRD_D7_GPIO_PORT);
    pin_set_output(BRD_D4, BRD_D4_GPIO_PORT);
    pin_set_output(BRD_D3, BRD_D3_GPIO_PORT);

    /* --------------- Configure pin D2, A5, A4, A3 as input -----------------*/
    pin_set_input(BRD_D2, BRD_D2_GPIO_PORT);
    pin_set_input(BRD_A5, BRD_A5_GPIO_PORT);
    pin_set_input(BRD_A4, BRD_A4_GPIO_PORT);
    pin_set_input(BRD_A3, BRD_A3_GPIO_PORT);

    /* ---------- Configure pin D2, A5, A4, A3 as external interrupts --------*/
    
    // Enable EXTI clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* ------------------------- Interrupt on D2------------------------------*/

    //select trigger source (port F, pin 15) on EXTICR1.
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI15;
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI15_PF;

    //EXTI->RTSR |= EXTI_RTSR_TR15; //enable rising edge
    EXTI->FTSR |= EXTI_FTSR_TR15; //enable falling edge
    EXTI->IMR |= EXTI_IMR_IM15;	//Enable external interrupt

    //Enable priority and interrupt callback.
    //NVIC_SetPriority(EXTI15_10_IRQn, 10);
    //NVIC_EnableIRQ(EXTI15_10_IRQn);
   
    /* ------------------------- Interrupt on A5------------------------------*/
    
    //select trigger source (port F, pin 10) on EXTICR3.
    SYSCFG->EXTICR[2] &= ~SYSCFG_EXTICR3_EXTI10;
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PF;

    //EXTI->RTSR |= EXTI_RTSR_TR10; //enable rising edge
    EXTI->FTSR |= EXTI_FTSR_TR10; //enable falling edge
    EXTI->IMR |= EXTI_IMR_IM10;	//Enable external interrupt

    //Enable priority and interrupt callback.
    NVIC_SetPriority(EXTI15_10_IRQn, 10);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    
    /* ------------------------- Interrupt on A4------------------------------*/
    //select trigger source (port F, pin 5) on EXTICR2.
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI5;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PF;

    //EXTI->RTSR |= EXTI_RTSR_TR5; //enable rising edge
    EXTI->FTSR |= EXTI_FTSR_TR5; //enable falling edge
    EXTI->IMR |= EXTI_IMR_IM5; //Enable external interrupt

    //Enable priority and interrupt callback.
    NVIC_SetPriority(EXTI9_5_IRQn, 10);
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    /* ------------------------- Interrupt on A3------------------------------*/
    //select trigger source (port F, pin 3) on EXTICR1.
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI3;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PF;

    //EXTI->RTSR |= EXTI_RTSR_TR3; //enable rising edge
    EXTI->FTSR |= EXTI_FTSR_TR3; //enable falling edge
    EXTI->IMR |= EXTI_IMR_IM3; //Enable external interrupt

    //Enable priority and interrupt callback.
    NVIC_SetPriority(EXTI3_IRQn, 10);
    NVIC_EnableIRQ(EXTI3_IRQn);
}

/**
 * @brief Sets the column values
 * @param colval an integer with lower nibble representing each column value
*/
void keypad_writecol(uint8_t colval) {
    
    /* Set column 1*/
    if (colval & 0x01) {
        BRD_D3_GPIO_PORT->ODR |= ((colval & 0x01) << BRD_D3);
    } else {
        BRD_D3_GPIO_PORT->ODR &= ~(0x01 << BRD_D3);
    }
    
    if (colval & 0x02) {
        BRD_D4_GPIO_PORT->ODR |= ((!!(colval & 0x02)) << BRD_D4);
    } else {
        BRD_D4_GPIO_PORT->ODR &= ~(0x01 << BRD_D4);
    }

    if (colval & 0x04) {
        BRD_D7_GPIO_PORT->ODR |= ((!!(colval & 0x04)) << BRD_D7);
    } else {
        BRD_D7_GPIO_PORT->ODR &= ~(0x01 << BRD_D7);
    }
    
    if (colval & 0x08) {
        BRD_D8_GPIO_PORT->ODR |= ((!!(colval & 0x08)) << BRD_D8);
    } else {
        BRD_D8_GPIO_PORT->ODR &= ~(0x01 << BRD_D8);
    }
}

/**
 * @brief Returns the status of the keypad
 * @return KeypadStatus the status of the keypad
*/
int s4587423_reg_keypad_read_status() {
    return KeypadStatus;
}

/**
 * @brief Reads the rows on the keypad
 * @return res the value of the rows as an integer (lower nibble)
*/
uint8_t keypad_readrow() {
    
    uint8_t row1Val = s4587423_reg_iss_eventcounter_read(ROW1_ISS_SOURCE);
    uint8_t row2Val = s4587423_reg_iss_eventcounter_read(ROW2_ISS_SOURCE);
    uint8_t row3Val = s4587423_reg_iss_eventcounter_read(ROW3_ISS_SOURCE);
    uint8_t row4Val = s4587423_reg_iss_eventcounter_read(ROW4_ISS_SOURCE);

    uint8_t r1bit = 0, r2bit = 0, r3bit = 0, r4bit = 0;

    if (row1Val != count.row1EventCount) {
        count.row1EventCount = row1Val;
        r1bit = 1;
    }

    if (row2Val != count.row2EventCount) {
        count.row2EventCount = row2Val;
        r2bit = 1;
    }
   
    if (row3Val != count.row3EventCount) {
        count.row3EventCount = row3Val;
        r3bit = 1;
    }
   
    if (row4Val != count.row4EventCount) {
        count.row4EventCount = row4Val;
        r4bit = 1;
    }
  
    uint8_t res = r1bit | (r2bit << 1) | (r3bit << 2) | (r4bit << 3);

    if (res) {
        KeypadStatus = 1;
    }
    return res;
}

/**
 * @brief Reads the keypad key that is pressed
 * @return KeypadValue the new keypad value 
*/
uint8_t s4587423_reg_keypad_read_key() {

    uint8_t pressedRows = keypad_readrow();
    
     
    if (pressedRows) {

        uint8_t keypadMatrix[4][4] = { 
                                   {0x01, 0x04, 0x07, 0x00}, 
                                   {0x02, 0x05, 0x08, 0x0F},                     
                                   {0x03, 0x06, 0x09, 0x0E},
                                   {0x0A, 0x0B, 0x0C, 0x0D}
                                 };

        if (pressedRows & 0x01) { // row 1
            // currentState is 1-4 depending on row (-1 for indexing)
            KeypadValue = keypadMatrix[KeypadFsmCurrentstate - 1][0];
        } else if (pressedRows & 0x02) { // row 2
            // currentState is 1-4 depending on row (-1 for indexing)
            KeypadValue = keypadMatrix[KeypadFsmCurrentstate - 1][1];
        } else if (pressedRows & 0x04) { // row 3
            // currentState is 1-4 depending on row (-1 for indexing)
            KeypadValue = keypadMatrix[KeypadFsmCurrentstate - 1][2];
        } else if (pressedRows & 0x08) { // row 4
            // currentState is 1-4 depending on row (-1 for indexing)
            KeypadValue = keypadMatrix[KeypadFsmCurrentstate - 1][3];
        }
    }
    return KeypadValue;
}

/**
 * @brief Returns the ASCII representation of the pressed key
 * @return char the new keypad value 
*/
char s4587423_reg_keypad_read_ascii() {

    char lookupTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', 
                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    return lookupTable[(int) s4587423_reg_keypad_read_key()];
}

/**
 * @brief Initialise the keypad
*/
void s4587423_reg_keypad_init() {

    KeypadFsmCurrentstate = INIT_STATE;
    count.row1EventCount = 0;
    count.row2EventCount = 0;
    count.row3EventCount = 0;
    count.row4EventCount = 0;
    count.lastStateTime = 0;
    KeypadStatus = 0;
}

/**
 * @brief The state machine for keypad row scanning
*/
void s4587423_reg_keypad_fsmprocessing() {

    switch (KeypadFsmCurrentstate) {

        case INIT_STATE:
            
            keypad_gpio_init();
            
            KeypadFsmCurrentstate = RSCAN1_STATE;
            keypad_col1();
            count.lastStateTime = HAL_GetTick();
            break;

        case RSCAN1_STATE:
            
            if ((HAL_GetTick() - count.lastStateTime) > MIN_STATE_TIME) {
                s4587423_reg_keypad_read_key();
                KeypadFsmCurrentstate = RSCAN2_STATE;
                keypad_writecol(0xFF);
                keypad_col2();
                count.lastStateTime = HAL_GetTick();
            }
            break;
        
        case RSCAN2_STATE:
            
            if ((HAL_GetTick() - count.lastStateTime) > MIN_STATE_TIME) {
                s4587423_reg_keypad_read_key();
                KeypadFsmCurrentstate = RSCAN3_STATE;
                keypad_writecol(0xFF);
                keypad_col3();
                count.lastStateTime = HAL_GetTick();
            } 
            break;
        
        case RSCAN3_STATE:
            
            if ((HAL_GetTick() - count.lastStateTime) > MIN_STATE_TIME) {
                s4587423_reg_keypad_read_key();
                KeypadFsmCurrentstate = RSCAN4_STATE;
                keypad_writecol(0xFF);
                keypad_col4();
                count.lastStateTime = HAL_GetTick();
            } 
            break;

        case RSCAN4_STATE:
            
            if ((HAL_GetTick() - count.lastStateTime) > MIN_STATE_TIME) {
                s4587423_reg_keypad_read_key();
                KeypadFsmCurrentstate = RSCAN1_STATE;
                keypad_writecol(0xFF);
                keypad_col1();
                count.lastStateTime = HAL_GetTick();
            }
            break;
    }
}

/**
 * @brief Keypad interupt system routine
 * @param gpioPin the source for the interrupt
*/
void s4587423_reg_keypad_iss_isr(int gpioPin) {
   
    s4587423_reg_iss_synchroniser(gpioPin);
}