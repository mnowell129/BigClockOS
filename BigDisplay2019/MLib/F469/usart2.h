#ifndef __USART2_H
#define __USART2_H


#include "stm32f4xx_ll_usart.h"
#ifndef UART_GETCH_ERROR
#define UART_GETCH_ERROR -1
#endif

// The buffer size must be a power of 2 so that the buffer wrap-around can be
// handled with the SIZE_MASK below
#define UART2_RX_SIZE   512   ///< Size of receive buffer

#define UART2_RX_SIZE_MASK  (UART2_RX_SIZE-1) ///< Wrap-around mask for receive buffer

#if (UART2_RX_SIZE & UART2_RX_SIZE_MASK)
   #error    Compiler error, UART2_RX_SIZE needs to be a power of 2
#endif

uint32_t uart2Init(uint32_t baudRate);
uint8_t uart2Getch(void);
int32_t uart2GetchTO(uint32_t timeout);
void uart2Gets(uint8_t *buffer);

#endif


