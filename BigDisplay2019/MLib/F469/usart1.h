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
#ifndef __USART1_H
#define __USART1_H


#include "stm32f4xx_ll_usart.h"

#ifndef UART_GETCH_ERROR
#define UART_GETCH_ERROR -1
#endif
// The buffer size must be a power of 2 so that the buffer wrap-around can be
// handled with the SIZE_MASK below
#define UART1_RX_SIZE   512   ///< Size of receive buffer

#define UART1_RX_SIZE_MASK  (UART1_RX_SIZE-1) ///< Wrap-around mask for receive buffer

#if (UART1_RX_SIZE & UART1_RX_SIZE_MASK)
   #error    Compiler error, UART1_RX_SIZE needs to be a power of 2
#endif

uint32_t uart1Init(uint32_t baudRate);
uint8_t uart1Getch(void);
int32_t uart1GetchTO(uint32_t timeout);
void uart1Gets(uint8_t *buffer);

#endif 


