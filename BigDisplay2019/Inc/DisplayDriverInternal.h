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
