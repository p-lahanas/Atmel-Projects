/**
***************************************************************
* @file     mylib/s4587423_reg_cli.c
* @author   Peter Lahanas - 45874239
* @date     13052021
* @brief    Radio Register Driver
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_cli_init() - initialses the cli hardware
***************************************************************
*/

#include "s4587423_reg_cli.h"


void s4587423_reg_cli_init() {

    /*Initialise GPIO pins for TX and RX
        TX : PIN D1 -> PG14 
        RX : PIN D0 -> PG9
    */
    
    // Enable UART DEV GPIO clock
    UART_DEV_GPIO_CLK();
    
    //Clear and Set Alternate Function for pin (upper ARF register) 
    MODIFY_REG(UART_DEV_GPIO->AFR[1], ((0x0F) << ((UART_DEV_RX_PIN-8) * 4)) | ((0x0F) << ((UART_DEV_TX_PIN-8)* 4)), ((UART_DEV_GPIO_AF << ((UART_DEV_RX_PIN-8) * 4)) | (UART_DEV_GPIO_AF << ((UART_DEV_TX_PIN-8)) * 4)));

    //Clear and Set Alternate Function Push Pull Mode
    MODIFY_REG(UART_DEV_GPIO->MODER, ((0x03 << (UART_DEV_RX_PIN * 2)) | (0x03 << (UART_DEV_TX_PIN * 2))), ((GPIO_MODE_AF_PP << (UART_DEV_RX_PIN * 2)) | (GPIO_MODE_AF_PP << (UART_DEV_TX_PIN * 2))));

    //Set low speed.
    SET_BIT(UART_DEV_GPIO->OSPEEDR, (GPIO_SPEED_LOW << UART_DEV_RX_PIN) | (GPIO_SPEED_LOW << UART_DEV_TX_PIN));

    //Set Bit for Push/Pull output
    SET_BIT(UART_DEV_GPIO->OTYPER, ((0x01 << UART_DEV_RX_PIN) | (0x01 << UART_DEV_TX_PIN)));

    //Clear and set bits for no push/pull
    MODIFY_REG(UART_DEV_GPIO->PUPDR, (0x03 << (UART_DEV_RX_PIN * 2)) | (0x03 << (UART_DEV_TX_PIN * 2)), (GPIO_PULLUP << (UART_DEV_RX_PIN * 2)) | (GPIO_PULLDOWN << (UART_DEV_TX_PIN * 2)));

    // UART Settings - No hardware flow control, 8 data bits, no parity, 1 start bit and 1 stop bit		
    // Enable USART clock
    UART_DEV_CLK();

    CLEAR_BIT(UART_DEV->CR1, USART_CR1_UE);  // Disable USART

    // Configure word length to 8 bit
    CLEAR_BIT(UART_DEV->CR1, USART_CR1_M);   // M: 00 = 8 data bits, 01 = 9 data bits, 10 = 7 data bits

    // Configure oversampling mode: Oversampling by 16 
    CLEAR_BIT(UART_DEV->CR1, USART_CR1_OVER8);  // 0 = oversampling by 16, 1 = oversampling by 8

    SET_BIT(UART_DEV->CR1, USART_CR1_RE | USART_CR1_TE);  	// Transmitter and Receiver enable
    CLEAR_BIT(UART_DEV->CR1, USART_CR1_PCE);						//Disable Parity

    // Configure stop bits to 1 stop bit and siable clock output (USART mode only)
    //   00: 1 Stop bit;      01: 0.5 Stop bit
    //   10: 2 Stop bits;     11: 1.5 Stop bit   
    CLEAR_BIT(UART_DEV->CR2, USART_CR2_CPHA | USART_CR2_CPOL | USART_CR2_CLKEN | USART_CR2_LBCL | USART_CR2_STOP);

    // Set Baudrate to 115200 using APB frequency (80,000,000 Hz) and 16 bit sampling
    // NOTE: If using USART1 or USART6, HAL_RCC_GetPCLK2Freq must be used.
    WRITE_REG(UART_DEV->BRR, UART_BRR_SAMPLING16(HAL_RCC_GetPCLK1Freq(), UART_DEV_BAUD));

    //Disable handshaing signals
    CLEAR_BIT(UART_DEV->CR3, USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);

    //Enable receive interrupt
    SET_BIT(UART_DEV->CR1, USART_CR1_RXNEIE);

    //Enable priority (10) and interrupt callback. Do not set a priority lower than 5.
    HAL_NVIC_SetPriority(UART_DEV_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(UART_DEV_IRQn);

    SET_BIT(UART_DEV->CR1, USART_CR1_UE); // UART enable  
}