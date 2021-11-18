#include "main.h"


int main(void) {

    HAL_Init(); // initialise hardware abstraction

    // Create all the tasks
    s4587423_tsk_cli_init();
    s4587423_tsk_sys_init();
    s4587423_tsk_radio_init();
    s4587423_tsk_oled_init();
    s4587423_tsk_joystick_init();

    // Start the scheduler
    vTaskStartScheduler();

    return 0;
}