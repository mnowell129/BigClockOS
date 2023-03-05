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
/**
 * Macroize the pin definitions.
 */
#ifndef IOCONFIGURATION_H
#define IOCONFIGURATION_H

#define NA                                      (0xFF)
#define LL_GPIO_AF_NA  LL_GPIO_AF_0

// PA6 
// PA7 
// PA8 
// PA9 

#define RS485TX                      RS485TX // needed for macros to work
#define RS485TX_PORT                 A
#define RS485TX_BUS                  AHB1
#define RS485TX_GRP                  1
#define RS485TX_PIN_NUMBER           8

#define RS485TX_MODE                 LL_GPIO_MODE_OUTPUT
#define RS485TX_AF                   LL_GPIO_AF_7
#define RS485TX_DEFAULT              BIT_LOW
#define RS485TX_ASSERTED             ACTIVE_HIGH
#define RS485TX_DEASSERTED           ACTIVE_LOW
#define RS485TX_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define RS485TX_PUPD                 (LL_GPIO_PULL_NO) 
#define RS485TX_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define RS485TX_GROUP                PORT(RS485TX_PORT)
#define RS485TX_BUS_ID               GROUPID(RS485TX_PORT,RS485TX_GRP)
#define RS485TX_PIN                  PIN(RS485TX_PIN_NUMBER)


#define RS485RX                      RS485RX // needed for macros to work
#define RS485RX_PORT                 B
#define RS485RX_BUS                  AHB1
#define RS485RX_GRP                  1
#define RS485RX_PIN_NUMBER           10

#define RS485RX_MODE                 LL_GPIO_MODE_OUTPUT
#define RS485RX_AF                   LL_GPIO_AF_NA
#define RS485RX_DEFAULT              BIT_LOW
#define RS485RX_ASSERTED             ACTIVE_LOW
#define RS485RX_DEASSERTED           ACTIVE_HIGH
#define RS485RX_GPIO_OTYPE           (LL_GPIO_OUTPUT_OPENDRAIN)
#define RS485RX_PUPD                 (LL_GPIO_PULL_NO) 
#define RS485RX_SPEED                (LL_GPIO_SPEED_FREQ_LOW)

#define RS485RX_GROUP                PORT(RS485RX_PORT)
#define RS485RX_BUS_ID               GROUPID(RS485RX_PORT,RS485RX_GRP)
#define RS485RX_PIN                  PIN(RS485RX_PIN_NUMBER)


#define XBEETX                      XBEETX // needed for macros to work
#define XBEETX_PORT                 A
#define XBEETX_BUS                  AHB1
#define XBEETX_GRP                  1
#define XBEETX_PIN_NUMBER           2

#define XBEETX_MODE                 LL_GPIO_MODE_OUTPUT
#define XBEETX_AF                   LL_GPIO_AF_7
#define XBEETX_DEFAULT              BIT_LOW
#define XBEETX_ASSERTED             ACTIVE_HIGH
#define XBEETX_DEASSERTED           ACTIVE_LOW
#define XBEETX_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define XBEETX_PUPD                 (LL_GPIO_PULL_UP) 
#define XBEETX_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define XBEETX_GROUP                PORT(XBEETX_PORT)
#define XBEETX_BUS_ID               GROUPID(XBEETX_PORT,XBEETX_GRP)
#define XBEETX_PIN                  PIN(XBEETX_PIN_NUMBER)


#define XBEERX                      XBEERX // needed for macros to work
#define XBEERX_PORT                 A
#define XBEERX_BUS                  AHB1
#define XBEERX_GRP                  1
#define XBEERX_PIN_NUMBER           3

#define XBEERX_MODE                 LL_GPIO_MODE_OUTPUT
#define XBEERX_AF                   LL_GPIO_AF_7
#define XBEERX_DEFAULT              BIT_LOW
#define XBEERX_ASSERTED             ACTIVE_HIGH
#define XBEERX_DEASSERTED           ACTIVE_LOW
#define XBEERX_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define XBEERX_PUPD                 (LL_GPIO_PULL_UP) 
#define XBEERX_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define XBEERX_GROUP                PORT(XBEERX_PORT)
#define XBEERX_BUS_ID               GROUPID(XBEERX_PORT,XBEERX_GRP)
#define XBEERX_PIN                  PIN(XBEERX_PIN_NUMBER)



