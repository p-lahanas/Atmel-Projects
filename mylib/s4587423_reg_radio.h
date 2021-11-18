#ifndef S4587423_REG_RADIO_H
#define S4587423_REG_RADIO_H

#include "board.h"
#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"

#include <string.h>
#include <stdbool.h>

#define SENDERADDR 0x45874239

/* Set D2 Pin*/
#define S4587423_REG_RADIO_TXMODE_SET() BRD_D2_GPIO_PORT->ODR |= (0x01 << BRD_D2)

/* Clear D2 Pin*/
#define S4587423_REG_RADIO_TXMODE_CLR() BRD_D2_GPIO_PORT->ODR &= ~(0x01 << BRD_D2)

/* Get error status bit*/
#define S4587423_REG_RADIO_ERRSTATUS_GET() (BRD_D3_GPIO_PORT->IDR & (0x01 << BRD_D3)

#define S4587423_REG_RADIO_TXBUFFSTATUS_GET() (BRD_D4_GPIO_PORT->IDR & (0x01 << BRD_D4)

#define PAYLOAD_SIZE 23

/* PACKET TYPES */
#define JOIN_TYPE 0x20
#define ACK_TYPE 0x21
#define XYZ_TYPE 0x22
#define ROT_TYPE 0x23
#define VAC_TYPE 0x24
#define PREAMBLE 0xAA

struct PacketMsg {
    char packetType; // the type of the packet
    int x; // x coordinate for the sorter position (if XYZ packet)
    int y;
    int z;
    int angle;
    uint32_t recAddr;
    bool pumpOn;

};

typedef struct Packet {

    char preamble; 
    char type; // packet type (JOIN, ACK, etc..)
    uint32_t senderAddr; // 32 bit address
    uint32_t receiverAddr; // 32 bit address
    char payload[PAYLOAD_SIZE]; // 23 bytes payload (string)
} Packet;

void s4587423_reg_radio_init();
void s4587423_create_packet_from_msg(struct PacketMsg msg, Packet* packet);
void s4587423_transmit_packet(Packet* packet);

#endif