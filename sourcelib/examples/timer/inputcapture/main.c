/**
  ******************************************************************************
  * @file    timer/inputcapture/main.c
  * @author  MDS
  * @date    02032018
  * @brief   Enable the Timer Input capture on Board Pin A6 (PC1).
  *			 See Section 18 (TIM3), P592 of the STM32F4xx Reference Manual.
  ******************************************************************************
  *
  */


#include "main.h"

int main(void) {
	/* STM32F4xx HAL library initialisation:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialisation
	 */
	HAL_Init();
	Hardware_init();

	/* Infinite loop */
	while (1) {
	}

	return 0;
}

/*
 * Initialise Hardware
 */
void Hardware_init(void) {

	BRD_LEDInit();		//Initialise LEDS

	// Turn off LEDs
	BRD_LEDRedOff();
	BRD_LEDGreenOff();
	BRD_LEDBlueOff();

	// Enable GPIOC Clock
	__GPIOB_CLK_ENABLE();

    GPIOB->OSPEEDR |= (GPIO_SPEED_FAST << 1);		//Set fast speed.
	GPIOB->PUPDR &= ~(0x03 << (1 * 2));				//Clear bits for no push/pull
	GPIOB->MODER &= ~(0x03 << (1 * 2));				//Clear bits
	GPIOB->MODER |= (GPIO_MODE_AF_PP << (1 * 2));  	//Set Alternate Function Push Pull Mode
	
	GPIOB->AFR[0] &= ~((0x0F) << (1 * 4));			//Clear Alternate Function for pin (lower ARF register)
	GPIOB->AFR[0] |= (GPIO_AF2_TIM3 << (1 * 4));	//Set Alternate Function for pin (lower ARF register) 

	// Timer 3 clock enable
	__TIM3_CLK_ENABLE();

	// Compute the prescaler value
	// Set the clock prescaler to 50kHz
	// SystemCoreClock is the system clock frequency
	TIM3->PSC = ((SystemCoreClock / 2) / TIMER_RUNNING_FREQ) - 1;

	// Counting direction: 0 = up-counting, 1 = down-counting (Timer Control Register 1)
	TIM3->CR1 &= ~TIM_CR1_DIR; 

	// Set the direction as input and select the active input
	// CC1S[1:0] for channel 1;
	// 00 = output
	// 01 = input, CC1 is mapped on timer Input 1
	// 10 = input, CC1 is mapped on timer Input 2
	// 11 = input, CC1 is mapped on slave timer
	TIM3->CCMR2 &= ~TIM_CCMR2_CC4S;
	TIM3->CCMR2 |= TIM_CCMR2_CC4S_0; 
											
	// Disable digital filtering by clearing IC1F[3:0] bits
	// because we want to capture every event
	TIM3->CCMR2 &= ~TIM_CCMR2_IC4F;
	
	// Select the edge of the active transition
	// Detect only rising edges in this example
	// CC4NP:CC4P bits
	// 00 = rising edge,
	// 01 = falling edge,
	// 10 = reserved,
	// 11 = both edges
	TIM3->CCER |= (TIM_CCER_CC4NP | TIM_CCER_CC4P);		// Both rising and falling edges.
	
	// Program the input prescaler
	// To capture each valid transition, set the input prescaler to zero;
	// IC4PSC[1:0] bits (input capture 1 prescaler)
	TIM3->CCMR2 &= ~TIM_CCMR2_IC4PSC; // Clear filtering because we need to capture every event
	
	// Enable Capture for channel 4
	TIM3->CCER |= TIM_CCER_CC4E;    
	
	// Enable related interrupts
	TIM3->DIER |= TIM_DIER_CC4IE;		// Enable Capture interrupts for channel 4
	TIM3->DIER |= TIM_DIER_UIE;			// Enable update interrupt to occur on timer counter overflow or underflow.
	
	//Enable priority (10) and interrupt callback. Do not set a priority lower than 5.
	HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	TIM3->CR1 |= TIM_CR1_CEN;				// Enable the counter
}

/*
 * Timer update interrupt callback
 */
void Tui_callback(void) {

	uint32_t current = 0;

	if((TIM3->SR & TIM_SR_UIF) != 0) {	// Check if input capture has taken place 

		BRD_LEDBlueToggle();												// If overflow occurred, Toggle LED 
		TIM3->SR &= ~TIM_SR_UIF; 					// Clear the UIF Flag
	}

	if((TIM3->SR & TIM_SR_CC4IF) != 0) { 
		current = TIM3->CCR4;  // Reading CCR1 clears CC4IF

		BRD_LEDRedToggle();												// If overflow occurred, Toggle LED 
	}
}