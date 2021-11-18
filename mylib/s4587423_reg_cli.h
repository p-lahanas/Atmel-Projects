#ifndef S4587423_REG_CLI_H
#define S4587423_REG_CLI_H

#include "board.h"
#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"

/*------------------------ UART PIN DEFINITIONS ----------------------------- */
#define UART_DEV_TX_PIN	    9
#define UART_DEV_RX_PIN	    8
#define UART_DEV_GPIO       GPIOD
#define UART_DEV_GPIO_AF    GPIO_AF7_USART3
#define UART_DEV_GPIO_CLK()	__GPIOD_CLK_ENABLE()

#define UART_DEV        USART3
#define UART_DEV_CLK()  __USART3_CLK_ENABLE()
#define UART_DEV_BAUD   9600 
#define UART_DEV_IRQn   USART3_IRQn


void s4587423_reg_cli_init();

#endif