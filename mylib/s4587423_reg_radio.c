/**
***************************************************************
* @file     mylib/s4587423_reg_radio.c
* @author   Peter Lahanas - 45874239
* @date     05052021
* @brief    Radio Register Driver
* REFERENCE: csse3010_mylib_reg_radio.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_reg_radio_init() - initialise the radio hardware
* s4587423_create_packet_from_msg(struct PacketMsg msg, Packet* packet) - create a packet from given msg
* s4587423_transmit_packet(Packet* packet) - transmits the packet (over UART)
***************************************************************
*/

#include "s4587423_reg_radio.h"

/* TX and RX pins need to be switched for USART to work*/
#define UART_DEV_TX_PIN     9
#define UART_DEV_RX_PIN     14

#define UART_DEV_GPIO       GPIOG
#define UART_DEV_GPIO_AF    GPIO_AF8_USART6
#define UART_DEV_GPIO_CLK() __GPIOG_CLK_ENABLE()

#define UART_DEV       USART6
#define UART_DEV_CLK() __USART6_CLK_ENABLE()
#define UART_DEV_BAUD  9600  //NOTE: If using USART1 or USART6, HAL_RCC_GetPCLK2Freq must be used.
#define UART_DEV_IRQn  USART6_IRQn

void s4587423_reg_radio_init() {

    /*------------------------- UART HARDWARE INIT -------------------------->*/
    /*Initialise GPIO pins for TX and RX
        TX : PIN D1 -> PG14 
        RX : PIN D0 -> PG91
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
    WRITE_REG(UART_DEV->BRR, UART_BRR_SAMPLING16(HAL_RCC_GetPCLK2Freq(), UART_DEV_BAUD));

    //Disable handshaing signals
    CLEAR_BIT(UART_DEV->CR3, USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);

    //Enable receive interrupt
    SET_BIT(UART_DEV->CR1, USART_CR1_RXNEIE);

    //Enable priority (10) and interrupt callback. Do not set a priority lower than 5.
    HAL_NVIC_SetPriority(UART_DEV_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(UART_DEV_IRQn);

    SET_BIT(UART_DEV->CR1, USART_CR1_UE); // UART enable

    /* Enable GPIO for D2 (PF15) OUTPUT, D3 (PE13) INPUT, 
         D4 (PF14) INPUT, D5 (PE11) OUTPUT*/

    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();

    /* Clear and set modes */

    MODIFY_REG(GPIOF->MODER, ((0x03 << (2 * BRD_D2)) | (0x03 << (2 * BRD_D4))), (0x01 << (2 * BRD_D2)));
    MODIFY_REG(GPIOE->MODER, ((0x03 << (2 * BRD_D3)) | (0x03 << (2 * BRD_D5))), (0x01 << (2 * BRD_D5)));

    // Set outputs pins to push-pull
    CLEAR_BIT(GPIOF->OTYPER, (0x01 << BRD_D2));
    CLEAR_BIT(GPIOE->OTYPER, (0x01 << BRD_D5));

    // Set to fast speed
    MODIFY_REG(GPIOF->OSPEEDR, (0x03 << (2 * BRD_D2)), (GPIO_SPEED_FREQ_HIGH << (2 * BRD_D2)));
    MODIFY_REG(GPIOE->OSPEEDR, (0x03 << (2 * BRD_D3)), (GPIO_SPEED_FREQ_HIGH << (2 * BRD_D5)));

    // Set to pull-down or pull up
    MODIFY_REG(GPIOF->PUPDR, ((0x03 << (2 * BRD_D2)) | (0x03 << (2 * BRD_D4))), ((GPIO_PULLDOWN << (2 * BRD_D2)) | (GPIO_PULLDOWN << (2 * BRD_D4))));
    MODIFY_REG(GPIOE->PUPDR, ((0x03 << (2 * BRD_D3)) | (0x03 << (2 * BRD_D5))), ((GPIO_PULLDOWN << (2 * BRD_D3)) | (GPIO_PULLUP << (2 * BRD_D5))));

}

