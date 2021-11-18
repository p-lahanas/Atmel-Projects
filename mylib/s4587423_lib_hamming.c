/**
***************************************************************
* @file     mylib/s4587423_lib_hamming.c
* @author   Peter Lahanas - 45874239
* @date     30032021
* @brief    Input Signal Synchroniser Register Driver
* REFERENCE: csse3010_mylib_lib_hamming.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* s4587423_lib_hamming_parity_error(uint8_t byte) - checks for a parity error
* s4587423_lib_hamming_byte_encode(uint8_t in) - encodes a byte of data
* s4587423_lib_hamming_byte_decode(uint8_t in) - decodes a bye of data
***************************************************************
*/

#include "s4587423_lib_hamming.h"

/**
 * Generator matrix
 * G = [ 0 1 1 | 1 0 0 0 ;
 *       1 0 1 | 0 1 0 0 ;
 *       1 1 0 | 0 0 1 0 ;
 *       1 1 1 | 0 0 0 1 ];
 *
 * H = [ 1 0 0 | 0 1 1 1 ;
 *       0 1 0 | 1 0 1 1 ;
 *       0 0 1 | 1 1 0 1 ];
 */


/**
 * @brief Checks if a parity error has occured
 * @param byte the byte of data to check for the parity
 * @return 1 if a parity occurs otherwise, 0
 */
int s4587423_lib_hamming_parity_error(uint8_t byte) {
    
    uint8_t receivedP = 0x01 & byte;
    uint8_t actualP = 0;

    for (int i = 1; i < 8; i++) {
        actualP ^= !!(byte & (1 << i));
    }

    if (receivedP != actualP) {
        return 1;
    } else {
        return 0;
    }
}


/**
 * @brief encodes half a byte of data
 * @param in the data to encode
 * @return the encoded data 
 */
uint8_t hamming_hbyte_encode(uint8_t in) {

    uint8_t d0, d1, d2, d3;
    uint8_t p = 0, h0, h1, h2;

    /* Extract the data bits*/
    d0 = !!(in & 0x01);
    d1 = !!(in & 0x02);
    d2 = !!(in & 0x04);
    d3 = !!(in & 0x08);

    /* Calculate the parity bits*/
    h0 = d1 ^ d2 ^ d3;
    h1 = d0 ^ d2 ^ d3;
    h2 = d0 ^ d1 ^ d3;

    /* LBit [P H0 H1 H2 D0 D1 D2 D3] */
    uint8_t out;
    out = (h0 << 1) +
          (h1 << 2) +
          (h2 << 3) +
          (d0 << 4) +
          (d1 << 5) +
          (d2 << 6) +
          (d3 << 7);

    for (int i = 1; i < 8; i++) {
        p ^= !!(out & (1 << i));
    }

    out |= p;

    return out;
}

/**
 * @brief encodes a byte of data
 * @param in the data to encode
 * @return the encoded data
 */
uint16_t s4587423_lib_hamming_byte_encode(uint8_t in) {

    uint8_t upper = hamming_hbyte_encode(in >> 4);
    uint8_t lower = hamming_hbyte_encode(in);
    uint16_t encodedByte = 0;

    encodedByte |= (upper << 8);
    encodedByte |= lower; 

    return encodedByte;
}

/**
 * @brief decodes a byte of data
 * @param in the data to decode
 * @return the decoded data
 */
uint8_t s4587423_lib_hamming_byte_decode(uint8_t in) {

    uint8_t h0, h1, h2, d0, d1, d2, d3, s0 = 0, s1 = 0, s2 = 0;
   
    h0 = !!(in & 0x02);
    h1 = !!(in & 0x04);
    h2 = !!(in & 0x08);
    d0 = !!(in & 0x10);
    d1 = !!(in & 0x20);
    d2 = !!(in & 0x40);
    d3 = !!(in & 0x80);

    /* Check each parity bit*/
    s2 = h2 ^ d3 ^ d1 ^ d0;
    s1 = h1 ^ d3 ^ d2 ^ d0;
    s0 = h0 ^ d1 ^ d2 ^ d3;

    /* LBIT [h0 h1 h2 d0 d1 d2 d3] 
    *  H = [1  0  0| 0  1  1  1 ;
    *       0  1  0| 1  0  1  1 ;
    *       0  0  1| 1  1  0  1 ]; */

    uint8_t syndrome = 0;
    // convert into decimal from binary representation
    syndrome = s0 + (2 * s1) + (4 * s2);
    
    if (syndrome == 0) {
        return (in); // no error
    }
   
    // Create a lookup table to determine which bit needs changing
    uint8_t lookup[] = {1, 2, 4, 6, 5, 3, 7};
    uint8_t flipBit = 0;

    for (int i = 0; i < 7; i++) {
        if (lookup[i] == syndrome) { // this is the wrong bit
            flipBit = i;
        }
    }
    
    // Flip the bit which is wrong (add one to skip p bit)
    uint8_t out = in ^ (1 << (flipBit + 1));
    return (out); 
}
