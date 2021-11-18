/**
***************************************************************
* @file     mylib/s4587423_cli_task.c
* @author   Peter Lahanas - 45874239
* @date     14052021
* @brief    CLI Task File
* REFERENCE: csse3010_mylib_reg_radio.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_tsk_cli_init(void) - creates a new cli task
***************************************************************
*/

#include "s4587423_cli_task.h"

/* FreeRTOS task configuration for stack size and priority*/
#define CLI_STACK_SIZE ( configMINIMAL_STACK_SIZE * 4 )
#define CLI_PRIORITY   ( tskIDLE_PRIORITY + 4 )

// The number of characters allocated to the input string command buffer
#define INPUT_STRING_BUFFER 100

// The PUTTY default encoding for the backspace key
#define PUTTY_BACKSPACE 0x7F

// Sets the buffer values to 0
#define CLEAR_BUFFER(SIZE, BUFFER) for (unsigned int i = 0; i < SIZE; i++) {BUFFER[i] = 0;}

// Output string buffer
#define BUFFER_SIZE 200

/* Holds a stream of bytes which can be pass from an ISR*/
StreamBufferHandle_t xStreamBuffer;

/* Prototypes of the the state update function and task*/
void s4587423TaskCli(void);
void write_asc_state(void);

/**
 * @brief Creates a new cli task
 * @param void
 * @return void
 **/
void s4587423_tsk_cli_init(void) {

    xTaskCreate((TaskFunction_t) &s4587423TaskCli, 
        (const signed char *) "CLI", CLI_STACK_SIZE, 
        NULL, CLI_PRIORITY, NULL);
}

/**
 * @brief The command line interface task
*/
void s4587423TaskCli(void) {
    
    // Initialise hardware for UART
    s4587423_reg_cli_init();

    // Init the state
    state.z = 30;
    joyState.x = 0;
    joyState.y = 0;
    joyState.z = 30;

    /* Create FREERTOS synchroniser variables used for the cli*/
    s4587423GroupEventCliState = xEventGroupCreate();
    s4587423QueuePacketMsg = xQueueCreate(10, sizeof(struct PacketMsg));
    s4587423QueueAscState = xQueueCreate(10, sizeof(AscState));
    s4587423QueueJoystick = xQueueCreate(10, sizeof(JoystickState));

    char cRxedChar; // bytes received from UART
    int inputIndex = 0;
    size_t xReceivedBytes; // Bytes received from the stream buffer
    BaseType_t xReturned;
    uint32_t ulNotifiedValue;

    // Stream buffer parameters
    const size_t xStreamBufferSizeBytes = 10, xTriggerLevel = 1;
    // Create stream buffer 
    xStreamBuffer = xStreamBufferCreate(xStreamBufferSizeBytes, xTriggerLevel);

    // REGISTER ALL THE CLI COMMANDS
    FreeRTOS_CLIRegisterCommand(&xGetSys);
    FreeRTOS_CLIRegisterCommand(&xSetTxAddr);
    FreeRTOS_CLIRegisterCommand(&xJoin);
    FreeRTOS_CLIRegisterCommand(&xJoystick);
    FreeRTOS_CLIRegisterCommand(&xOrigin);
    FreeRTOS_CLIRegisterCommand(&xMove);
    FreeRTOS_CLIRegisterCommand(&xHead);
    FreeRTOS_CLIRegisterCommand(&xVacuum);
    FreeRTOS_CLIRegisterCommand(&xRotate);
    FreeRTOS_CLIRegisterCommand(&xSys);
    FreeRTOS_CLIRegisterCommand(&xNew);
    FreeRTOS_CLIRegisterCommand(&xDel);

    // Create the input string buffer and initialise it
    char cInputString[INPUT_STRING_BUFFER];
    CLEAR_BUFFER(INPUT_STRING_BUFFER, cInputString);
    
    // Create the command ouput buffer
    char pcOutputString[BUFFER_SIZE];
    CLEAR_BUFFER(BUFFER_SIZE, pcOutputString);

    for (;;) {

        // Read characters from the UART BUFFER (sent from the computer)
        xReceivedBytes = xStreamBufferReceive(xStreamBuffer, (void *) &ulNotifiedValue, sizeof(ulNotifiedValue), pdMS_TO_TICKS(20));

        cRxedChar = (uint8_t) ulNotifiedValue;
        
        // Process if character if not Null
        if (xReceivedBytes > 0) {
            
            if (cRxedChar != '\0') {
                
                // Echo character
   	   	   	    WRITE_REG(UART_DEV->DR, (unsigned char) cRxedChar);

                // Wait for character to be transmitted.
                while((READ_REG(UART_DEV->SR) & USART_SR_TC) == 0);

               	// Process only if return is received. 
   	   	        if (cRxedChar == '\r') {

                    // Put null character in command input string. 
   	   	   	        cInputString[inputIndex] = '\0';
                    xReturned = pdTRUE;
    
                    // Process command input string. 
                    while (xReturned != pdFALSE) {

                        // Returns pdFALSE, when all strings have been returned 
                        xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, BUFFER_SIZE);

                        // Display CLI command output string (not thread safe)
                        portENTER_CRITICAL();
                        for (unsigned int i = 0; i < strlen(pcOutputString); i++) {

                            // Transmit 1 character
                            WRITE_REG(UART_DEV->DR, (unsigned char) (*(pcOutputString + i)));

                            // Wait for character to be transmitted.
        	            	while((READ_REG(UART_DEV->SR) & USART_SR_TC) == 0);

            	        }
                        
                        WRITE_REG(UART_DEV->DR, (unsigned char) ('\n'));
                        // Wait for character to be transmitted.
     	            	while((READ_REG(UART_DEV->SR) & USART_SR_TC) == 0);
                        
                        portEXIT_CRITICAL();

                        vTaskDelay(5);
                    }

                    CLEAR_BUFFER(INPUT_STRING_BUFFER, cInputString);
                    CLEAR_BUFFER(BUFFER_SIZE, pcOutputString);
                    inputIndex = 0;

                } else {

                    if( cRxedChar == '\r' ) {

                        // Ignore the character. 
                    } else if( cRxedChar == '\b' || cRxedChar == PUTTY_BACKSPACE ) {

                        // Backspace was pressed.  Erase the last character in the
                        // string - if any.
                        if( inputIndex > 0 ) {
                            inputIndex--;
                            cInputString[ inputIndex ] = '\0';
                        }

                    } else {

                        //  A character was entered.  Add it to the string
                        // entered so far.  When a \n is entered the complete
                        // string will be passed to the command interpreter.
                        if( inputIndex < 20 ) {
                            cInputString[ inputIndex ] = cRxedChar;
                            inputIndex++;
                        }
                    }
                }
            }
        }
        write_asc_state(); // update state
        vTaskDelay(20);
    }
}

