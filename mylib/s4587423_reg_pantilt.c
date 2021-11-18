/**
***************************************************************
* @file     mylib/s4587423_reg_pantilt.c
* @author   Peter Lahanas - 45874239
* @date     12032021
* @brief    PanTilt Register Driver
* REFERENCE: csse3010_mylib_reg_pantilt.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* void s4587423_reg_pantilt_init(void) - initialise the pantilt pins
* S4587423_REG_PANTILT_PAN_WRITE(angle) - write the given angle for the pan
* S4587423_REG_PANTILT_PAN_READ() - read the current pan angle
* S4587423_REG_PANTILT_TILT_WRITE(angle) - write the given angle for the tilt
* S4587423_REG_PANTILT_TILT_READ() - read the current tilt angle
***************************************************************
*/

#include "s4587423_reg_pantilt.h"

#define PULSE_FREQ 50 // frequency for PWM (20ms)
#define RES 327680 // resolution for PWM (ARR)


/*------------------- Servo Pulse Constants --------------------------------->*/
#define MIN_CYCLE 0.0225 // min is 2.25%
#define MIDDLE_CYCLE 0.0725
#define DUTY_RANGE 0.1 // 10% (12.25 - 2.25)

/**
 * @brief Initialise the pantilt pins and PWM timers 
 * @param void
 * @return void
*/
void s4587423_reg_pantilt_init(void) {

    __GPIOE_CLK_ENABLE(); // both pins are port E so happy days

    /* ----------------------- Set Pin D6 (PE9) ---------------------------->*/
    BRD_D6_GPIO_PORT->MODER &= ~(0x03 << (2 * BRD_D6)); // Clear pin
    BRD_D6_GPIO_PORT->MODER |= (GPIO_MODE_AF_PP << (2 * BRD_D6)); // Alternate  
    BRD_D6_GPIO_PORT->OSPEEDR &= ~(0x03 << (2 * BRD_D6)); // Clear
    BRD_D6_GPIO_PORT->OSPEEDR |= (GPIO_SPEED_FAST << (2 * BRD_D6)); // Clear
    BRD_D6_GPIO_PORT->PUPDR &= (0x03 << (2 * BRD_D6)); // Clear for no PUPD
    
    // Shift left by only 1 (AFR[1] starts at pin 8) (4 bits per pin)
    BRD_D6_GPIO_PORT->AFR[1] &= ~(0x0F << (4 * 1)); // clear
    BRD_D6_GPIO_PORT->AFR[1] |= (GPIO_AF1_TIM1 << (4 * 1)); // Set to AF1

    /* ------------------------ Set Pin D5 (PE11) --------------------------->*/
    BRD_D5_GPIO_PORT->MODER &= ~(0x03 << (2 * BRD_D5));
    BRD_D5_GPIO_PORT->MODER |= (GPIO_MODE_AF_PP << (2 * BRD_D5)); // Alternate
    BRD_D5_GPIO_PORT->OSPEEDR &= ~(0x03 << (2 * BRD_D5));
    BRD_D5_GPIO_PORT->OSPEEDR |= ~(GPIO_SPEED_FAST << (2 * BRD_D5));
    BRD_D5_GPIO_PORT->PUPDR &= (0x03 << (2 * BRD_D5));

    // Shift left by 3 (AFR[1] starts at pin 8) (4 bits per pin)
    BRD_D5_GPIO_PORT->AFR[1] &= ~(0x0F << (4 * 3)); // clear
    BRD_D5_GPIO_PORT->AFR[1] |= (GPIO_AF1_TIM1 << (4 * 3)); // Set to AF1

    /* ----------------------- General Timer Config ------------------------->*/
    __TIM1_CLK_ENABLE();

    // Set prescaler
    TIM1->PSC = (SystemCoreClock / (RES)) - 1;
    
    // Set as counting up
    TIM1->CR1 &= ~TIM_CR1_DIR;

    // Set arr value
    TIM1->ARR = (SystemCoreClock / (TIM1->PSC * PULSE_FREQ)) - 1;

    /* ----------------------- Setup for channel 1 (D6) --------------------->*/
    // For channel 1 set 7.25% duty cycle (middle)
    TIM1->CCR1 = MIDDLE_CYCLE * TIM1->ARR;
    
    // Clear output compare mode bit for channel 1
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= (0x06 << 4); // 6 for mode 1 and 4 for OCM position
    TIM1->CCMR1 |= (TIM_CCMR1_OC1PE); // enable preload 

    /* -------------------- Setup for channel 2 (D5) ------------------------>*/
    // Set channel 2 for 7.25% duty cycle (middle)
    TIM1->CCR2 = MIDDLE_CYCLE * TIM1->ARR;

    TIM1->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM1->CCMR1 |= (0x06 << 12); // 6 for mode 1 and 4 for OCM position
    TIM1->CCMR1 |= (TIM_CCMR1_OC2PE); // enable preload 

    // More generic timer stuff

    TIM1->CR1 |= (TIM_CR1_ARPE); // enable auto reload preload
    TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E; // enable output 
    TIM1->CCER &= ~TIM_CCER_CC1NE | ~TIM_CCER_CC2NE; // complementary outputa

    TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSR | TIM_BDTR_OSSI; 
    TIM1->CR1 |= TIM_CR1_CEN; // Enable the counter
}

/**
 * @brief Write the given angle to the given servo 
 * @param int the corresponding servo to change
 * @param int the angle to write the servo to
 * @return void
*/
void s4587423_pantilt_angle_write(int type, int angle) {
    
    /* Check angle to ensure not past servo extremes*/ 
    if (angle > 90) {

        angle = 90;
    } else if (angle < -90) {

        angle = -90;
    }

    if (type == TILT) {
        
        angle += S4587423_REG_PANTILT_TILT_ZERO_CAL_OFFSET;
        TIM1->CCR2 = (uint16_t) (((DUTY_RANGE * ((angle + 90.0) / 180.0)) 
                + MIN_CYCLE) * TIM1->ARR);
    } else { // this is pan
        
        angle += S4587423_REG_PANTILT_PAN_ZERO_CAL_OFFSET;
        TIM1->CCR1 = (uint16_t) (((DUTY_RANGE * ((angle + 90.0) / 180.0)) 
                + MIN_CYCLE) * TIM1->ARR);
    } 


}

/**
 * @brief Reads the angle of the selected servo
 * @param int the corresponding servo to read the angle from
 * @return int the angle of the servo
*/
int s4587423_pantilt_angle_read(int type) {

    // could rewrite this later to fix repeated code 
    if (type == TILT) {

        double dutyCycle = ((double) TIM1->CCR2 / (double) TIM1->ARR);
        double dutyPercentage = (dutyCycle - MIN_CYCLE) / DUTY_RANGE;
        return ((int) ((dutyPercentage * 180) - 90) - S4587423_REG_PANTILT_TILT_ZERO_CAL_OFFSET);
    } else {

        double dutyCycle = ((double) TIM1->CCR1 / (double) TIM1->ARR);
        double dutyPercentage = (dutyCycle - MIN_CYCLE) / DUTY_RANGE;
        return ((int) ((dutyPercentage * 180) - 90) - S4587423_REG_PANTILT_PAN_ZERO_CAL_OFFSET);   
    }
}
