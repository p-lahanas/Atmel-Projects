#ifndef S4587423_TSK_SYS_H
#define S4587423_TSK_SYS_H

#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "processor_hal.h"

#include "s4587423_cli_includes.h"

void s4587423_tsk_sys_init(void);
void s4587423_tsk_sys_deinit(void);
uint32_t s4587423_get_system_time_count(void);


#endif