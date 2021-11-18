/**
***************************************************************
* @file     mylib/s4587423_reg_cli.c
* @author   Peter Lahanas - 45874239
* @date     13052021
* @brief    Radio Register Driver
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
*/

#include "s4587423_cli_includes.h"
#include "s4587423_tsk_joystick.h"
#include "s4587423_tsk_oled.h"
#include "s4587423_tsk_radio.h"

/* Defines symbols for each state that the task can be in
    r - Ready
    R - Running
    B - Blocking
    S - Suspended
    D - Deleted
    I - Invalid
*/
char taskState[6] = {'r', 'R', 'B', 'S', 'D', 'I'};

/* Function prototypes for shell commands*/
static BaseType_t prvSetTxAddr(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvJoin(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvGetSys(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvJoystick(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvSys(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvNew(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvDel(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);


/*------------------ getsys -------------------->*/
CLI_Command_Definition_t xGetSys = {
    "getsys",
    "getsys:\r\nPrint the current system time (seconds).\r\n",
    prvGetSys,
    0
};

static BaseType_t prvGetSys(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    // Writes the system time to the buffer 
    xWriteBufferLen = sprintf((char *) pcWriteBuffer, "\n\r%d\n\r", s4587423_get_system_time_count());

    return pdFALSE;
}

/*------------------ settxaddr -------------------->*/
CLI_Command_Definition_t xSetTxAddr = {
    "settxaddr",
    "settxaddr:\r\nSet radio TX address to TXaddr in HEX.\r\n",
    prvSetTxAddr,
    1
};

/**
 * @brief Calculates the powers of 16
 * @param pow the power of which to raise 16 to
 * @return res the result of the operation
 **/
uint32_t power_16(int pow) {
    
    uint32_t res = 1;

    for (int i = 0; i < pow; i++) {
        res = res * 16;
    }

    return res;
}

static BaseType_t prvSetTxAddr(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {
    
    long lParam_len;
    const char *cParameter;

    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    uint32_t total = 0;
    int paramLen = strlen(cParameter);

    // Go from right to left
    for (int i = (paramLen - 1); i >= 0; i--) {
        // convert from char to number and then multiply by postion
        total += ((cParameter[i] - '0') * (power_16(paramLen - i - 1)));
    }

    s4587423ReceiverAddress = total;

    return pdFALSE;
}

/*------------------ join -------------------->*/
CLI_Command_Definition_t xJoin = {
    "join",
    "join:\r\nSends a join packet to a Sorter, to ensure exclusive use of the Sorter by the sender.\r\n",
    prvJoin,
    0
};

static BaseType_t prvJoin(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    struct PacketMsg msg;

    if (s4587423QueuePacketMsg != NULL) {

        // create join message packet 
        msg.packetType = JOIN_TYPE;
        msg.recAddr = s4587423ReceiverAddress;
        xQueueSendToFront(s4587423QueuePacketMsg, (void *) &msg, (portTickType) 10);
    }

    return pdFALSE;
}

/*------------------ joystick -------------------->*/
CLI_Command_Definition_t xJoystick = {
    "joystick",
    "joystick:\r\nEnable '1' or disable '0' (default) joystick control.\r\n",
    prvJoystick,
    1
};

static BaseType_t prvJoystick(char *pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString) {

    long lParam_len;
    const char *cParameter;

    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if (s4587423GroupEventCliState != NULL) {
        // Check that param_len is 1 character
        if (*cParameter == '0') {

            // disable the joystick (set event group bit to 0)
            xEventGroupClearBits(s4587423GroupEventCliState, JOYSTICK_STATE);


        } else if (*cParameter == '1') {

            // enable the joystick (set event group bit to 1)
            xEventGroupSetBits(s4587423GroupEventCliState, JOYSTICK_STATE);
        } else {
            sprintf(pcWriteBuffer, "Invalid parameter.\r\n");
        }
    } 
    return pdFALSE;
}


/*------------------ sys -------------------->*/
CLI_Command_Definition_t xSys = {
    "sys",
    "sys:\r\nDisplay the current tasks.\r\n",
    prvSys,
    0
};

void get_task_info(char *pcWriteBuffer);

static BaseType_t prvSys(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {

    get_task_info(pcWriteBuffer);

    return pdFALSE;
}

void get_task_info(char *pcWriteBuffer) {

    TaskStatus_t *pxTaskStatusArray;
  
    volatile UBaseType_t arraySize;
   
    unsigned long ulTotalRunTime, ulStatsAsPercentage;

    /* Get the number of running tasks*/
    arraySize = uxTaskGetNumberOfTasks();

    /* Allocate a TaskStatus_t structure for each task.  An array could be
    allocated statically at compile time. */
    pxTaskStatusArray = pvPortMalloc(arraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL) {

       /* Generate raw status information about each task. */
        arraySize = uxTaskGetSystemState(pxTaskStatusArray,
                                  arraySize,
                                  &ulTotalRunTime);
         
        sprintf(pcWriteBuffer, "  name   id  state  priority stackMem\r\n");
        pcWriteBuffer += strlen((char *) pcWriteBuffer);
        
        for (unsigned int i = 0; i < arraySize; i++) {
            // Format the printing in columns
            sprintf(pcWriteBuffer, "%8s  %1d  %4c  %7d  %7d\r\n",
                    pxTaskStatusArray[i].pcTaskName,
                    pxTaskStatusArray[i].xTaskNumber,
                    taskState[pxTaskStatusArray[i].eCurrentState],
                    pxTaskStatusArray[i].uxCurrentPriority,
                    pxTaskStatusArray[i].usStackHighWaterMark); 
            pcWriteBuffer += strlen((char *) pcWriteBuffer);
        }
    }

    /* The array is no longer needed, free the memory it consumes. */
    vPortFree(pxTaskStatusArray);
}

/*------------------ new -------------------->*/
CLI_Command_Definition_t xNew = {
    "new",
    "new:\r\nCreate a new task.\r\n",
    prvNew,
    1
};

static BaseType_t prvNew(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {

    // Stores parameter info
    long lParam_len;
    const char *cParameter;

    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);
    
    /* Check parameter to see if it's with each task*/ 
    if (!strcmp(cParameter, "joystick")) {
      
        s4587423_tsk_joystick_init();
    } else if (!strcmp(cParameter, "oled")) {
       
        s4587423_tsk_oled_init();
    } else if (!strcmp(cParameter, "radio")) {
        
        s4587423_tsk_radio_init();
    } else {
     
        sprintf(pcWriteBuffer, "Invalid task\r\n");
    } 
    

    return pdFALSE;
}

/*------------------ del -------------------->*/
CLI_Command_Definition_t xDel = {
    "del",
    "del:\r\nEnd a task.\r\n",
    prvDel,
    1
};

static BaseType_t prvDel(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    
    long lParam_len;
    const char *cParameter;

    TaskHandle_t handle = NULL;
    // Select first parameter to command (0 indexed)
    cParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    // Check what task name the user input
    if (!strcmp(cParameter, "joystick")) {

        handle = xTaskGetHandle("JOYSTICK");
        vTaskDelete(handle);
    } else if (!strcmp(cParameter, "oled")) {

        handle = xTaskGetHandle("OLED");
        vTaskDelete(handle);
    } else if (!strcmp(cParameter, "radio")) {

        handle = xTaskGetHandle("RADIO");
        vTaskDelete(handle);
    } else {

        handle = xTaskGetHandle(cParameter); // try get a task with that name

        if (handle == NULL) { // check a task was found

            sprintf(pcWriteBuffer, "Invalid task\r\n");
        } else {

            vTaskDelete(handle); // remove task if found
        }
    }

    return pdFALSE;
}