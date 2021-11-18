/**
***************************************************************
* @file     mylib/s4587423_cli_asc.c
* @author   Peter Lahanas - 45874239
* @date     18052021
* @brief    Radio Register Driver
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
*/

#include "s4587423_cli_includes.h"

/* Command protypes */
static BaseType_t prvOrigin(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvMove(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvHead(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvVacuum(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvRotate(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

// Converts the given string to an integer
int stringToInt(const char* str, int len) {
    
    int val = 0, i = 0;
    bool isNegative = false;

    // Make sign negative at the end
    if (str[0] == '-') {

        isNegative = true;
        i++;
    }

    // do it for each letter in the string 
    while (i < len) {
        val = val * 10 + str[i] - '0';
        i++;
    }

    // flip sign if was originally negative 
    if (isNegative) {
        val = val * (-1);
    }
 
    return val;
}

/*------------------ origin -------------------->*/
CLI_Command_Definition_t xOrigin = {
    "origin",
    "origin:\r\nSend an XYZ packet to move the Sorter's head to [0,0,0].\r\n",
    prvOrigin,
    0
};

static BaseType_t prvOrigin(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    struct PacketMsg msg;

    if (s4587423QueuePacketMsg != NULL) { // check msg queue is created

        // create package message        
        msg.packetType = XYZ_TYPE;
        msg.x = 0;
        msg.y = 0;
        msg.z = 0;
        msg.recAddr = s4587423ReceiverAddress;

        // update the current state
        state.x = 0;
        state.y = 0;
        state.z = 0;
        xQueueSendToFront(s4587423QueuePacketMsg, (void *) &msg, (portTickType) 10);
    }

    return pdFALSE;
}

/*------------------ move -------------------->*/
CLI_Command_Definition_t xMove = {
    "move",
    "move \r\nmove the Sorter's head to location (x,y) in mm.\r\n",
    prvMove,
    2
};

static BaseType_t prvMove(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    struct PacketMsg msg;
    long lParam_len, lParam_len2;
    const char *cParameter;
    const char *cParameter2;

    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);
    cParameter2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lParam_len2);

    char bufferX[5];

    //remove comma from first parameter
    for(int i = 0; i < 5; i++) {
        if (cParameter[i] == ',') {

            bufferX[i] = '\0'; // set it to end of string
            break;
        } else {
            bufferX[i] = cParameter[i];
        }
    }

    // Convert values to integer to check bounds 
    int x = stringToInt(bufferX, strlen(bufferX));
    int y = stringToInt(cParameter2, lParam_len2);

    // Check bounds
    if (x > 200 || y > 200 || x < -1 || y < -1) {
        sprintf(pcWriteBuffer, "Invalid Position\r\n");
        return pdFALSE;
    }

    // Update message contents and state
    if (s4587423QueuePacketMsg != NULL) {
        
        if (x == -1 && y == -1) {

            msg.x = joyState.x;
            msg.y = joyState.y;
            msg.z = joyState.z;

        } else {

            msg.x = x;
            msg.y = y;
            msg.z = state.z;
        }

        msg.packetType = XYZ_TYPE;
        state.x = msg.x;
        state.y = msg.y;
        state.z = msg.z;
        
        msg.recAddr = s4587423ReceiverAddress;

        xQueueSendToFront(s4587423QueuePacketMsg, (void *) &msg, (portTickType) 10);
    }

    return pdFALSE;
}

/*------------------ head -------------------->*/
CLI_Command_Definition_t xHead = {
    "head",
    "head:\r\nraise or lower the Sorter's head\r\n",
    prvHead,
    1
};

static BaseType_t prvHead(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    long lParam_len;
    const char *cParameter;

    struct PacketMsg msg;

    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);
    msg.packetType = XYZ_TYPE;
    
    // keep asc head at current position
    msg.x = state.x;  
    msg.y = state.y;
    msg.recAddr = s4587423ReceiverAddress;
   
    // Check whether param is raise or lower
    if (strcmp(cParameter, "raise") == 0) {

        msg.z = 30;
        state.z = 30;

    } else if (strcmp(cParameter, "lower") == 0) {

        msg.z = 99;
        state.z = 99;
    } else {

        sprintf(pcWriteBuffer, "Invalid parameter ('raise' or 'lower').\r\n");
        return pdFALSE;
    } 
    
    if (s4587423QueuePacketMsg != NULL) {
        xQueueSendToFront(s4587423QueuePacketMsg, (void *) &msg, (portTickType) 10);
    }

    return pdFALSE;
}

/*------------------ vacuum -------------------->*/
CLI_Command_Definition_t xVacuum = {
    "vacuum",
    "vacuum:\r\nturn on or off the vacuum pump. On setting will grip the object and off will release it\r\n",
    prvVacuum,
    1
};

static BaseType_t prvVacuum(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    long lParam_len;
    const char *cParameter;

    struct PacketMsg msg;

    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    // Start filling msg info
    msg.packetType = VAC_TYPE;
    msg.recAddr = s4587423ReceiverAddress;
   
    // Check whether param is raise or lower
    if (strcmp(cParameter, "on") == 0) {

        msg.pumpOn = true;

    } else if (strcmp(cParameter, "off") == 0) {

        msg.pumpOn = false;
    } else {
        sprintf(pcWriteBuffer, "Invalid argument ('off' or 'on')\r\n");
        return pdFALSE;
    }

    if (s4587423QueuePacketMsg != NULL) {
        xQueueSendToFront(s4587423QueuePacketMsg, (void *) &msg, (portTickType) 10);
    }
    return pdFALSE;
}

/*------------------ rotate -------------------->*/
CLI_Command_Definition_t xRotate = {
    "rotate",
    "rotate:\r\nrotate the Sorter’s head by a relative angle (in◦)\r\n",
    prvRotate,
    1
};

static BaseType_t prvRotate(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    struct PacketMsg msg;
    long lParam_len;
    const char *cParameter;

    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if (s4587423QueuePacketMsg != NULL) {
         
        msg.packetType = ROT_TYPE;
        msg.angle = stringToInt(cParameter, lParam_len);

        if (msg.angle > 180 || msg.angle < 0) {
            sprintf(pcWriteBuffer, "Invalid angle\r\n");
            return pdFALSE;
        }

        state.angle = msg.angle;
        msg.recAddr = s4587423ReceiverAddress;

        xQueueSendToFront(s4587423QueuePacketMsg, (void *) &msg, (portTickType) 10);
    }

    return pdFALSE;
}