#define RS485DATA_TX                           RS485DATA_TX
#define RS485DATA_TX_PORT                      A
#define RS485DATA_TX_BUS                       AHB1
#define RS485DATA_TX_GRP                       1
#define RS485DATA_TX_PIN_NUMBER                9

#define RS485DATA_TX_MODE                      LL_GPIO_MODE_OUTPUT
#define RS485DATA_TX_AF                        LL_GPIO_AF_7
#define RS485DATA_TX_DEFAULT                   BIT_LOW
#define RS485DATA_TX_ASSERTED                  ACTIVE_HIGH
#define RS485DATA_TX_DEASSERTED                ACTIVE_LOW
#define RS485DATA_TX_GPIO_OTYPE                (LL_GPIO_OUTPUT_PUSHPULL)
#define RS485DATA_TX_PUPD                      (LL_GPIO_PULL_NO) 
#define RS485DATA_TX_SPEED                     (LL_GPIO_SPEED_FREQ_VERY_HIGH)
                                     
#define RS485DATA_TX_GROUP                     PORT(RS485DATA_TX_PORT)
#define RS485DATA_TX_BUS_ID                    GROUPID(RS485DATA_TX_PORT,RS485DATA_TX_GRP)
#define RS485DATA_TX_PIN                       PIN(RS485DATA_TX_PIN_NUMBER)



// 
#define RS485DATA_RX                           RS485DATA_RX
#define RS485DATA_RX_PORT                      A
#define RS485DATA_RX_BUS                       AHB1
#define RS485DATA_RX_GRP                       1
#define RS485DATA_RX_PIN_NUMBER                10

#define RS485DATA_RX_MODE                      LL_GPIO_MODE_OUTPUT
#define RS485DATA_RX_AF                        LL_GPIO_AF_7
#define RS485DATA_RX_DEFAULT                   BIT_LOW
#define RS485DATA_RX_ASSERTED                  ACTIVE_HIGH
#define RS485DATA_RX_DEASSERTED                ACTIVE_LOW
#define RS485DATA_RX_GPIO_OTYPE                (LL_GPIO_OUTPUT_PUSHPULL)
#define RS485DATA_RX_PUPD                      (LL_GPIO_PULL_UP) 
#define RS485DATA_RX_SPEED                     (LL_GPIO_SPEED_FREQ_VERY_HIGH)
                                     
#define RS485DATA_RX_GROUP                     PORT(RS485DATA_RX_PORT)
#define RS485DATA_RX_BUS_ID                    GROUPID(RS485DATA_RX_PORT,RS485DATA_TX_GRP)
#define RS485DATA_RX_PIN                       PIN(RS485DATA_RX_PIN_NUMBER)


#define DEBUG_SERIAL_TX                           DEBUG_SERIAL_TX
#define DEBUG_SERIAL_TX_PORT                      A
#define DEBUG_SERIAL_TX_BUS                       AHB1
#define DEBUG_SERIAL_TX_GRP                       1
#define DEBUG_SERIAL_TX_PIN_NUMBER                0

#define DEBUG_SERIAL_TX_MODE                      LL_GPIO_MODE_OUTPUT
#define DEBUG_SERIAL_TX_AF                        LL_GPIO_AF_8
#define DEBUG_SERIAL_TX_DEFAULT                   BIT_LOW
#define DEBUG_SERIAL_TX_ASSERTED                  ACTIVE_HIGH
#define DEBUG_SERIAL_TX_DEASSERTED                ACTIVE_LOW
#define DEBUG_SERIAL_TX_GPIO_OTYPE                (LL_GPIO_OUTPUT_PUSHPULL)
#define DEBUG_SERIAL_TX_PUPD                      (LL_GPIO_PULL_NO) 
#define DEBUG_SERIAL_TX_SPEED                     (LL_GPIO_SPEED_FREQ_VERY_HIGH)
                                     
#define DEBUG_SERIAL_TX_GROUP                     PORT(DEBUG_SERIAL_TX_PORT)
#define DEBUG_SERIAL_TX_BUS_ID                    GROUPID(DEBUG_SERIAL_TX_PORT,DEBUG_SERIAL_TX_GRP)
#define DEBUG_SERIAL_TX_PIN                       PIN(DEBUG_SERIAL_TX_PIN_NUMBER)



