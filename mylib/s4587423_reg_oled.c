/**
***************************************************************
* @file     mylib/s4587423_reg_oled.c
* @author   Peter Lahanas - 45874239
* @date     16042021
* @brief    Joystick Register Driver
* REFERENCE: csse3010_mylib_reg_oled.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_oled_init() - initialise the SSD1306 device for I2C
***************************************************************
*/

#include "s4587423_reg_oled.h"

#define I2C_DEV_SDA_PIN     9
#define I2C_DEV_SCL_PIN     8
#define I2C_DEV_GPIO        GPIOB
#define I2C_DEV_GPIO_CLK()  __GPIOB_CLK_ENABLE()
#define I2C_DEV				I2C1
#define I2C_DEV_CLOCKSPEED 	100000

/** 
 * @brief Function to initialise the OLED display
 * @param void
 * @return void
*/
void s4587423_reg_oled_init() {

    /* Enable GPIO Clock */
    I2C_DEV_GPIO_CLK();

    /*--------------- Initialise SCL BEFORE initialising SDA --------------->*/
    
    // Clear alternate function PB8 (D15) (SCL) and PB9 (D14) (SDA)
    I2C_DEV_GPIO->AFR[1] &= ~((0x0F << (4 * (I2C_DEV_SCL_PIN - 8))) 
            | (0x0F << (4 * (I2C_DEV_SDA_PIN - 8))));

    // Set to alternate function 4 (I2C) 
    I2C_DEV_GPIO->AFR[1] |= ((0x04 << (4 * (I2C_DEV_SCL_PIN - 8)))) 
            | (0x04 << (4 * (I2C_DEV_SDA_PIN - 8)));
    // Clear MODER
    I2C_DEV_GPIO->MODER &= ~((0x03 << (2 * I2C_DEV_SCL_PIN)) 
            | (0x03 << (2 * I2C_DEV_SDA_PIN)));
    // Set MODER 
    I2C_DEV_GPIO->MODER |= ((GPIO_MODE_AF_OD << (2 * I2C_DEV_SCL_PIN)) 
            | (GPIO_MODE_AF_OD << (2 * I2C_DEV_SDA_PIN)));
    // Set low speed (0x00)
    I2C_DEV_GPIO->OSPEEDR &= ~((0x03 << (2 * I2C_DEV_SCL_PIN))
            | (0x03 << (2 * I2C_DEV_SDA_PIN))); 
    
    // Set for open-drain output
    I2C_DEV_GPIO->OTYPER |= (1 << I2C_DEV_SCL_PIN) | (1 << I2C_DEV_SDA_PIN);

    I2C_DEV_GPIO->PUPDR &= ~((0x03 << (2 * I2C_DEV_SCL_PIN)) 
            | (0x03 << (2 * I2C_DEV_SDA_PIN)));

    // Set for pull up 
    I2C_DEV_GPIO->PUPDR |= ((GPIO_PULLUP << (2 * I2C_DEV_SCL_PIN)) 
            | (GPIO_PULLUP << (2 * I2C_DEV_SDA_PIN)));

    // Enable I2C peripheral Clock 
    __I2C1_CLK_ENABLE();

    // Clear the selected I2C peripheral
    I2C_DEV->CR1 &= ~I2C_CR1_PE;
   
    uint32_t pclk1;
	uint32_t freqrange;
  
  	pclk1 = HAL_RCC_GetPCLK1Freq();			// Get PCLK1 frequency
  	freqrange = I2C_FREQRANGE(pclk1);		// Calculate frequency range 

  	//I2Cx CR2 Configuration - Configure I2Cx: Frequency range
  	MODIFY_REG(I2C_DEV->CR2, I2C_CR2_FREQ, freqrange);

	// I2Cx TRISE Configuration - Configure I2Cx: Rise Time
  	MODIFY_REG(I2C_DEV->TRISE, I2C_TRISE_TRISE, I2C_RISE_TIME(freqrange, I2C_DEV_CLOCKSPEED));

   	// I2Cx CCR Configuration - Configure I2Cx: Speed
  	MODIFY_REG(I2C_DEV->CCR, (I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR), I2C_SPEED(pclk1, I2C_DEV_CLOCKSPEED, I2C_DUTYCYCLE_2));

   	// I2Cx CR1 Configuration - Configure I2Cx: Generalcall and NoStretch mode
  	MODIFY_REG(I2C_DEV->CR1, (I2C_CR1_ENGC | I2C_CR1_NOSTRETCH), (I2C_GENERALCALL_DISABLE| I2C_NOSTRETCH_DISABLE));

   	// I2Cx OAR1 Configuration - Configure I2Cx: Own Address1 and addressing mode
  	MODIFY_REG(I2C_DEV->OAR1, (I2C_OAR1_ADDMODE | I2C_OAR1_ADD8_9 | I2C_OAR1_ADD1_7 | I2C_OAR1_ADD0), I2C_ADDRESSINGMODE_7BIT);

   	// I2Cx OAR2 Configuration - Configure I2Cx: Dual mode and Own Address2
  	MODIFY_REG(I2C_DEV->OAR2, (I2C_OAR2_ENDUAL | I2C_OAR2_ADD2), I2C_DUALADDRESS_DISABLE);

  	// Enable the selected I2C peripheral
	SET_BIT(I2C_DEV->CR1, I2C_CR1_PE);

	ssd1306_Init();	//Initialise SSD1306 OLED.

}