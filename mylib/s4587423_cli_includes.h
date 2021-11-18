#ifndef S4587423_CLI_INCLUDES_H
#define S4587423_CLI_INCLUDES_H

#include "processor_hal.h"
#include "stm32f4xx_hal.h"
#include "board.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"

#include "s4587423_tsk_sys.h"
#include "s4587423_reg_radio.h"

/*---------------------------- CLI SHELL COMMANDS ----------------------------*/
CLI_Command_Definition_t xGetSys, xSetTxAddr, xJoin, xJoystick;

/*---------------------------- ASC SHELL COMMANDS ----------------------------*/
CLI_Command_Definition_t xOrigin, xMove, xHead, xVacuum, xRotate, 
        xSys, xNew, xDel;

// Push button semaphore (updates the joystick state)
SemaphoreHandle_t s4587423SemaphorePb;

// Event Group (For CLI status)
EventGroupHandle_t s4587423GroupEventCliState;

// Queue for Packets
QueueHandle_t s4587423QueuePacketMsg;

// Queue for OLED position
QueueHandle_t s4587423QueueAscState;

// Queue for joystick X, Y
QueueHandle_t s4587423QueueJoystick;

// Receiver Address
uint32_t s4587423ReceiverAddress;

// Joystick state bit in event group
#define JOYSTICK_STATE 0x01 << 0

// Ack packet received from xenon (flag)
#define JOIN_ACK_REC 0x01 << 1

// Bit is set when joystick is in Z mode
#define JOYSTICK_STATE_Z 0x01 << 2

typedef struct AscState {

    int x; // Current x coordinate
    int y; // Current y coordinate
    int z; // Current z coordinate 
    int angle; // Current rotation angle

} AscState;

// State of the Sorting Machine (for the cli task)
AscState state;

// Holds the current state of the joystick (saves joystick values)
typedef struct JoystickState {
    
    int x;
    int y;
    int z;

} JoystickState;


// State of the Joystick for the Sorting machine (for the cli task)
JoystickState joyState;

#endif