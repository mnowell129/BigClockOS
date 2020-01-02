 #ifndef __USART4_H
#define __USART4_H


#include "stm32f4xx_ll_usart.h"
#ifndef UART_GETCH_ERROR
#define UART_GETCH_ERROR -1
#endif

// The buffer size must be a power of 2 so that the buffer wrap-around can be
// handled with the SIZE_MASK below
#define UART4_RX_SIZE   512   ///< Size of receive buffer

#define UART4_RX_SIZE_MASK  (UART4_RX_SIZE-1) ///< Wrap-around mask for receive buffer

#if (UART4_RX_SIZE & UART4_RX_SIZE_MASK)
   #error    Compiler error, UART4_RX_SIZE needs to be a power of 2
#endif


uint32_t uart4Init(uint32_t baudRate);
uint8_t uart4Getch(void);
int32_t uart4GetchTO(uint32_t timeout);
void uart4Gets(uint8_t *buffer);

#endif


