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
#include "usart1.h"

volatile uint32_t uart1RxWriteIndex;
volatile uint32_t uart1RxReadIndex;
volatile int32_t  uart1RxCount;
RTOS_SEMA_PTR      uart1RxSema;
Byte uart1RxBuffer[UART1_RX_SIZE];

#define UART1_RX_ERRORS (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)

void USART1_IRQHandler(void)
{
   ISRCONTEXT;
   uint32_t statusRegister;
   volatile Byte receivedByte;

   while(1)
   {
      statusRegister = USART1->SR;

      if(statusRegister & UART1_RX_ERRORS)
      {
         receivedByte = USART1->DR;
         continue;
      }
      if((statusRegister & (UART1_RX_ERRORS | USART_SR_RXNE)) == 0)
      {
         break;
      }
      if(statusRegister & USART_SR_RXNE)
      {
         receivedByte = USART1->DR;
         if(uart1RxCount >= UART1_RX_SIZE - 1)
         {
            // don't put receivedByte in the buffer
         }
         else
         {
            uart1RxBuffer[uart1RxWriteIndex] = receivedByte;
            uart1RxWriteIndex++;
            uart1RxWriteIndex &= UART1_RX_SIZE_MASK;
            uart1RxCount++;
            if(uart1RxCount == 0)
            {
               // then it was -1, someone was waiting
               ISR_SEMA_PUT(uart1RxSema);
            }
         }
      }
   }
   ISR_EXIT();
}


static LL_USART_InitTypeDef      usartInit;

uint32_t uart1Init(uint32_t baudRate)
                 
{

   uart1RxSema = xSemaphoreCreateCounting(1024,0); 
   uart1RxWriteIndex = 0;
   uart1RxReadIndex = 0;
   uart1RxCount = 0;

   INITGPIOAF(RS485DATA_RX);
   LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
   NVIC_SetPriority(USART1_IRQn,5);

   usartInit.BaudRate = baudRate;
   usartInit.DataWidth = 8;
   usartInit.TransferDirection = LL_USART_DIRECTION_TX_RX;

   if(LL_USART_DeInit(USART1) == ERROR)
   {
      while(1);
   }

   if(LL_USART_Init(USART1,&usartInit) == ERROR)
   {
      while(1);
   }
   USART1->CR1 |= USART_CR1_UE;

   USART1->CR1 &= ~(USART_CR1_TCIE | USART_CR1_TXEIE);

   USART1->CR1 |= USART_CR1_RXNEIE;
   USART1->SR = 0; 

   NVIC_EnableIRQ(USART1_IRQn);
   return(1);
} 

int32_t uart1GetchTO(uint32_t timeout)
{
   int32_t value;
   volatile CONTEXT;

   ENTER_CRITICAL();  
   uart1RxCount--; 
   if(uart1RxCount < 0)
   {  
      EXIT_CRITICAL();
      SEMA_GET(uart1RxSema,timeout,error);
      if(FAILTEST(error)) 
      {  
         ENTER_CRITICAL();  
         if(uart1RxCount < 0)
         {
            uart1RxCount = 0;
            EXIT_CRITICAL();
            return(UART_GETCH_ERROR);
         } //if
         else
         {
            uart1RxCount++;
            EXIT_CRITICAL();
            SEMA_GET(uart1RxSema,WAIT_FOREVER,error);
            return(UART_GETCH_ERROR);
         } 
      } 
   }  
   else  
   {
      EXIT_CRITICAL();  
   }  
   value = uart1RxBuffer[uart1RxReadIndex++];
   uart1RxReadIndex &= UART1_RX_SIZE_MASK;
   return(value);
} 


Byte uart1Getch(void)
{
   int32_t input = UART_GETCH_ERROR;
   Byte  goodByte;

   while(input == UART_GETCH_ERROR)
   {
      input = uart1GetchTO(WAIT_FOREVER);
   }
   goodByte = (Byte)(0xff & input);
   return(goodByte);
} 


void uart1Gets(Byte *buffer)
{
   int32_t value;
   *buffer = 0;
   int count = 0;
   while(1)
   {
      value = uart1Getch();
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

