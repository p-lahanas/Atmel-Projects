#ifndef S4587423_TSK_RADIO_H
#define S4587423_TSK_RADIO_H

#include "board.h"
#include "processor_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "stream_buffer.h"

#include "s4587423_reg_radio.h"
#include "s4587423_cli_includes.h"

void s4587423_tsk_radio_init(void);
void s4587423_tsk_radio_deinit(void);
void Uart6_callback(void);

#endif