/**
***************************************************************
* @file     mylib/s4587423_tsk_joystick.c
* @author   Peter Lahanas - 45874239
* @date     18042021
* @brief    Joystick Task Driver
* REFERENCE: csse3010_mylib_task_cli_peripheral_guide.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_tsk_joystick_init() - initialise the joystick task
* s4587423_tsk_joystick_deinit(void) - destoys the current joystick task
***************************************************************
*/

#include "s4587423_tsk_joystick.h"

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define JOYSTICK_STACK_SIZE	( configMINIMAL_STACK_SIZE * 2 )

// Task Priorities (Idle Priority is the lowest priority
#define JOYSTICK_PRIORITY   ( tskIDLE_PRIORITY + 3 )

/* Semaphore which is given from the joystick Z interrupt */
SemaphoreHandle_t s4587423SemaphoreJoystickZ;

/* Create handle for the joystick task*/
TaskHandle_t joystickTaskHandle = NULL;

void s4587423TaskJoystick(void);

/**
 * @brief Initialises the joystick task function (call in main)
 * @param void
 * @return void
*/
void s4587423_tsk_joystick_init() {
    
    /* Create the Joystick Control task*/
    xTaskCreate((TaskFunction_t) &s4587423TaskJoystick, 
            (const signed char *) "JOYSTICK", JOYSTICK_STACK_SIZE, 
            NULL, JOYSTICK_PRIORITY, &joystickTaskHandle);

}

/**
 * @brief Destroys the Joystick Task
 * @param void
 * @return void
*/
void s4587423_tsk_joystick_deinit(void) {
    
    vTaskDelete(joystickTaskHandle);
    joystickTaskHandle = NULL;
}

/**
 * @brief The main joystick task. Checks if it can take the semaphore and 
 *      runs the iss
 * @param void
 * @return void
*/
void s4587423TaskJoystick(void) {

    // Create the joystick semaphore 
    s4587423SemaphoreJoystickZ = xSemaphoreCreateBinary();
    
    // Create the pb semaphore
    s4587423SemaphorePb = xSemaphoreCreateBinary();
    
    // Holds event bit states 
    EventBits_t uxBits;
    JoystickState vals;
    
    // Initialise joystick Z pin for interrupt
    portDISABLE_INTERRUPTS();
    s4587423_reg_joystick_init();
    portENABLE_INTERRUPTS();
    s4587423_reg_iss_init();
    s4587423_reg_iss_delaytimer_ctrl(S4587423_REG_ISS_SOURCE3, 50);
   
    s4587423_reg_iss_init();
    uint32_t joystickZCount = 0;

    for (;;) {
        
        // Check if semaphore has been created
        if (s4587423SemaphoreJoystickZ != NULL) {
             
            
            // check if we can take the semaphore
            if(xSemaphoreTake(s4587423SemaphoreJoystickZ, 1) == pdTRUE) {
                    
                // access to the joystick ISS
                s4587423_reg_iss_synchroniser(S4587423_REG_ISS_SOURCE3);
                
                if (s4587423_reg_iss_eventcounter_read(S4587423_REG_ISS_SOURCE3) != joystickZCount
                        && s4587423GroupEventCliState != NULL) {
                    
                    joystickZCount++;

                    if (joystickZCount % 2) {
                       
                        xEventGroupSetBits(s4587423GroupEventCliState, JOYSTICK_STATE_Z);
                    } else {
                        
                        xEventGroupClearBits(s4587423GroupEventCliState, JOYSTICK_STATE_Z);
                    }

                }
                
            }
        }

        // Check if joystick is active
        if (s4587423GroupEventCliState != NULL && s4587423QueueJoystick != NULL) {
           
            uxBits = xEventGroupWaitBits(s4587423GroupEventCliState, JOYSTICK_STATE, pdFALSE, pdFALSE, 1);

            if (uxBits & JOYSTICK_STATE) {

                // Update x and y values then write them to the queue 
                vals.x = S4587423_REG_JOYSTICK_X_READ();
                vals.y = S4587423_REG_JOYSTICK_Y_READ();
                xQueueSendToFront(s4587423QueueJoystick, (void *) &vals, (portTickType) 10);
            }

        }
        vTaskDelay(50);

    }
}

/**
 * @brief The joystick Z interrupt handler. Gives the semaphore on interrupt
 * @param void
 * @return void
*/
void joystick_callback() {

    // Type to check if giving a semaphore has unblocked a higher priority task 
    BaseType_t xHigherPriorityTaskWoken;
    
    // Clear interrupt flag
    EXTI->PR |= EXTI_PR_PR3;

    xHigherPriorityTaskWoken = pdFALSE;
  
    if (s4587423SemaphoreJoystickZ != NULL) {
        xSemaphoreGiveFromISR(s4587423SemaphoreJoystickZ, 
                &xHigherPriorityTaskWoken);

    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief The pb interrupt handler. Gives the semaphore on interrupt
 * @param void
 * @return void
*/
void pb_callback() {
   
    BaseType_t xHigherPriorityTaskWoken;
   
    EXTI->PR |= EXTI_PR_PR13; //Clear interrupt flag.
    
    xHigherPriorityTaskWoken = pdFALSE;
    
    if (s4587423SemaphorePb != NULL) {
    
        xSemaphoreGiveFromISR(s4587423SemaphorePb, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}