void uart_send_byte(uint8_t sendByte) {
   
    // Transmit 1 character
    WRITE_REG(UART_DEV->DR, (unsigned char) sendByte);
    // Wait for character to be transmitted.
    while((READ_REG(UART_DEV->SR) & USART_SR_TC) == 0);
}

/* Convert integer 'num' to a string and store in mem*/
void int_to_string(int num, char* mem) {

    uint8_t isNegative = 0;
    int i = 0;
    /* Handle the 0 case explicitely*/
    if (num == 0) {
        mem[0] = '0';
        mem[1] = '\0';
    }

    /* Negative case*/
    if (num < 0) {
        num = -1 * num;
        isNegative = 1; //set negative flag
    }

    while (num > 0) {
        int rem = num % 10;
        mem[i++] = rem + '0';
        num = num / 10;
    }

    if (isNegative) {
        mem[i++] = '-'; //append minus sign
    }

    // REVERSE THE STRING
    char buffer[PAYLOAD_SIZE]; // string cannot be bigger than the payload size

    for (int j = 0; j < i; j++) {
        buffer[j] = mem[i - j - 1]; //subtract 2 as i is one space along
    }
    // Copy contents across
    for (int j = 0; j < i; j++) {
        mem[j] = buffer[j];
    }
    mem[i] = '\0';
}



void payload_from_packetmsg(struct PacketMsg msg, Packet* packet) {
    
    char msgType = msg.packetType;

    // Set payload to 0 so 0 padded
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        // pad with 0    
        packet->payload[i] = 0;
    }

    // message buffers
    char bufferx[5];
    char buffery[5]; 
    char bufferz[5]; 

    // Form messages based on message type
    switch (msgType) {

        case JOIN_TYPE:
           
            strcpy(packet->payload, "JOIN");
            break;

        case XYZ_TYPE:
            
            int_to_string(msg.x, bufferx);
            int_to_string(msg.y, buffery);
            int_to_string(msg.z, bufferz);
            sprintf(packet->payload, "XYZ%03s%03s%02s", bufferx, buffery, bufferz);
            break;
        
        case ROT_TYPE:
          
            sprintf(packet->payload, "ROT%03d", msg.angle);
            break;

        case VAC_TYPE:
         
            if (msg.pumpOn) {
                strcpy(packet->payload, "VON");
                break;
            }
            strcpy(packet->payload, "VOFF");
            break;
    }
    

}

/** 
 * @brief Creates a new Packet from the given packet message
 * @param msg the packet message to turn into a packet
 * @param packet a pointer to a packet to store the packet information
 **/
void s4587423_create_packet_from_msg(struct PacketMsg msg, Packet* packet) {

    packet->preamble = (char) 0xAA;
    packet->type = msg.packetType;
    packet->senderAddr = SENDERADDR;
    packet->receiverAddr = msg.recAddr;

    // create the payload for the packet
    payload_from_packetmsg(msg, packet);

}

void s4587423_transmit_packet(Packet* packet) {
    
    //transmit packet 1 byte at a time
    uart_send_byte(packet->preamble);
    uart_send_byte(packet->type);

    // ADDRESS IS SENT IN BIG ENDIAN ORDER
    uart_send_byte((uint8_t) (packet->senderAddr >> (6 * 4))); // *4 as hex shift
    uart_send_byte((uint8_t) ((packet->senderAddr >> (4 * 4)) & 0x000000FF));
    uart_send_byte((uint8_t) ((packet->senderAddr >> (2 * 4)) & 0x000000FF));
    uart_send_byte((uint8_t) ((packet->senderAddr) & 0x000000FF));

    // ADDRESS IS SENT IN BIG ENDIAN ORDER
    uart_send_byte((uint8_t) (packet->receiverAddr >> (6 * 4)));
    uart_send_byte((uint8_t) ((packet->receiverAddr >> (4 * 4)) & 0x000000FF));
    uart_send_byte((uint8_t) ((packet->receiverAddr >> (2 * 4)) & 0x000000FF));
    uart_send_byte((uint8_t) ((packet->receiverAddr) & 0x000000FF));

    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        uart_send_byte(packet->payload[i]);
    }

}