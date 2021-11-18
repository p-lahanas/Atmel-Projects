#ifndef S4587423_TSK_OLED_H
#define S4587423_TSK_OLED_H

#include "board.h"
#include "processor_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "s4587423_reg_oled.h"
#include "oled_pixel.h"
#include "oled_string.h"
#include "fonts.h"
#include "s4587423_cli_includes.h"


extern void s4587423_tsk_oled_init();
extern void s4587423_tsk_oled_deinit(void);

/* Queue to communicate to Oled task*/
QueueHandle_t s4587423QueueOledMsg;

/* Struct which holds the required information for the oled to update*/
struct oledTextMsg {
    
    int startX;
    int startY;
    char displayText[20];
};

#endif