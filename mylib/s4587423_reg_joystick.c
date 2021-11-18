/**
***************************************************************
* @file     mylib/s4587423_reg_joystick.c
* @author   Peter Lahanas - 45874239
* @date     11032021
* @brief    Joystick Register Driver
* REFERENCE: csse3010_mylib_reg_joystick.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_joystick_init(void) - initialises the joystick pins and ADC
* S4587423_REG_JOYSTICK_X_READ() - Read the current X ADC value
* S4587423_REG_JOYSTICK_Y_READ() - Read the current Y ADC value
* S4587423_REG_JOYSTICK_Z_READ() - Check if joystick Z event has occured
***************************************************************
*/

#include "s4587423_reg_joystick.h"

#define BRD_PB_PORT GPIOC
#define BRD_PB_PIN 13

/**
 * @brief Initialise the joystick pins and ADC 
 * @param void
 * @return void
*/
void s4587423_reg_joystick_init(void) {

    /* Initialise the PB*/
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

    /* ---------------- Configure joystick button (Z) Pin A2 ---------------->*/
    __GPIOC_CLK_ENABLE();

    // Configure GPIO pin
    BRD_A2_GPIO_PORT->MODER &= ~(0x03 << (2 * BRD_A2)); // clear for input mode
    BRD_A2_GPIO_PORT->OSPEEDR &= ~(0x03 << (2 * BRD_A2)); // clear bits
    BRD_A2_GPIO_PORT->OSPEEDR |= (GPIO_SPEED_FAST << (2 * BRD_A2)); // fast mode
    BRD_A2_GPIO_PORT->PUPDR &= ~(0x03 << (2 * BRD_A2)); // Clear
    BRD_A2_GPIO_PORT->PUPDR |= (0x01 << (2 * BRD_A2)); // Set as pull up
   
    // Enable EXTI clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
   
    // // Set interrupt on Joystick
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI3;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PC;
    //SYSCFG->EXTICR[0] &= ~(0x000F); // clear bits below pin 3

    //EXTI->RTSR |= EXTI_RTSR_TR3;	//enable rising dedge
    EXTI->FTSR |= EXTI_FTSR_TR3;	//enable falling edge
    EXTI->IMR |= EXTI_IMR_IM3;		//Enable external interrupt
    
    //Enable priority and interrupt callback.
    HAL_NVIC_SetPriority(EXTI3_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    /* ---------------- Configure joystick X (Pin A0) ----------------------->*/

    __GPIOA_CLK_ENABLE();
    
    BRD_A0_GPIO_PORT->MODER   |= (0x03 << (2 * BRD_A0)); // analog mode
    BRD_A0_GPIO_PORT->OSPEEDR &= ~(0x03 << (2 * BRD_A0)); // clear
    BRD_A0_GPIO_PORT->OSPEEDR |= (GPIO_SPEED_FAST << (2 * BRD_A0)); // fast
    BRD_A0_GPIO_PORT->PUPDR   &= ~(0x03, (2 * BRD_A0)); //no pupd

    /* ----------------- Configure joystick Y (Pin A1) ---------------------->*/

    BRD_A1_GPIO_PORT->MODER   |= (0x03 << (2 * BRD_A1)); // analog mode
    BRD_A1_GPIO_PORT->OSPEEDR &= ~(0x03 << (2 * BRD_A1)); // clear
    BRD_A1_GPIO_PORT->OSPEEDR |= (GPIO_SPEED_FAST << (2 * BRD_A1)); // fast
    BRD_A1_GPIO_PORT->PUPDR   &= ~(0x03, (2 * BRD_A1)); //no pupd

    /*---------------- Setup Adc Handlers ----------------------------------->*/
    
    __ADC1_CLK_ENABLE(); //Enable ADC1 clock

    adcHandleX.Instance = (ADC_TypeDef *) (ADC1_BASE);
    adcHandleX.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2; //Set clock prescaler
    adcHandleX.Init.Resolution            = ADC_RESOLUTION12b; //Set 12-bit data resolution
    adcHandleX.Init.ScanConvMode          = DISABLE;
    adcHandleX.Init.ContinuousConvMode    = DISABLE;
    adcHandleX.Init.DiscontinuousConvMode = DISABLE;
    adcHandleX.Init.NbrOfDiscConversion   = 0;
    adcHandleX.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; //No Trigger
    adcHandleX.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1; //No Trigger
    adcHandleX.Init.DataAlign             = ADC_DATAALIGN_RIGHT; //Right align data
    adcHandleX.Init.NbrOfConversion       = 1;
    adcHandleX.Init.DMAContinuousRequests = DISABLE;
    adcHandleX.Init.EOCSelection          = DISABLE;

    HAL_ADC_Init(&adcHandleX);
    // Configure ADC Channel
    adcChanConfigX.Channel      = ADC_CHANNEL_3; //PA3 has Analog Channel 3 connected
    adcChanConfigX.Rank         = 1;
    adcChanConfigX.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    adcChanConfigX.Offset       = 0;
    
    HAL_ADC_ConfigChannel(&adcHandleX, &adcChanConfigX); //Initialise ADC Channel
    
    /*-------------------------- Setup ADC 2 -------------------------------->*/
    __ADC2_CLK_ENABLE(); //Enable ADC2 clock

    adcHandleY.Instance = (ADC_TypeDef *) (ADC2_BASE);
    adcHandleY.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2; //Set clock prescaler
    adcHandleY.Init.Resolution            = ADC_RESOLUTION12b; //Set 12-bit data resolution
    adcHandleY.Init.ScanConvMode          = DISABLE;
    adcHandleY.Init.ContinuousConvMode    = DISABLE;
    adcHandleY.Init.DiscontinuousConvMode = DISABLE;
    adcHandleY.Init.NbrOfDiscConversion   = 0;
    adcHandleY.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; //No Trigger
    adcHandleY.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1; //No Trigger
    adcHandleY.Init.DataAlign             = ADC_DATAALIGN_RIGHT; //Right align data
    adcHandleY.Init.NbrOfConversion       = 1;
    adcHandleY.Init.DMAContinuousRequests = DISABLE;
    adcHandleY.Init.EOCSelection          = DISABLE;

    HAL_ADC_Init(&adcHandleY);

    // Configure ADC Channel
    adcChanConfigY.Channel      = ADC_CHANNEL_10; //PC0 has Analog Channel 1 connected
    adcChanConfigY.Rank         = 1;
    adcChanConfigY.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    adcChanConfigY.Offset       = 0;

    HAL_ADC_ConfigChannel(&adcHandleY, &adcChanConfigY); //Initialise ADC Channel

}

/**
 * @brief Convert analog voltage to digital value (12 bits)
 * @param ADC_HandleTypeDef a pointer to the ADC handle
 * @return int the converted value
*/
int joystick_readxy(ADC_HandleTypeDef* handler) {
  
    HAL_ADC_Start(handler); // start conversion

    while (HAL_ADC_PollForConversion(handler, 10) != HAL_OK); // wait for end conversion
    
    int adcVal = (int) handler->Instance->DR;

    return adcVal;
}