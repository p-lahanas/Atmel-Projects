#ifndef S4587423_LIB_HAMMING_H
#define S4587423_LIB_HAMMING_H

#include "board.h"
#include "processor_hal.h"

extern int s4587423_lib_hamming_parity_error(uint8_t byte);
extern uint16_t s4587423_lib_hamming_byte_encode(uint8_t in);
extern uint8_t s4587423_lib_hamming_byte_decode(uint8_t in);

#endif