/**
 * @brief Writes the ASC state to the OLED queue
 * @param void
 * @return void
 **/
void write_asc_state() {

    AscState currentState; // holds current asc state
    JoystickState joystickVals; // holds the current save joystick value

    // Check if the joystick is active 
    EventBits_t uxBits;
    uxBits = xEventGroupWaitBits(s4587423GroupEventCliState, JOYSTICK_STATE, pdFALSE, pdFALSE, 1);

    if ((uxBits & JOYSTICK_STATE) && s4587423QueueJoystick != NULL 
            && !(uxBits & JOYSTICK_STATE_Z)) { 
                
        // joystick is active and not in Z selection

        xQueueReceive(s4587423QueueJoystick, &joystickVals, 10); // receive joystick values
        currentState.x = (int) ((joystickVals.x / 4095.0) * 200.0);
        currentState.y = (int) ((joystickVals.y / 4095.0) * 200.0);
        currentState.angle = state.angle;
        currentState.z = state.z;
        
        if (s4587423SemaphorePb != NULL) {

            if (xSemaphoreTake(s4587423SemaphorePb, 1) == pdTRUE) {
            
                // button has been pressed so save the state
                joyState.x = currentState.x;
                joyState.y = currentState.y;
            }
        }
    } else if ((uxBits & JOYSTICK_STATE) && s4587423QueueJoystick != NULL) {
        
        // joystick is active and in Z selection mode
        xQueueReceive(s4587423QueueJoystick, &joystickVals, 10);
        currentState.x = state.x;
        currentState.y = state.y;
        currentState.angle = state.angle;
        currentState.z = (int) ((joystickVals.y / 4095.0) * 200.0);

        if (s4587423SemaphorePb != NULL) {

            if (xSemaphoreTake(s4587423SemaphorePb, 1) == pdTRUE) {
            
                // button has been pressed so save the state
                joyState.z = currentState.z;
            }
        }

    } else {
        
        currentState = state; // joystick not active
    }
   
    if (s4587423QueueAscState != NULL) {

        // update the state 
        xQueueSendToFront(s4587423QueueAscState, (void *) &currentState, (portTickType) 10);
    }
    
    
}

void Uart3_callback() {
    
    uint8_t rxChar;
  
    // Must be Initialised to pdFALSE!
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t ulPreviousValue;
    
    if ((UART_DEV->SR & USART_SR_RXNE) != 0) {
        
        //Receive character from data register (automatically clears flag when read occurs)
        rxChar = READ_REG(UART_DEV->DR);
        
        xStreamBufferSendFromISR(xStreamBuffer, &rxChar,1, &xHigherPriorityTaskWoken);
    }

}