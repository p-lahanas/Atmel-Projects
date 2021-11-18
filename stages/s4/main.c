#include "main.h"

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define MAIN_STACK_SIZE	( configMINIMAL_STACK_SIZE * 2 )
// Task Priorities (Idle Priority is the lowest priority
#define MAIN_PRIORITY   ( tskIDLE_PRIORITY + 3 )

#define TTL_STACK_SIZE  ( configMINIMAL_STACK_SIZE * 2 )
#define TTL_PRIORITY     ( tskIDLE_PRIORITY + 3 )

#define TTR_STACK_SIZE  ( configMINIMAL_STACK_SIZE * 2 )
#define TTR_PRIORITY     ( tskIDLE_PRIORITY + 2 )

#define TTD_STACK_SIZE  ( configMINIMAL_STACK_SIZE * 2 )
#define TTD_PRIORITY     ( tskIDLE_PRIORITY + 3 )

// Timer value pixel offsets
#define START_X 10
#define START_Y 10

#define DISPLAY_TEXT_SIZE 20 // Font size


void mainTask(void);
void TaskTimerLeft(void);
void TaskTimerRight(void);
void TaskTimerDisplay(void);

/* A struct to hold the information to send between the timers and the 
        timer display*/
struct dualTimerMsg {

    char type; // either 'l' or 'r'
    unsigned char timerValue;
};

/* Queue which holds the dualTimerMsg*/
QueueHandle_t queueTimerDisplay;

int main () {

    HAL_Init();

    // Create the joystick task
    s4587423_tsk_joystick_init();

    // create the oled task
    s4587423_tsk_oled_init();

    // create the tasktimer left task
    xTaskCreate((TaskFunction_t) &TaskTimerLeft, 
            (const signed char *) "TTL", TTL_STACK_SIZE, 
            NULL, TTL_PRIORITY, NULL);
    // create the tasktimer right task
    xTaskCreate((TaskFunction_t) &TaskTimerRight, 
            (const signed char *) "TTR", TTR_STACK_SIZE, 
            NULL, TTR_PRIORITY, NULL);
    // create the tast timer display task
    xTaskCreate((TaskFunction_t) &TaskTimerDisplay, 
            (const signed char *) "TTD", TTD_STACK_SIZE, 
            NULL, TTD_PRIORITY, NULL);
    

    vTaskStartScheduler();

    return 0;
}

/**
 * @brief Task to handle the left timer
 * @param void
 * @return void
*/
void TaskTimerLeft(void) {

    const TickType_t xleftDelay = 1000 / portTICK_PERIOD_MS; // 1 second delay
    unsigned char leftTimerVal = 0; // current timer value
    struct dualTimerMsg msg;

    queueTimerDisplay = xQueueCreate(10, sizeof(struct dualTimerMsg));

    for (;;) {
        // Check the queue exists
        if (queueTimerDisplay != NULL) {
            msg.type = 'l';
            msg.timerValue = leftTimerVal;
            xQueueSendToFront(queueTimerDisplay, 
                    (void *) &msg, (portTickType) 10);

            vTaskDelay(xleftDelay);

            // Pause the count Z button has been pressed 
            if (!(s4587423_reg_iss_eventcounter_read(S4587423_REG_ISS_SOURCE3) 
                    % 2)) { 
                leftTimerVal++;
            } 
        } 
    }
}

/**
 * @brief Task to handle the right timer
 * @param void
 * @return void
*/
void TaskTimerRight(void) {

    const TickType_t xrightDelay = 50 / portTICK_PERIOD_MS; // delay 50ms
    unsigned char rightTimerVal = 0; // current timer value
    struct dualTimerMsg msg;

    for (;;) {

        // Check the queue exists
        if (queueTimerDisplay != NULL) {
            
            msg.type = 'r';
            msg.timerValue = rightTimerVal;
            xQueueSendToFront(queueTimerDisplay, 
                    (void *) &msg, (portTickType) 10);

            vTaskDelay(xrightDelay);

            // Pause counter if the Z button has been pressed
            if (!(s4587423_reg_iss_eventcounter_read(S4587423_REG_ISS_SOURCE3) 
                    % 2)) {
                rightTimerVal++;
            } 
        } 
    }
}

/**
 * @brief Function which sets the given string to space (' ')
 * @param displayText The array to set
 * @return void
*/
void clear_display_text(char displayText[]) {

    for (int i = 0; i < DISPLAY_TEXT_SIZE; i++) {
        displayText[i] = ' ';
    }
}

/**
 * @brief Task which handles the displaying of the two 
 *      separate timers (left and right)
 * @param void
 * @return void
*/
void TaskTimerDisplay(void) {
    
    struct oledTextMsg send;
    struct dualTimerMsg rec;
    
    clear_display_text(send.displayText);

    for(;;) {
        
        if (queueTimerDisplay != NULL) {

            if (xQueueReceive(queueTimerDisplay, &rec, 10)) {
                /*  THIS NEED WORK   */
                send.startX = START_X;
                send.startY = START_Y;
                
                send.displayText[2] = ':';
                // set two leftmost digits
                if (rec.type == 'l') {
                   
                    send.displayText[0] = '0' + ((rec.timerValue % 32) / 10);
                    send.displayText[1] = '0' + ((rec.timerValue % 32) % 10);
                } else {
                    
                    send.displayText[3] = '0' + ((rec.timerValue % 32) / 10);
                    send.displayText[4] = '0' + ((rec.timerValue % 32) % 10);
                }

                if (s4587423QueueOledMsg != NULL) {

                    xQueueSendToFront(s4587423QueueOledMsg, (void *) &send, 
                            (portTickType) 10);
                }
            }
        }
    }
}