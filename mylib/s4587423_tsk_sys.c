/**
***************************************************************
* @file     mylib/s4587423_tsk_sys.c
* @author   Peter Lahanas - 45874239
* @date     16052021
* @brief    System Status task
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_get_system_time_count(void) - get the current system time
* s4587423_tsk_sys_init(void) - creates a new system task
***************************************************************
*/

#include "s4587423_tsk_sys.h"

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define SYS_STACK_SIZE	( configMINIMAL_STACK_SIZE * 2 )

// Task Priorities (Idle Priority is the lowest priority
#define SYS_PRIORITY   ( tskIDLE_PRIORITY + 3 )

// Global task handle for sys
TaskHandle_t sysTaskHandle = NULL;

// Global system time
uint32_t systemTimeCount = 0;

void s4587423TaskSys(void);

/**
 * @brief Returns the system time count
 * @param void
 * @return void
 **/
uint32_t s4587423_get_system_time_count(void) {
    return systemTimeCount;
}

/**
 * @brief creates a new system task
 * @param void
 * @return void
 */
void s4587423_tsk_sys_init(void) {

    if (sysTaskHandle == NULL) {
        /* Create the System Control task*/
        xTaskCreate((TaskFunction_t) &s4587423TaskSys, 
            (const signed char *) "SYSTEM", SYS_STACK_SIZE, 
            NULL, SYS_PRIORITY, &sysTaskHandle);
    }
}

/**
 * @brief Removes a system task
 * @param void
 * @return void
 */
void s4587423_tsk_sys_deinit() {
    
    vTaskDelete(sysTaskHandle);
    sysTaskHandle = NULL;
}

/* System task handles LED status indicators and system time*/
void s4587423TaskSys(void) {

    // Initialise LED Hardware
    BRD_LEDInit();
    BRD_LEDRedOff();
    BRD_LEDGreenOff();
    BRD_LEDBlueOff();

    TickType_t tickCount = xTaskGetTickCount();
    EventBits_t uxBits;
    
 
    for (;;) {

        // check if 1 second has elapsed 
        if ((xTaskGetTickCount() - tickCount) * portTICK_PERIOD_MS >= 1000) {

            BRD_LEDGreenToggle(); // toggle alive led
            tickCount = xTaskGetTickCount();
            systemTimeCount++;
        }

        if (s4587423GroupEventCliState != NULL) {

            // wait for joystick state to be set (for value of 1 tick) 
            uxBits = xEventGroupWaitBits(s4587423GroupEventCliState, JOYSTICK_STATE, pdFALSE, pdFALSE, 1);

            // check if bit is set
            if (uxBits & JOYSTICK_STATE != 0) {

                BRD_LEDRedOn(); 
            } else {

                BRD_LEDRedOff();
            }
            if (uxBits & JOIN_ACK_REC) {

                BRD_LEDBlueOn();
            } else {

                BRD_LEDBlueOff();
            }
        }
        vTaskDelay(3);
    }
}