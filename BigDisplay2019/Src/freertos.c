/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicrOETlectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicrOETlectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicrOETlectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROETLECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROETLECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "usart.h"


#include "gpio.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "type.h"
#include "rtos.h"

#include "stm32F4xx.h"
#include "stm32F446deviceall.h"
#include "gpioapi.h"
#include "IOConfiguration.h"
#include "fonts.h"
#include "usart2.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId scannerTaskHandle;
osThreadId receiverTaskHandle;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void (*TickHook)(void);

void vApplicationTickHook(void)
{
   if(TickHook)
   {
      (*TickHook)();
   }
}
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
   static StaticTask_t xIdleTaskTCB;
   static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

   /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
   state will be stored. */
   *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

   /* Pass out the array that will be used as the Idle task's stack. */
   *ppxIdleTaskStackBuffer = uxIdleTaskStack;

   /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
   Note that, as the array is necessarily of type StackType_t,
   configMINIMAL_STACK_SIZE is specified in words, not bytes. */
   *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}


__STATIC_INLINE void killCLOCKTs(int32_t CLOCKTs)
{
   int32_t currentCLOCKTs;
   int32_t  newCLOCKTs;

   currentCLOCKTs = (int32_t)(DWT->CYCCNT);

   while(CLOCKTs--)
   {
      // updateCounter(CLOCKTs);
      newCLOCKTs = (int32_t)(DWT->CYCCNT);
      if((newCLOCKTs - currentCLOCKTs) > CLOCKTs)
      {
         return;
      }
   }

}




#define NOP()  killCLOCKTs(3)

/*
These are definitions for the STM32F446 using a STM32F446RE Nucleo board as a base. 
PORT A controls the bank bits, Port C the colors. 
This driver will control either a 1x4 32x32 panel, two 1x4 32x32 panels, or one 2x5 32x32 panels. 
Both 1/16 and 1/8 scan panels can be used. 
For 1/8 scan the putPixel routine maps the pixels in the odd way needed. 
*/
#define BANK_PORT           GPIOA
#define BANK_CLEAR_MASK     (BIT4|BIT5|BIT6|BIT7)
#define BANK(x)             BANK_PORT->ODR = (BANK_PORT->ODR & ~BANK_CLEAR_MASK) | ((x) << 4)


#define COLOR_PORT          GPIOC
#define COLOR(x)            COLOR_PORT->ODR = x;


volatile Int32 seconds;
volatile Int32 minutes;
volatile Int32 _milliseconds;
volatile Int32 milliseconds;

Byte pilotOffset = 0;
volatile bool doBlink = false;
volatile bool doReverse = true;

uint32_t phase,secondsIn,round,heat,flight;
uint32_t i;


uint32_t bank = 0;
uint32_t color = 0;
#define PIXEL(a,b,c,d)  ((a<<9)|(b<<6)|(c<<3)|(d))

//void putPixels(uint32_t a,uint32_t b,uint32_t c, uint32_t d)
void putPixels(uint32_t pixel)
{

   // COLOR(PIXEL(a,b,c,d));
   COLOR(pixel);
   NOP();
   LOWGPIO(CLOCKT);
   LOWGPIO(CLOCKB);
   NOP();
   HIGHGPIO(CLOCKT);
   HIGHGPIO(CLOCKB);
   NOP();
   LOWGPIO(CLOCKT);
   LOWGPIO(CLOCKB);
}


// 320 pixels across, in a funny order
// 7 rows, 4 subrows per row i.e. 4 pixels per output word, 12 bits
// 16 bits is enough
uint16_t displayBuffer[2][8][320];
#define NUMPIXELS   320

void clearBuffer(uint32_t buffer)
{
   memset(displayBuffer[buffer],0,8 * 320 * 2);
}

const uint32_t  x1Delta[] =
{
   0,48,48 + 16,112,112 + 16,176,176 + 16,240,240 + 16,304,304
};


const uint32_t  x2Delta[] =
{
   16,80,144,208,272
};



// divide x by 16, get the x1 delta
// then add original x to offset

