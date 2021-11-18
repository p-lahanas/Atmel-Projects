/**
***************************************************************
* @file     mylib/s4587423_tsk_oled.c
* @author   Peter Lahanas - 45874239
* @date     19042021
* @brief    Joystick Task Driver
* REFERENCE: csse3010_mylib_task_cli_peripheral_guide.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* void s4587423_tsk_oled_init() - initialise the oled task
***************************************************************
*/

#include "s4587423_tsk_oled.h"

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define OLED_STACK_SIZE	( configMINIMAL_STACK_SIZE * 2 )

// Task Priorities (Idle Priority is the lowest priority
#define OLED_PRIORITY    ( tskIDLE_PRIORITY + 3 )

// The pixel constants for the grid
#define GRID_PIXEL_SIDE 30
#define START_HEIGHT 0
#define START_WIDTH 0

// Sorter dimensions (used for scaling)
#define SORTER_ACTUAL_HEIGHT 200.0
#define SORTER_ACTUAL_WIDTH 200.0

/* Drawing helper functions*/
void draw_box(void);
void draw_cursor(AscState state);
void draw_z_angle(AscState state);
void get_asc_state(AscState* currentState);

void s4587423TaskOled(void);

TaskHandle_t oledTaskHandle = NULL;

/**
 * @brief Creates the TaskOled Task
 * @param void
 * @return void
*/
void s4587423_tsk_oled_init() {
   
    xTaskCreate((TaskFunction_t) &s4587423TaskOled, 
            (const signed char *) "OLED", OLED_STACK_SIZE, 
            NULL, OLED_PRIORITY, &oledTaskHandle);

}

/**
 * @brief Destroys the Oled Task
 * @param void
 * @return void
*/
void s4587423_tsk_oled_deinit(void) {
    
    vTaskDelete(oledTaskHandle);
    oledTaskHandle = NULL;
}

/**
 * @brief The main task to update the oled display
 * @param void
 * @return void
*/
void s4587423TaskOled(void) {

    s4587423_reg_oled_init();

    struct oledTextMsg receivedMsg; // the received message

    AscState currentState;

    for (;;) {

        // Update the current state
        get_asc_state(&currentState);

        // Pause tasks and resume after I2C transmission
        vTaskSuspendAll();
        ssd1306_Fill(Black); // clear screen
        xTaskResumeAll();

        // Draw elements of the display
        draw_box();
        draw_cursor(currentState);
        draw_z_angle(currentState);

        // Suspend tasks for screen update (I2C not interrupted) 
        vTaskSuspendAll();
        ssd1306_UpdateScreen();
        xTaskResumeAll();

        vTaskDelay(20);
    }
}

/**
 * @brief Gets the current asc values from the queue
 * @param currentState an AscState variable to hold the current state
 * @return void
 **/
void get_asc_state(AscState* currentState) {

    if (s4587423QueueAscState != NULL) {

        xQueueReceive(s4587423QueueAscState, currentState, 10 );
    }
}

// Updates and draws the z value and asc angle
void draw_z_angle(AscState state) {

    char zVal[4], angle[4]; 

    sprintf(zVal, "%d", state.z);
    
    sprintf(angle, "%d", state.angle);
   
    vTaskSuspendAll();
    // Go to top right for Z
    ssd1306_SetCursor(100, 8);
    ssd1306_WriteString(zVal, Font_6x8, SSD1306_WHITE);

    // Go to bottom right for angle
    ssd1306_SetCursor(100, 20);
    ssd1306_WriteString(angle, Font_6x8, SSD1306_WHITE);

    xTaskResumeAll();

}

// Draws the '+' cursor
void draw_cursor(AscState state) {

    // Scale the x and y values to the border
    int posX, posY;

    posX = (int) ((state.x / SORTER_ACTUAL_WIDTH) * 28.0);
    posY = (int) ((state.y / SORTER_ACTUAL_HEIGHT) * 28.0);

    for (int i = -2; i < 3; i++) {

        // Draw the cross
        
        vTaskSuspendAll();
        ssd1306_DrawPixel(posX + i + 2, posY + 2, SSD1306_WHITE);
        ssd1306_DrawPixel(posX + 2, posY + i + 2, SSD1306_WHITE);
        xTaskResumeAll();
    } 
}

// Draws the outside border
void draw_box(void) {
    
    // Draw each side of the grid 
    for (int i = 0; i < GRID_PIXEL_SIDE; i++) {
        
        vTaskSuspendAll();
        // TOP LINE
        ssd1306_DrawPixel(i + 1, (SSD1306_HEIGHT - 1) - (GRID_PIXEL_SIDE), SSD1306_WHITE);

        // BOTTOM LINE
        ssd1306_DrawPixel(i + 1, (SSD1306_HEIGHT - 1), SSD1306_WHITE);

        // LEFT SIDE
        ssd1306_DrawPixel(0 + 1, i + ((SSD1306_HEIGHT - 1) - (GRID_PIXEL_SIDE)), SSD1306_WHITE);

        // RIGHT SIDE
        ssd1306_DrawPixel(GRID_PIXEL_SIDE - 1 + 1, i + ((SSD1306_HEIGHT - 1) - (GRID_PIXEL_SIDE)), SSD1306_WHITE);

        xTaskResumeAll();
    }
}