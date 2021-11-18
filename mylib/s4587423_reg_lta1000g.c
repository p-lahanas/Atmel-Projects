/**
***************************************************************
* @file     mylib/s4587423_reg_lta1000g.c
* @author   Peter Lahanas - 45874239
* @date     01032021
* @brief    LTA1000G LED Bar Display Register Driver
* REFERENCE: csse3010_mylib_reg_lta1000g.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_lta1000g_init() - initialise led bar pins
* s4587423_reg_lta1000g_write() - sets LED display according to value
***************************************************************
*/

#include "s4587423_reg_lta1000g.h"

/**
 * @brief Initialise an individual pin as output
 * @param uint pin number
 * @param GPIO_TypeDef* pointer to GPIO register struct
 */
void led_pin_init(unsigned int pinNum, GPIO_TypeDef *gpioPort) {

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
  * @brief  Initialise GPIO pins as output
  * @param  None
  * @retval None
*/
void s4587423_reg_lta1000g_init(void) {

    // Enable each port clock
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();

    // Initialise pins D16-D25
    led_pin_init(BRD_D16, BRD_D16_GPIO_PORT);
    led_pin_init(BRD_D17, BRD_D17_GPIO_PORT);
    led_pin_init(BRD_D18, BRD_D18_GPIO_PORT);
    led_pin_init(BRD_D19, BRD_D19_GPIO_PORT);
    led_pin_init(BRD_D20, BRD_D20_GPIO_PORT);
    led_pin_init(BRD_D21, BRD_D21_GPIO_PORT);
    led_pin_init(BRD_D22, BRD_D22_GPIO_PORT);
    led_pin_init(BRD_D23, BRD_D23_GPIO_PORT);
    led_pin_init(BRD_D24, BRD_D24_GPIO_PORT);
    led_pin_init(BRD_D25, BRD_D25_GPIO_PORT);
}

/**
  * @brief  Initialise GPIO pins as output
  * @param int the segment to change
  * @param unsigned char the value to set the segment
  * @retval None
  *
*/
void lta1000g_seg_set(int segment, unsigned char segmentValue) {

    switch (segment) {

    case 0:
        BRD_D16_GPIO_PORT->ODR &= ~(1 << BRD_D16); // reset pin
        BRD_D16_GPIO_PORT->ODR |= (segmentValue << BRD_D16);
        break;

    case 1:
        BRD_D17_GPIO_PORT->ODR &= ~(1 << BRD_D17); // reset pin
        BRD_D17_GPIO_PORT->ODR |= (segmentValue << BRD_D17);
        break;

    case 2:
        BRD_D18_GPIO_PORT->ODR &= ~(1 << BRD_D18); // reset pin
        BRD_D18_GPIO_PORT->ODR |= (segmentValue << BRD_D18);
        break;

    case 3:
        BRD_D19_GPIO_PORT->ODR &= ~(1 << BRD_D19); // reset pin
        BRD_D19_GPIO_PORT->ODR |= (segmentValue << BRD_D19);
        break;

    case 4:
        BRD_D20_GPIO_PORT->ODR &= ~(1 << BRD_D20); // reset pin
        BRD_D20_GPIO_PORT->ODR |= (segmentValue << BRD_D20);
        break;

    case 5:
        BRD_D21_GPIO_PORT->ODR &= ~(1 << BRD_D21); // reset pin
        BRD_D21_GPIO_PORT->ODR |= (segmentValue << BRD_D21);
        break;

    case 6:
        BRD_D22_GPIO_PORT->ODR &= ~(1 << BRD_D22); // reset pin
        BRD_D22_GPIO_PORT->ODR |= (segmentValue << BRD_D22);
        break;

    case 7:
        BRD_D23_GPIO_PORT->ODR &= ~(1 << BRD_D23); // reset pin
        BRD_D23_GPIO_PORT->ODR |= (segmentValue << BRD_D23);
        break;

    case 8:
        BRD_D24_GPIO_PORT->ODR &= ~(1 << BRD_D24); // reset pin
        BRD_D24_GPIO_PORT->ODR |= (segmentValue << BRD_D24);
        break;

    case 9:
        BRD_D25_GPIO_PORT->ODR &= ~(1 << BRD_D25); // reset pin
        BRD_D25_GPIO_PORT->ODR |= (segmentValue << BRD_D25);
        break;
    }
}

/**
  * @brief  Initialise GPIO pins as output
  * @param  unsigned short display pin mask
  * @retval None
  *
*/
void s4587423_reg_lta1000g_write(unsigned short value) {
    
    int mask = 0x001; // used to select an individual bit

    for (int i = 0; i < NUM_LEDS; i++) {

        lta1000g_seg_set(i, (value & mask) >> i);
        mask = mask << 1; // move mast over to check next bit
    }
}