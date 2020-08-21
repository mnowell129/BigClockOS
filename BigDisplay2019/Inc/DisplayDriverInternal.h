/**
 * No warrantees or liability is assumed with this software. 
 * This software is the property of Charles M. Nowell, Jr. 
 * It is placed in the public domain for personal use only. 
 * All commercial rights are reserved. 
 * You may use this software to make a display(s) for yourself 
 * or for a model club. You may not use this software to make a 
 * product for sale, whether for profit or not. 
 * Charles M. Nowell, Jr 
 * 2-Jan-2020 
 * mickeysbigtimer@gmail.com 
 *  
 */
#ifndef DISPLAYDRIVERINTERNAL_H
#define DISPLAYDRIVERINTERNAL_H
/***************************************************************************************/
/***************************************************************************************/
/* Macros */
/***************************************************************************************/
/***************************************************************************************/
#define BANK_PORT           GPIOA
#define BANK_CLEAR_MASK     (BIT4|BIT5|BIT6|BIT7)
#define BANK(x)             BANK_PORT->ODR = (BANK_PORT->ODR & ~BANK_CLEAR_MASK) | ((x) << 4)

#define PIXEL(a,b,c,d)  ((a<<9)|(b<<6)|(c<<3)|(d))

#define COLOR_PORT          GPIOC
#define COLOR(x)            COLOR_PORT->ODR = x;


#define NUMPIXELS_1x2_ONE_EIGHTH_SCAN       (32*4)   // 320
#define NUMPIXELS_1x2x3_ONE_EIGHTH_SCAN     (32 * 12)  // 256
// #define ONE_EIGHT_SCAN_ELECTRONIC_ROWS      (8)

#define NUMPIXELS_2x5_ONE_EIGHTH_SCAN       (32*10)   // 320
#define NUMPIXELS_1x4_ONE_EIGHTH_SCAN       (32 * 8)  // 256
#define NUMPIXELS_2x5_ONE_SIXTEENTH_SCAN    (32*5)    // 160
#define NUMPIXELS_1x4_ONE_SIXTEENTH_SCAN    (32 * 4)  // 128
#define ONE_EIGHT_SCAN_ELECTRONIC_ROWS      (8)
#define ONE_SIXTEENTH_SCAN_ELECTRONIC_ROWS  (16)
#define FRAMES                              (2)



#define INVERTED
#undef  INVERTED


#endif
