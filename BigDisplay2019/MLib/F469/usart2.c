#include <intrinsics.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#include "rtos.h"
#include "type.h"
#include "ascii.h"

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"

#include "gpioapi.h"
#include "ioconfiguration.h"
#include "usart2.h"

volatile uint32_t uart2RxWriteIndex;
volatile uint32_t uart2RxReadIndex;
volatile int32_t  uart2RxCount;
RTOS_SEMA_PTR      uart2RxSema;
Byte uart2RxBuffer[UART2_RX_SIZE];

#define UART2_RX_ERRORS (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)

void USART2_IRQHandler(void)
{
   ISRCONTEXT;
   uint32_t statusRegister;
   volatile Byte receivedByte;

   while(1)
   {
      statusRegister = USART2->SR;

      if(statusRegister & UART2_RX_ERRORS)
      {
         receivedByte = USART2->DR;
         continue;
      }
      if((statusRegister & (UART2_RX_ERRORS | USART_SR_RXNE)) == 0)
      {
         break;
      }
      if(statusRegister & USART_SR_RXNE)
      {
         receivedByte = USART2->DR;
         if(uart2RxCount >= UART2_RX_SIZE - 1)
         {
            // don't put receivedByte in the buffer
         }
         else
         {
            uart2RxBuffer[uart2RxWriteIndex] = receivedByte;
            uart2RxWriteIndex++;
            uart2RxWriteIndex &= UART2_RX_SIZE_MASK;
            uart2RxCount++;
            if(uart2RxCount == 0)
            {
               // then it was -1, someone was waiting
               ISR_SEMA_PUT(uart2RxSema);
            }
         }
      }
   }
   ISR_EXIT();
}


static LL_USART_InitTypeDef      usartInit;

uint32_t uart2Init(uint32_t baudRate)
                 
{

   uart2RxSema = xSemaphoreCreateCounting(1024,0); 
   uart2RxWriteIndex = 0;
   uart2RxReadIndex = 0;
   uart2RxCount = 0;

   INITGPIOAF(XBEERX);
   LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
   NVIC_SetPriority(USART2_IRQn,5);

   usartInit.BaudRate = baudRate;
   usartInit.DataWidth = 8;
   usartInit.TransferDirection = LL_USART_DIRECTION_TX_RX;

   if(LL_USART_DeInit(USART2) == ERROR)
   {
      while(1);
   }

   if(LL_USART_Init(USART2,&usartInit) == ERROR)
   {
      while(1);
   }
   USART2->CR1 |= USART_CR1_UE;

   USART2->CR1 &= ~(USART_CR1_TCIE | USART_CR1_TXEIE);

   USART2->CR1 |= USART_CR1_RXNEIE;
   USART2->SR = 0; 

   NVIC_EnableIRQ(USART2_IRQn);
   return(1);
} 

int32_t uart2GetchTO(uint32_t timeout)
{
   int32_t value;
   volatile CONTEXT;

   ENTER_CRITICAL();  
   uart2RxCount--; 
   if(uart2RxCount < 0)
   {  
      EXIT_CRITICAL();
      SEMA_GET(uart2RxSema,timeout,error);
      if(FAILTEST(error)) 
      {  
         ENTER_CRITICAL();  
         if(uart2RxCount < 0)
         {
            uart2RxCount = 0;
            EXIT_CRITICAL();
            return(UART_GETCH_ERROR);
         } //if
         else
         {
            uart2RxCount++;
            EXIT_CRITICAL();
            SEMA_GET(uart2RxSema,WAIT_FOREVER,error);
            return(UART_GETCH_ERROR);
         } 
      } 
   }  
   else  
   {
      EXIT_CRITICAL();  
   }  
   value = uart2RxBuffer[uart2RxReadIndex++];
   uart2RxReadIndex &= UART2_RX_SIZE_MASK;
   return(value);
} 


Byte uart2Getch(void)
{
   int32_t input = UART_GETCH_ERROR;
   Byte  goodByte;

   while(input == UART_GETCH_ERROR)
   {
      input = uart2GetchTO(WAIT_FOREVER);
   }
   goodByte = (Byte)(0xff & input);
   return(goodByte);
} 


void uart2Gets(Byte *buffer)
{
   int32_t value;
   *buffer = 0;
   int count = 0;
   while(1)
   {
      value = uart2Getch();
      if(value == UART_GETCH_ERROR)
      {
         continue;
      }

      switch(value)
      {
         case ASCII_BS:
            if(count > 0)
            {
               count--;
               buffer--;
               *buffer = 0;
            }
            break;
         case '\r':
            return;
         case '\n':
            break;
         case ASCII_NUL:
            *buffer++ = (Byte)(0xff & value);
            count++;
            *buffer = 0;
            return;

         default:
            if(isprint(value))
            {
               *buffer++ = (Byte)(0xff & value);
               count++;
               *buffer = 0;
            } 
            break;
      } 
   } 
} 