#define NEWX1(x)   (x & 0x0F)+(x1Delta[x>>4])
#define NEWX2(x)   (x & 0x1F)+(x2Delta[x>>5])



void putPixel(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx, localy;

   if(x < 0 || y < 0)
   {
      return;
   }
   if(x > 63 || y > 159)
   {
      return;
   }

   y = 63 - y;
   x = 159 - x;

   shift = y >> 4;
   if(y & 0x08)
   {
      localx = NEWX2(x);
   }
   else
   {
      localx = NEWX1(x);
   }
   displayBuffer[buffer][y & 7][localx] |= color << (shift * 3);
}

volatile uint32_t bufferNumber;
void pickBuffer(uint32_t buffer)
{
   bufferNumber = buffer;
}
void outputRow(void)
{
   int i;
   uint16_t   *pixelRow;

   uint32_t pixel;

   pixelRow = &(displayBuffer[showFrame][bank][0]);
   BANK(0);
   // BANK(1);
   NOP();

   // HIGHGPIO(LATCHT);
   // NOP();
   LOWGPIO(LATCHT);
   LOWGPIO(LATCHB);
   NOP();
   HIGHGPIO(OET);
   HIGHGPIO(OEB);
   // LOWGPIO(OET);
   NOP();
   LOWGPIO(CLOCKT);
   LOWGPIO(CLOCKB);
   NOP();

   for(i = 0;i < NUMPIXELS;i++)
   {
      //putPixels(i,i,i,i);
      putPixels(*pixelRow++); // color,color,color,color);
   }

   NOP();
   //HIGHGPIO(OET);
   NOP();
   HIGHGPIO(LATCHT);
   HIGHGPIO(LATCHB);
   NOP();
   NOP();
   NOP();
   LOWGPIO(LATCHT);
   LOWGPIO(LATCHB);
   NOP();
   LOWGPIO(OET);
   LOWGPIO(OEB);
   //  bank++;
   BANK(bank++);
   bank &= 0x07;
//   color++;
//   color &= 7;
}

void initGPIO(void)
{
   INITGPIOOUT(LATCHT);
   INITGPIOOUT(OET);
   INITGPIOOUT(LATCHB);
   INITGPIOOUT(OEB);
   INITGPIOOUT(BANK_A);
   INITGPIOOUT(BANK_B);
   INITGPIOOUT(BANK_C);
   INITGPIOOUT(BANK_D);
   INITGPIOOUT(CLOCKB);
   INITGPIOOUT(CLOCKT);
}


void scannerTask(void const *argument)
{
   initGPIO();
   LOWGPIO(OET);
   LOWGPIO(OEB);
   /* Infinite loop */
   for(;;)
   {
      RTOS_MSEC_DELAY(1);
      outputRow(); // 0,5,320);
   }
}

char inputBuffer[80];

bool extractData(char *buffer,uint32_t *phase,uint32_t *seconds, uint32_t *round,
                 uint32_t *heat,uint32_t *flight)
{
   char *field;
   buffer++; // skip the letter
   if(*buffer != ':')
   {
      return(false);
   }
   buffer++;
   // now pointing to the number
   field = strtok(buffer,",");
   if(!field)
   {
      return(false);
   }
   *phase = atoi(field);
   field = strtok(NULL,",");
   if(!field)
   {
      return(false);
   }
   *seconds = atoi(field);
   field = strtok(NULL,",");
   if(!field)
   {
      return(false);
   }
   *round = atoi(field);
   field = strtok(NULL,",");
   if(!field)
   {
      return(false);
   }
   *heat = atoi(field);
   field = strtok(NULL,",");
   if(!field)
   {
      return(false);
   }
   *flight = atoi(field);
   return(true);



}

void extractBlink(char *buffer,bool volatile *doBlinkPtr)
{
   int  blinkValue;
   char *field;
   buffer++; // skip the letter
   if(*buffer != ':')
   {
      return;
   }
   buffer++;
   // now pointing to the number
   field = strtok(buffer,",");
   if(!field)
   {
      return;
   }
   blinkValue = atoi(field);
   if(blinkValue)
   {
      *doBlinkPtr = true;
   }
   else
   {
      *doBlinkPtr = false;
   }
}