// 
#define DEBUG_SERIAL_RX                           DEBUG_SERIAL_RX
#define DEBUG_SERIAL_RX_PORT                      A
#define DEBUG_SERIAL_RX_BUS                       AHB1
#define DEBUG_SERIAL_RX_GRP                       1
#define DEBUG_SERIAL_RX_PIN_NUMBER                1

#define DEBUG_SERIAL_RX_MODE                      LL_GPIO_MODE_OUTPUT
#define DEBUG_SERIAL_RX_AF                        LL_GPIO_AF_8
#define DEBUG_SERIAL_RX_DEFAULT                   BIT_LOW
#define DEBUG_SERIAL_RX_ASSERTED                  ACTIVE_HIGH
#define DEBUG_SERIAL_RX_DEASSERTED                ACTIVE_LOW
#define DEBUG_SERIAL_RX_GPIO_OTYPE                (LL_GPIO_OUTPUT_PUSHPULL)
#define DEBUG_SERIAL_RX_PUPD                      (LL_GPIO_PULL_NO) 
#define DEBUG_SERIAL_RX_SPEED                     (LL_GPIO_SPEED_FREQ_VERY_HIGH)
                                     
#define DEBUG_SERIAL_RX_GROUP                     PORT(DEBUG_SERIAL_RX_PORT)
#define DEBUG_SERIAL_RX_BUS_ID                    GROUPID(DEBUG_SERIAL_RX_PORT,DEBUG_SERIAL_TX_GRP)
#define DEBUG_SERIAL_RX_PIN                       PIN(DEBUG_SERIAL_RX_PIN_NUMBER)


// PA10
// PA11
// PA12
// PA13 
// PA14 
// PA15 
// PB0
// PB1
// PB2
// PB3 Bank A 
#define BANK_A                      BANK_A // needed for macros to work
#define BANK_A_PORT                 A
#define BANK_A_BUS                  AHB1
#define BANK_A_GRP                  1
#define BANK_A_PIN_NUMBER           4

#define BANK_A_MODE                 LL_GPIO_MODE_OUTPUT
#define BANK_A_AF                   LL_GPIO_AF_NA
#define BANK_A_DEFAULT              BIT_LOW
#define BANK_A_ASSERTED             ACTIVE_HIGH
#define BANK_A_DEASSERTED           ACTIVE_LOW
#define BANK_A_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define BANK_A_PUPD                 (LL_GPIO_PULL_NO) 
#define BANK_A_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define BANK_A_GROUP                PORT(BANK_A_PORT)
#define BANK_A_BUS_ID               GROUPID(BANK_A_PORT,BANK_A_GRP)
#define BANK_A_PIN                  PIN(BANK_A_PIN_NUMBER)
// PB4 Bank B 
#define BANK_B                      BANK_B // needed for macros to work
#define BANK_B_PORT                 A
#define BANK_B_BUS                  AHB1
#define BANK_B_GRP                  1
#define BANK_B_PIN_NUMBER           5

#define BANK_B_MODE                 LL_GPIO_MODE_OUTPUT
#define BANK_B_AF                   LL_GPIO_AF_NA
#define BANK_B_DEFAULT              BIT_LOW
#define BANK_B_ASSERTED             ACTIVE_HIGH
#define BANK_B_DEASSERTED           ACTIVE_LOW
#define BANK_B_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define BANK_B_PUPD                 (LL_GPIO_PULL_NO) 
#define BANK_B_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define BANK_B_GROUP                PORT(BANK_B_PORT)
#define BANK_B_BUS_ID               GROUPID(BANK_B_PORT,BANK_B_GRP)
#define BANK_B_PIN                  PIN(BANK_B_PIN_NUMBER)
// PB5 Bank C 
#define BANK_C                      BANK_C // needed for macros to work
#define BANK_C_PORT                 A
#define BANK_C_BUS                  AHB1
#define BANK_C_GRP                  1
#define BANK_C_PIN_NUMBER           6

