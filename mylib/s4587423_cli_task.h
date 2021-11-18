#ifndef S4587423_CLI_TASK_H
#define S4587423_CLI_TASK_H

// STDIO functions
#include "processor_hal.h"
#include "board.h"
#include "stm32f429xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "FreeRTOS_CLI.h"

#include <string.h>

// CLI commands
#include "s4587423_cli_includes.h"

// Register functions for the cli
#include "s4587423_reg_cli.h"

// Cli task initialiser
void s4587423_tsk_cli_init(void);

// Interrupt handler
void Uart3_callback();

#endif