void extractReverse(char *buffer,bool volatile *doReversePtr)
{
   int  reverseValue;
   char *field;
   buffer++; // skip the letter
   if(*buffer != ':')
   {
      return;
   }
   buffer++;
   // now pointing to the number
   field = strtok(buffer,",");
   if(!field)
   {
      return;
   }
   reverseValue = atoi(field);
   if(reverseValue)
   {
      *doReversePtr = true;
   }
   else
   {
      *doReversePtr = false;
   }
}


void showData(uint32_t minutes, uint32_t seconds, uint32_t round, uint32_t heat, int32_t flight, uint32_t symbol, uint32_t color)
{
   clearImage();

   putColon(color);
   putValue64(seconds % 10,ONES_PLACE,color);
   putValue64(seconds / 10,TENS_PLACE,color);
   putValue64(minutes % 10,MINUTES_PLACE,color);
   if(minutes / 10)
   {
      putValue(minutes / 10,TENS_OF_MINUTES_PLACE,color);
   }
   putStage(color,symbol);
   if(flight > 0)
   {
      putHalfBigValue(flight,1,PIXELS_HIGH - 14,WHITE);
   }
#if 0

   if(round > 0)
   {
      if(round > 9)
      {
         // putHalfBigValue(topImage + drawFrame,1,1,1,WHITE);
         // putHalfBigValue(topImage + drawFrame,round - 10,1,9,WHITE);
         putHalfBigValue(topImage + drawFrame,round / 10 ,1,1,WHITE);
         putHalfBigValue(topImage + drawFrame,round % 10,1,9,WHITE);
      }
      else
      {
         putHalfBigValue(topImage + drawFrame,round,1,1,WHITE);
      }
      putHalfBigLetter(topImage + drawFrame,heat - 'A',16,1,WHITE);
      if(flight > 0)
      {
         putHalfBigValue(topImage + drawFrame,flight,16,11,WHITE);
      }
   }
#endif
}

#define BIGP 0
#define BIGW 1
#define BIGL 2
#define MINUS 3
#define WINDOW 4
#define PLUS 5
#define STOP 6
#define HOURGLASS 7
#define BREAK  8
// const DigitType(symbols[9]) =
// {
//    &bigP,&bigW,&bigL,&minusIcon,&windowIcon,&plusIcon,&stopIcon,&hourGlassIcon,&breakIcon
// };



void getInput(void)
{
   while(1)
   {
      uart2Gets((uint8_t *)inputBuffer);
      switch(inputBuffer[0])
      {

         case 'R': // set the reverse
            extractReverse(inputBuffer,&doReverse);
            break;
         case 'B': // set the blink
            extractBlink(inputBuffer,&doBlink);
            break;
         case 'P':
            if(extractData(inputBuffer,&phase,&secondsIn,&round,&heat,&flight))
            {
               minutes = secondsIn / 60;
               seconds = secondsIn % 60;
               switch(phase)
               {

                  case 0:
                     // prep time - flying allowed
                     showData(minutes,seconds,round,heat + 'A',flight,FLY_TOP,YELLOW);
                     break;
                  case 1:
                     // window flying allowed
                     showData(minutes,seconds,round,heat + 'A',flight,FLY_MIDDLE,GREEN);
                     break;
                  case 2:
                     // landing window
                     if(flight > 0)
                     {
                        showData(minutes,seconds,round,heat + 'A',flight,LAND_MIDDLE,RED);
                     }
                     else
                     {
                        showData(minutes,seconds,round,heat + 'A',flight,LAND_BOTTOM,RED);
                     }
                     break;
                  case 3: // no fly in all up
                     showData(minutes,seconds,round,heat + 'A',flight,STOP_MIDDLE,RED);
                     break;
                  case 4: // break between rounds
                     showData(minutes,seconds,round,heat + 'A',0,FLY_TOP,CYAN);
                     break;
               }

            }
            break;
         case 'S':
            changeImagePlane();
            break;
      }
   }
}