#define BANK_C_MODE                 LL_GPIO_MODE_OUTPUT
#define BANK_C_AF                   LL_GPIO_AF_NA
#define BANK_C_DEFAULT              BIT_LOW
#define BANK_C_ASSERTED             ACTIVE_HIGH
#define BANK_C_DEASSERTED           ACTIVE_LOW
#define BANK_C_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define BANK_C_PUPD                 (LL_GPIO_PULL_NO) 
#define BANK_C_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define BANK_C_GROUP                PORT(BANK_C_PORT)
#define BANK_C_BUS_ID               GROUPID(BANK_C_PORT,BANK_C_GRP)
#define BANK_C_PIN                  PIN(BANK_C_PIN_NUMBER)
// PB6 Bank D 
#define BANK_D                      BANK_D // needed for macros to work
#define BANK_D_PORT                 A
#define BANK_D_BUS                  AHB1
#define BANK_D_GRP                  1
#define BANK_D_PIN_NUMBER           7

#define BANK_D_MODE                 LL_GPIO_MODE_OUTPUT
#define BANK_D_AF                   LL_GPIO_AF_NA
#define BANK_D_DEFAULT              BIT_LOW
#define BANK_D_ASSERTED             ACTIVE_HIGH
#define BANK_D_DEASSERTED           ACTIVE_LOW
#define BANK_D_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define BANK_D_PUPD                 (LL_GPIO_PULL_NO) 
#define BANK_D_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define BANK_D_GROUP                PORT(BANK_D_PORT)
#define BANK_D_BUS_ID               GROUPID(BANK_D_PORT,BANK_D_GRP)
#define BANK_D_PIN                  PIN(BANK_D_PIN_NUMBER)
// PB7
// PB8
// PB9
// PC0 Latch
#define LATCHT                      LATCHT // needed for macros to work
#define LATCHT_PORT                 B
#define LATCHT_BUS                  AHB2
#define LATCHT_GRP                  1
#define LATCHT_PIN_NUMBER           6

#define LATCHT_MODE                 LL_GPIO_MODE_OUTPUT
#define LATCHT_AF                   LL_GPIO_AF_NA
#define LATCHT_DEFAULT              BIT_HIGH
#define LATCHT_ASSERTED             ACTIVE_HIGH
#define LATCHT_DEASSERTED           ACTIVE_LOW
#define LATCHT_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define LATCHT_PUPD                 (LL_GPIO_PULL_NO) 
#define LATCHT_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define LATCHT_GROUP                PORT(LATCHT_PORT)
#define LATCHT_BUS_ID               GROUPID(LATCHT_PORT,LATCHT_GRP)
#define LATCHT_PIN                  PIN(LATCHT_PIN_NUMBER)
// PC1 OE
#define OET                      OET // needed for macros to work
#define OET_PORT                 B
#define OET_BUS                  AHB2
#define OET_GRP                  1
#define OET_PIN_NUMBER           4

#define OET_MODE                 LL_GPIO_MODE_OUTPUT
#define OET_AF                   LL_GPIO_AF_NA
#define OET_DEFAULT              BIT_HIGH
#define OET_ASSERTED             ACTIVE_HIGH
#define OET_DEASSERTED           ACTIVE_LOW
#define OET_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define OET_PUPD                 (LL_GPIO_PULL_NO) 
#define OET_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define OET_GROUP                PORT(OET_PORT)
#define OET_BUS_ID               GROUPID(OET_PORT,OET_GRP)
#define OET_PIN                  PIN(OET_PIN_NUMBER)
// PC2 
// PC3
// PC4
// PC5
// PC6
// PC7  Clock
#define CLOCKT                      CLOCKT // needed for macros to work
#define CLOCKT_PORT                 B
#define CLOCKT_BUS                  AHB2
#define CLOCKT_GRP                  1
#define CLOCKT_PIN_NUMBER           0

#define CLOCKT_MODE                 LL_GPIO_MODE_OUTPUT
#define CLOCKT_AF                   LL_GPIO_AF_NA
#define CLOCKT_DEFAULT              BIT_LOW
#define CLOCKT_ASSERTED             ACTIVE_HIGH
#define CLOCKT_DEASSERTED           ACTIVE_LOW
#define CLOCKT_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define CLOCKT_PUPD                 (LL_GPIO_PULL_NO) 
#define CLOCKT_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define CLOCKT_GROUP                PORT(CLOCKT_PORT)
#define CLOCKT_BUS_ID               GROUPID(CLOCKT_PORT,CLOCKT_GRP)
#define CLOCKT_PIN                  PIN(CLOCKT_PIN_NUMBER)


