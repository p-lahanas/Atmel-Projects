/**
  ******************************************************************************
  * @file    blink/main.c
  * @author  MDS
  * @date    27112020
  * @brief   Nucleo429ZI onboard LED flashing example. Uses the system time (ms) 
  *          and system delay (ms) to ensure that the LEDs toggle, every 2s.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "processor_hal.h"

void Hardware_init(void);

/*
 * @brief  Main program - flashes onboard LEDs
 */
int main(void)  {

  uint32_t prev_tick;

	HAL_Init();			//Initalise Board
	Hardware_init();	//Initalise hardware modules
	
  prev_tick = 0;

	// Main processing loop
  while (1) {
        
		// Toggle all LEDs, every 2 seconds
    if ((HAL_GetTick() - prev_tick) >= 2000) {
		  BRD_LEDRedToggle();
		  BRD_LEDGreenToggle();
		  BRD_LEDBlueToggle();

      prev_tick =  HAL_GetTick();
    }

		HAL_Delay(1000);		//Delay for 1s
	}

  return 0;
}

/*
 * @brief  Initialise Hardware
 */
void Hardware_init(void) {

	BRD_LEDInit();		//Initialise LEDS

	// Turn off LEDs
	BRD_LEDRedOff();
	BRD_LEDGreenOff();
	BRD_LEDBlueOff();
}
