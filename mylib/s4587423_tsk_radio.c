/**
***************************************************************
* @file     mylib/s4587423_tsk_oled.c
* @author   Peter Lahanas - 45874239
* @date     17052021
* @brief    Radio Task File
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_tsk_radio_init(void) - creates a new radio task
* s4587423_tsk_radio_deinit(void) - destroys a radio task
***************************************************************
*/

#include "s4587423_tsk_radio.h"

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define RADIO_STACK_SIZE    ( configMINIMAL_STACK_SIZE * 2 )

// Task Priorities (Idle Priority is the lowest priority
#define RADIO_PRIORITY    ( tskIDLE_PRIORITY + 3 )

/* Holds a stream of bytes which can be passed from an ISR*/
StreamBufferHandle_t xStreamBuffer2;

/* Create a task handle for the radio task*/
TaskHandle_t radioTaskHandle = NULL;

void s4587423TaskRadio(void);

/**
 * @brief Creates a new radio task
 * @param void
 * @return void
*/
void s4587423_tsk_radio_init(void) {

    xTaskCreate((TaskFunction_t) &s4587423TaskRadio, 
            (const signed char *) "RADIO", RADIO_STACK_SIZE, 
            NULL, RADIO_PRIORITY, &radioTaskHandle);
}

/**
 * @brief Removes the current radio task
 * @param void
 * @return void
*/
void s4587423_tsk_radio_deinit(void) {
    
    vTaskDelete(radioTaskHandle);
    radioTaskHandle = NULL;
}

/* The radio task*/
void s4587423TaskRadio(void) {

    s4587423_reg_radio_init();

    /* Create data types to send messages*/
    struct PacketMsg receivedMsg;
    Packet packet;

    /* Create the stream buffer*/ 
    const size_t xStreamBufferSizeBytes = 10, xTriggerLevel = 1;
    xStreamBuffer2 = xStreamBufferCreate(xStreamBufferSizeBytes, xTriggerLevel);
    size_t xReceivedBytes; // Bytes received from the stream buffer
    uint32_t ulNotifiedValue;
    uint8_t preambleRec = 0;

    for (;;) {

        // Check if a packet msg is on the queue        
        if (s4587423QueuePacketMsg != NULL) {
            
            if (xQueueReceive(s4587423QueuePacketMsg, &receivedMsg, 10)) {

                // Set to transmit mode
                S4587423_REG_RADIO_TXMODE_SET(); 
                // Form the packet from the message
                s4587423_create_packet_from_msg(receivedMsg, &packet);
                s4587423_transmit_packet(&packet);

                // back to receive mode
                S4587423_REG_RADIO_TXMODE_CLR();
            }
        }

        // Read characters from the UART BUFFER (sent from the xenon)
        xReceivedBytes = xStreamBufferReceive(xStreamBuffer2, (void *) &ulNotifiedValue, sizeof(ulNotifiedValue), 0);

        if (xReceivedBytes > 0) {

            if (s4587423GroupEventCliState != NULL) {
                       
                xEventGroupSetBits(s4587423GroupEventCliState, JOIN_ACK_REC);
            }
            uint8_t cRxedVal = (uint8_t) ulNotifiedValue;
        }
        vTaskDelay(50);
    }
}

/* Uart 6 callback (received from the xenon) */
void Uart6_callback(void) {

    uint8_t rxChar;
  
    // Must be Initialised to pdFALSE!
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t ulPreviousValue;
    
    if ((USART6->SR & USART_SR_RXNE) != 0) {
        
        //Receive character from data register (automatically clears flag when read occurs)
        rxChar = READ_REG(USART6->DR);

        xStreamBufferSendFromISR( xStreamBuffer2, &rxChar, 1, &xHigherPriorityTaskWoken );
    }

}