void receiverTask(void const *argument)
{
   uint8_t value;
   /* Infinite loop */
   uart2Init(USART2,9600,NO_PARITY);
   minutes = 0;
   seconds = 0;
   round = 1;
   heat = 'A';
   flight = 1;

   getInput();
   for(;;)
   {
      value = uart2Getch();
      switch(value)
      {
         case '0':
            // prep time - flying allowed
            showData(minutes,seconds,round,heat + 'A',flight,FLY_TOP,YELLOW);
            changeImagePlane();

            break;
         case '1':
            // window flying allowed
            showData(minutes,seconds,round,heat + 'A',flight,FLY_MIDDLE,GREEN);
            changeImagePlane();
            break;
         case '2':
            // landing
            if(flight > 0)
            {
               showData(minutes,seconds,round,heat + 'A',flight,LAND_MIDDLE,RED);
            }
            else
            {
               showData(minutes,seconds,round,heat + 'A',flight,LAND_BOTTOM,RED);
            }
            changeImagePlane();
            break;
         case '3':
            // no fly in all up
            showData(minutes,seconds,round,heat + 'A',flight,STOP_MIDDLE,RED);
            changeImagePlane();
            break;
         case '4':
            // break between rounds
            showData(minutes,seconds,round,heat + 'A',0,FLY_TOP,CYAN);
            changeImagePlane();
            break;
         case 'f':
            flight++;
            if(flight > 3)
            {
               flight = 0;
            }
            break;
         case '+':
            seconds++;
            if(seconds > 59)
            {
               minutes++;
               seconds = 0;
            }
            showData(minutes,seconds,round,heat + 'A',flight,FLY_MIDDLE,GREEN);
            changeImagePlane();
            break;
         case 'c':
            minutes = seconds = 0;
            break;
      }
   }
}




void StartDefaultTask(void const *argument)
{
   int i;
   int j;
   uint32_t color;
   uint32_t buffer;
   // start the scanner task
   osThreadDef(scannerTask,scannerTask,15,0,128);
   scannerTaskHandle = osThreadCreate(osThread(scannerTask),NULL);
   osThreadDef(receiverTask,receiverTask,10,0,128);
   receiverTaskHandle = osThreadCreate(osThread(receiverTask),NULL);

   /* USER CODE BEGIN StartDefaultTask */
   /* Infinite loop */
   color = 0;
   buffer = 1;
   for(;;)
   {
      RTOS_MSEC_DELAY(1000);
      continue;

#if 0
      tick();
      tick();
      tick();

      while(1) // for(i=0;i<30;i++)
      {
         paint64();
         changeImagePlane();
         RTOS_MSEC_DELAY(1000);
         tick();
      }
#endif
#if 0
      paintRound64(buffer,'A' + color,color);
      changeImagePlane();
      color++;
      color &= 7;
      buffer++;
      if(buffer > 24)
      {
         buffer = 1;
      }
      RTOS_MSEC_DELAY(500);
   }
   for(;;)
   {
      color++;
      color &= 7;
      for(i = 0;i < 64;i++)
      {
         for(j = 0;j < 160;j++)
         {
            putPixel(buffer,j,i,color);
//           RTOS_MSEC_DELAY(1);
         }
      }

      pickBuffer(buffer);
      buffer++;
      buffer &= 1;
      clearBuffer(buffer);
      RTOS_MSEC_DELAY(500);
#endif
   }

   /* USER CODE END StartDefaultTask */
}


/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void)
{
   // enable the cycle counter
   CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
   DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
   DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
   DWT->CYCCNT = 0;
   /* Create the thread(s) */
   /* definition and creation of defaultTask */
   osThreadDef(defaultTask,StartDefaultTask,osPriorityNormal,0,128);
   defaultTaskHandle = osThreadCreate(osThread(defaultTask),NULL);
}

/* StartDefaultTask function */

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicrOETlectronics *****END OF FILE****/