// PC0 Latch
#define LATCHB                      LATCHB // needed for macros to work
#define LATCHB_PORT                 B
#define LATCHB_BUS                  AHB2
#define LATCHB_GRP                  1
#define LATCHB_PIN_NUMBER           7

#define LATCHB_MODE                 LL_GPIO_MODE_OUTPUT
#define LATCHB_AF                   LL_GPIO_AF_NA
#define LATCHB_DEFAULT              BIT_HIGH
#define LATCHB_ASSERTED             ACTIVE_HIGH
#define LATCHB_DEASSERTED           ACTIVE_LOW
#define LATCHB_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define LATCHB_PUPD                 (LL_GPIO_PULL_NO) 
#define LATCHB_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define LATCHB_GROUP                PORT(LATCHB_PORT)
#define LATCHB_BUS_ID               GROUPID(LATCHB_PORT,LATCHB_GRP)
#define LATCHB_PIN                  PIN(LATCHB_PIN_NUMBER)
// PC1 OE
#define OEB                      OEB // needed for macros to work
#define OEB_PORT                 B
#define OEB_BUS                  AHB2
#define OEB_GRP                  1
#define OEB_PIN_NUMBER           5

#define OEB_MODE                 LL_GPIO_MODE_OUTPUT
#define OEB_AF                   LL_GPIO_AF_NA
#define OEB_DEFAULT              BIT_HIGH
#define OEB_ASSERTED             ACTIVE_HIGH
#define OEB_DEASSERTED           ACTIVE_LOW
#define OEB_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define OEB_PUPD                 (LL_GPIO_PULL_NO) 
#define OEB_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define OEB_GROUP                PORT(OEB_PORT)
#define OEB_BUS_ID               GROUPID(OEB_PORT,OEB_GRP)
#define OEB_PIN                  PIN(OEB_PIN_NUMBER)
// PC2 
// PC3
// PC4
// PC5
// PC6
// PC7  Clock
#define CLOCKB                      CLOCKB // needed for macros to work
#define CLOCKB_PORT                 B
#define CLOCKB_BUS                  AHB2
#define CLOCKB_GRP                  1
#define CLOCKB_PIN_NUMBER           1

#define CLOCKB_MODE                 LL_GPIO_MODE_OUTPUT
#define CLOCKB_AF                   LL_GPIO_AF_NA
#define CLOCKB_DEFAULT              BIT_LOW
#define CLOCKB_ASSERTED             ACTIVE_HIGH
#define CLOCKB_DEASSERTED           ACTIVE_LOW
#define CLOCKB_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define CLOCKB_PUPD                 (LL_GPIO_PULL_NO) 
#define CLOCKB_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define CLOCKB_GROUP                PORT(CLOCKB_PORT)
#define CLOCKB_BUS_ID               GROUPID(CLOCKB_PORT,CLOCKB_GRP)
#define CLOCKB_PIN                  PIN(CLOCKB_PIN_NUMBER)


#define BUTTON                      BUTTON // needed for macros to work
#define BUTTON_PORT                 C
// LL_AHB1_GRP1_EnableClock
#define BUTTON_BUS                  AHB1
#define BUTTON_GRP                  1
#define BUTTON_PIN_NUMBER           13

#define BUTTON_MODE                 LL_GPIO_MODE_OUTPUT
#define BUTTON_AF                   LL_GPIO_AF_NA
#define BUTTON_DEFAULT              BIT_HIGH
#define BUTTON_ASSERTED             ACTIVE_HIGH
#define BUTTON_DEASSERTED           ACTIVE_LOW
#define BUTTON_GPIO_OTYPE           (LL_GPIO_OUTPUT_PUSHPULL)
#define BUTTON_PUPD                 (LL_GPIO_PULL_NO) 
#define BUTTON_SPEED                (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define BUTTON_GROUP                PORT(BUTTON_PORT)
#define BUTTON_BUS_ID               GROUPID(BUTTON_PORT,BUTTON_GRP)
#define BUTTON_PIN                  PIN(BUTTON_PIN_NUMBER)


#endif
