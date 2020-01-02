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
#include "usart4.h"

volatile uint32_t uart4RxWriteIndex;
volatile uint32_t uart4RxReadIndex;
volatile int32_t  uart4RxCount;
RTOS_SEMA_PTR      uart4RxSema;
Byte uart4RxBuffer[UART4_RX_SIZE];

#define UART4_RX_ERRORS (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)

void UART4_IRQHandler(void)
{
   ISRCONTEXT;
   uint32_t statusRegister;
   volatile Byte receivedByte;

   while(1)
   {
      statusRegister = UART4->SR;

      if(statusRegister & UART4_RX_ERRORS)
      {
         receivedByte = UART4->DR;
         continue;
      }
      if((statusRegister & (UART4_RX_ERRORS | USART_SR_RXNE)) == 0)
      {
         break;
      }
      if(statusRegister & USART_SR_RXNE)
      {
         receivedByte = UART4->DR;
         if(uart4RxCount >= UART4_RX_SIZE - 1)
         {
            // don't put receivedByte in the buffer
         }
         else
         {
            uart4RxBuffer[uart4RxWriteIndex] = receivedByte;
            uart4RxWriteIndex++;
            uart4RxWriteIndex &= UART4_RX_SIZE_MASK;
            uart4RxCount++;
            if(uart4RxCount == 0)
            {
               // then it was -1, someone was waiting
               ISR_SEMA_PUT(uart4RxSema);
            }
         }
      }
   }
   ISR_EXIT();
}


static LL_USART_InitTypeDef      usartInit;

uint32_t uart4Init(uint32_t baudRate)
                 
{

   uart4RxSema = xSemaphoreCreateCounting(1024,0); 
   uart4RxWriteIndex = 0;
   uart4RxReadIndex = 0;
   uart4RxCount = 0;

   INITGPIOAF(DEBUG_SERIAL_RX);

   LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
   NVIC_SetPriority(UART4_IRQn,5);

   usartInit.BaudRate = baudRate;
   usartInit.DataWidth = 8;
   usartInit.TransferDirection = LL_USART_DIRECTION_TX_RX;

   if(LL_USART_DeInit(UART4) == ERROR)
   {
      while(1);
   }

   if(LL_USART_Init(UART4,&usartInit) == ERROR)
   {
      while(1);
   }
   UART4->CR1 |= USART_CR1_UE;

   UART4->CR1 &= ~(USART_CR1_TCIE | USART_CR1_TXEIE);

   UART4->CR1 |= USART_CR1_RXNEIE;
   UART4->SR = 0; 

   NVIC_EnableIRQ(UART4_IRQn);
   return(1);
} 

int32_t uart4GetchTO(uint32_t timeout)
{
   int32_t value;
   volatile CONTEXT;

   ENTER_CRITICAL();  
   uart4RxCount--; 
   if(uart4RxCount < 0)
   {  
      EXIT_CRITICAL();
      SEMA_GET(uart4RxSema,timeout,error);
      if(FAILTEST(error)) 
      {  
         ENTER_CRITICAL();  
         if(uart4RxCount < 0)
         {
            uart4RxCount = 0;
            EXIT_CRITICAL();
            return(UART_GETCH_ERROR);
         } //if
         else
         {
            uart4RxCount++;
            EXIT_CRITICAL();
            SEMA_GET(uart4RxSema,WAIT_FOREVER,error);
            return(UART_GETCH_ERROR);
         } 
      } 
   }  
   else  
   {
      EXIT_CRITICAL();  
   }  
   value = uart4RxBuffer[uart4RxReadIndex++];
   uart4RxReadIndex &= UART4_RX_SIZE_MASK;
   return(value);
} 


Byte uart4Getch(void)
{
   int32_t input = UART_GETCH_ERROR;
   Byte  goodByte;

   while(input == UART_GETCH_ERROR)
   {
      input = uart4GetchTO(WAIT_FOREVER);
   }
   goodByte = (Byte)(0xff & input);
   return(goodByte);
} 


void uart4Gets(Byte *buffer)
{
   int32_t value;
   *buffer = 0;
   int count = 0;
   while(1)
   {
      value = uart4Getch();
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

