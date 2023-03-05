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
 * Display Application 
 * 
 *  
 * Charles M Nowell, Jr. 
 * 2019 
 *  
 */

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"


#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

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
#include "usart4.h"
#include "usart1.h"
#include "DisplayDriver.h"
#include "DisplayApplication.h"
#include "crc.h"
#include "flashinterface.h"

// Free RTOS task handles.
osThreadId defaultTaskHandle;
osThreadId receiverTaskHandle;
osThreadId radioReceiverTaskHandle;
osThreadId buttonTaskHandle;


void (*TickHook)(void);

void vApplicationTickHook(void)
{
   if(TickHook)
   {
      (*TickHook)();
   }
}
/**
 * @brief Needed by static task allocation model if used.
 *
 * 
 * @author Charles "Mickey" Nowell (12/31/2019)
 * 
 * @param ppxIdleTaskTCBBuffer 
 * @param ppxIdleTaskStackBuffer 
 * @param pulIdleTaskStackSize 
 */
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


/**
 * Storage for the mode selected. 
 * Puts the mode in flash so it is remembered every time. 
 * One time setup stuff. 
 */
#define MODE_SIZE_FOR_CRC  8
#define SIGNATURE   0xBEEFBABE
/**
 * @brief Recover the mode from sector 1 in flash if the
 *        signature and crc both pass.
 *
 * 
 * @author Charles "Mickey" Nowell (12/28/2019)
 * 
 * @return bool 
 */
bool recoverMode(void)
{
   uint8_t *address;
   uint32_t *mode;
   address = (uint8_t *)PARAMETER_ADDRESS;
   if(calcCRC(address,MODE_SIZE_FOR_CRC + 2) == 0)
   {
      mode = (uint32_t *)PARAMETER_ADDRESS;
      if(mode[0] == SIGNATURE)
      {
         setMode(mode[1]);
         return(true);
      }
   }
   return(false);
}

/**
 * @brief Commit the current mode to flash for use after a power
 *        cycle.
 *
 * 
 * @author Charles "Mickey" Nowell (12/28/2019)
 */
void commitMode(void)
{
   uint32_t modeBuffer[3];
   modeBuffer[0] = SIGNATURE;
   modeBuffer[1] = currentMode;
   addCRC16((uint8_t *)modeBuffer,MODE_SIZE_FOR_CRC);
   eraseParameters();
   writeFlash((uint8_t *)modeBuffer,MODE_SIZE_FOR_CRC + 2);
}

void putAPixel(int32_t x,int32_t y, uint32_t color);


/**
 * @brief This tasks monitors the button in a rather simple way.
 *        Then short press cycles through the modes. Long press
 *        of greater than 1 second, commits the mode to flash.
 *        Used for initial setup.
 *
 * 
 * @author Charles "Mickey" Nowell (12/28/2019)
 * 
 * @param argument 
 */
void buttonTask(void const *argument)
{
   int32_t downEdgeTime;
   int32_t i;
   int32_t j;
   INITGPIOIN(BUTTON);

   RTOS_MSEC_DELAY(500);

   RTOS_MSEC_DELAY(250);
   #warning put this back
   #if 1
   showData(10,00,1,'A',0,FLY_MIDDLE,GREEN);
   changeImagePlane();
   #else
   while(1)
   {
      for(j = 0;j < 64;j++)
      {
         for(i = 0;i < 32 * 5;i++)
         {
            clearImage(0);
            putAPixel(i,j,7);
            primeTheBuffer(true);
            changeImagePlane();
            RTOS_MSEC_DELAY(5);
         }
      }
   }
   #endif


   while(1)
   {
      if(ISLOW(BUTTON))
      {
         // debounce
         RTOS_MSEC_DELAY(10);
         if(ISLOW(BUTTON))
         {
            downEdgeTime = OSTIME();
            // at least held
            while(1)
            {
               if(ISHIGH(BUTTON))
               {
                  int32_t delta;
                  delta = OSTIME() - downEdgeTime;
                  if((delta > 50) && (delta < 1000))
                  {
                     incrementMode();
                     showData(10,59,18,1 + 'A',2,FLY_MIDDLE,GREEN);
                     changeImagePlane();
                  }
                  else if(delta > 1000)
                  {
                     commitMode();
                  }
                  break;
               }


            }
         }
      }
      else
      {
         // The effective sample rate
         RTOS_MSEC_DELAY(50);
      }
   }
}


/**
 * @brief Parse the data from the incoming command. MBT format
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 * 
 * @param buffer 
 * @param phase 
 * @param seconds 
 * @param round 
 * @param heat 
 * @param flight 
 * 
 * @return bool 
 */
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

#define INFO_COLOR    CYAN

/**
 * @brief Paint the display buffer with the new data.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param minutes 0-1
 * @param seconds 0-60 ok, but usually 59
 * @param round 1....N
 * @param heat  'A', 'B' , ... etc, i.e. first heat is 0x41
 * @param flight 0 says only one flight so dont' put up the
 *               flight number
 * @param symbol icon on the left, position is indicated as part
 *               of symbol
 * @param color color of the digits, color of symbol is implied
 */
void showData(uint32_t minutes, uint32_t seconds, uint32_t round, uint32_t heat, int32_t flight, uint32_t symbol, uint32_t color)
{

   clearImage(0);

   if(displayMode.do2x5)
   {
      putColon(color);
      putValue64(seconds % 10,ONES_PLACE,color);
      putValue64(seconds / 10,TENS_PLACE,color);
      putValue64(minutes % 10,MINUTES_PLACE,color);
      if(minutes / 10)
      {
         putValue64(minutes / 10,TENS_OF_MINUTES_PLACE,color);
      }
      putStage(color,symbol);
      if(flight > 0)
      {

         switch(symbol)
         {
            case STOP_TOP:
               putHalfBigValue(flight,PIXELS_HIGH - 14,10,WHITE);
               break;
            case FLY_TOP:
               putHalfBigValue(flight,PIXELS_HIGH - 14,10,WHITE);
               break;
            case LAND_TOP:
               putHalfBigValue(flight,PIXELS_HIGH - 14,10,WHITE);
               break;
            case STOP_MIDDLE:
               putHalfBigValue(flight,PIXELS_HIGH - 14,10,WHITE);
               break;
            case FLY_MIDDLE:
               putHalfBigValue(flight,PIXELS_HIGH - 14,10,WHITE);
               break;
            case LAND_MIDDLE:
               putHalfBigValue(flight,PIXELS_HIGH - 14,10,WHITE);
               break;
            case STOP_BOTTOM:
               break;
            case FLY_BOTTOM:
               break;
            case LAND_BOTTOM:
               break;
         }
      }
   }
   else
   {
      // this is the 1x4 display, or a back to back 1x4
      putLittleColon(color);
      putValue(seconds % 10,SMALL_ONES_PLACE,color);
      putValue(seconds / 10,SMALL_TENS_PLACE,color);
      putValue(minutes % 10,SMALL_MINUTES_PLACE,color);
      if(minutes / 10)
      {
         putValue(minutes / 10,TENS_OF_SMALL_MINUTES_PLACE,color);
      }
      switch(symbol)
      {
         case STOP_TOP:
            putSymbol(6,SIGN_PLACE,RED);
            break;
         case FLY_TOP:
            putSymbol(4,SIGN_PLACE,GREEN);
            break;
         case LAND_TOP:
            putSymbol(6,SIGN_PLACE,RED);
            break;
         case STOP_MIDDLE:
            putSymbol(6,SIGN_PLACE,RED);
            break;
         case FLY_MIDDLE:
            putSymbol(4,SIGN_PLACE,GREEN);
            break;
         case LAND_MIDDLE:
            putSymbol(6,SIGN_PLACE,RED);
            break;
         case STOP_BOTTOM:
            putSymbol(6,SIGN_PLACE,RED);
            break;
         case FLY_BOTTOM:
            putSymbol(4,SIGN_PLACE,GREEN);
            break;
         case LAND_BOTTOM:
            putSymbol(6,SIGN_PLACE,RED);
            break;

      }
      if(round > 9)
      {
         putHalfBigValue(round / 10,0,0,INFO_COLOR);
         putHalfBigValue(round % 10,0,10,INFO_COLOR);
      }
      else
      {
         putHalfBigValue(round,0,0,INFO_COLOR);
      }
      putHalfBigLetter(heat - 'A',16,0,INFO_COLOR);
      if(flight > 0)
      {
         putHalfBigValue(flight,16,10,INFO_COLOR);
      }
   }
   primeTheBuffer(true);
}

/**
 * @brief Show the round and heat only in the given color. This
 *        is for announcement time before prep time.
 *
 * 
 * @author Charles "Mickey" Nowell (1/1/2020)
 * 
 * @param round 
 * @param heat 
 * @param color 
 */
void showRoundAndHeat(uint32_t round, uint32_t heat,uint32_t color)
{

   clearImage(0);

   if(displayMode.do2x5)
   {
      putBigLetter64(heat,ONES_PLACE,color);
      putValue64(round % 10,MINUTES_PLACE,color);
      if(round / 10)
      {
         putValue64(round / 10,HIGH_ROUND_PLACE,color);
      }
   }
   else
   {
      // this is the 1x4 display, or a back to back 1x4
      putBigLetter(heat,SMALL_ONES_PLACE,color);
      putValue(round % 10,SMALL_MINUTES_PLACE,color);
      if(round / 10)
      {
         putValue(round / 10,TENS_OF_SMALL_MINUTES_PLACE,color);
      }
   }
   primeTheBuffer(true);

}

void showRoundAndHeatNumericGroup(uint32_t round, uint32_t heat,uint32_t color)
{

   clearImage(0);

   if(displayMode.do2x5)
   {
      putColon(color);
      // get relative to start of array
      putBigLetter64('G' - 'A',TENS_PLACE,color);
      // putBigLetter64(heat,ONES_PLACE,color);
      putValue64(heat,ONES_PLACE,color);
      putValue64(round % 10,MINUTES_PLACE,color);
      if(round / 10)
      {
         putValue64(round / 10,HIGH_ROUND_PLACE,color);
      }
   }
   else
   {
      // this is the 1x4 display, or a back to back 1x4
      putBigLetter(heat,SMALL_ONES_PLACE,color);
      putValue(round % 10,SMALL_MINUTES_PLACE,color);
      if(round / 10)
      {
         putValue(round / 10,TENS_OF_SMALL_MINUTES_PLACE,color);
      }
   }
   primeTheBuffer(true);

}

/**
 * @brief Clear the display and just show the time. Used for TOD
 *        or for idle time when the timer is inactive.
 *
 * 
 * @author Charles "Mickey" Nowell (12/31/2019)
 * 
 * @param minutes 
 * @param seconds 
 * @param color 
 */
void showTimeOnly(uint32_t minutes, uint32_t seconds,uint32_t color)
{

   clearImage(0);

   if(displayMode.do2x5)
   {
      putColon(color);
      putValue64(seconds % 10,ONES_PLACE,color);
      putValue64(seconds / 10,TENS_PLACE,color);
      putValue64(minutes % 10,MINUTES_PLACE,color);
      if(minutes / 10)
      {
         putValue64(minutes / 10,TENS_OF_MINUTES_PLACE,color);
      }
      putHalfBigLetter('T' - 'A',0,0,GREEN);
      putHalfBigLetter('I' - 'A',16,0,GREEN);
      putHalfBigLetter('M' - 'A',32,0,GREEN);
      putHalfBigLetter('E' - 'A',48,0,GREEN);

   }
   else
   {
      // this is the 1x4 display, or a back to back 1x4
      putLittleColon(color);
      putValue(seconds % 10,SMALL_ONES_PLACE,color);
      putValue(seconds / 10,SMALL_TENS_PLACE,color);
      putValue(minutes % 10,SMALL_MINUTES_PLACE,color);
      if(minutes / 10)
      {
         putValue(minutes / 10,TENS_OF_SMALL_MINUTES_PLACE,color);
      }
   }
   primeTheBuffer(true);
}


// Current data
uint32_t phase,secondsIn,round,heat,flight;
/**
 * @brief Look at an incoming string and repaint the display
 *        with new data. Accepts MBT protocol and
 *        GliderScore/Embedded Ability protocol legacy and
 *        enhanced.
 *
 * 
 * @author Charles "Mickey" Nowell (1/1/2020)
 * 
 * @param inputBuffer 
 */
void parseCommand(char *inputBuffer)
{

   switch(inputBuffer[0])
   {

      case 'A':
         minutes = (toupper(inputBuffer[1]) - '0') * 10 + toupper(inputBuffer[2]) - '0';
         seconds = (toupper(inputBuffer[3]) - '0') * 10 + toupper(inputBuffer[4]) - '0';
         showTimeOnly(minutes,seconds,GREEN);
         changeImagePlane();
         break;
      case 'R':
         if(inputBuffer[1] == ':')
         {
            // old legacy command
            // ignore
            break;
         }
         // Perform GliderScore interpretation
         // Standard "Embedded-Ability" format: "Ammss+CR"
         // ASCII character string "Ammss \r" (the letter A, the minutes digits, the seconds digits, the
         // ASCII character for Carriage Return).
         // Extended Protocol format: "R99G99TmmssAA+CR"
         // R99 - 99 is the round number.
         // Sample R10G02T0947WT
         // R10G02T0947PT
         // R10G14T0100PT
         // R10G15T0059PT
         // R10G02T0947ST
         // R10G02T0947DT    R10G02T1247DT
         // R10G02T0947LT
         // G99 - 99 is the group number.
         // Tmmss, there "mm" is 2 digits for minutes and "ss" is 2 digits for seconds.
         // AA - Two character code for timer state: PT prep time, WT working time, LT landing time, ST sleep time, DT display time-of-day.
         // CR - ASCII Carriage Return.
         round =   (toupper(inputBuffer[1]) - '0') * 10 + toupper(inputBuffer[2]) - '0';
         heat =    (toupper(inputBuffer[4]) - '0') * 10 + toupper(inputBuffer[5]) - '0';
         // heat is 1 based in gliderscore world
         // we need to map 1 = A, to 0 = A for this code.
         heat--;
         minutes = (toupper(inputBuffer[7]) - '0') * 10 + toupper(inputBuffer[8]) - '0';
         seconds = (toupper(inputBuffer[9]) - '0') * 10 + toupper(inputBuffer[10]) - '0';
         flight = 0;
         switch(inputBuffer[11])
         {
            case 'P':
               // Show stop sign if time is less than or equal to 1 minute
               // no fly time for F3K, warning time for other disciplines
               if(minutes == 0)
               {
                  showData(minutes,seconds,round,heat + 'A',flight,STOP_TOP,YELLOW);
               }
               else if((minutes == 1) && (seconds == 0))
               {
                  showData(minutes,seconds,round,heat + 'A',flight,STOP_TOP,YELLOW);
               }
               else
               {
                  showData(minutes,seconds,round,heat + 'A',flight,FLY_TOP,YELLOW);
               }
               changeImagePlane();
               break;
            case 'W':
               showData(minutes,seconds,round,heat + 'A',flight,FLY_MIDDLE,GREEN);
               changeImagePlane();
               break;
            case 'L':
               showData(minutes,seconds,round,heat + 'A',flight,LAND_BOTTOM,RED);
               changeImagePlane();
               break;
            case 'S':
               // announcement time no clock running
               showRoundAndHeatNumericGroup(round,heat + 1,WHITE);
               changeImagePlane();
               break;
            default:
               showTimeOnly(minutes,seconds,CYAN);
               changeImagePlane();
               break;
         }
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
                  changeImagePlane();
                  break;
               case 1:
                  // window flying allowed
                  showData(minutes,seconds,round,heat + 'A',flight,FLY_MIDDLE,GREEN);
                  changeImagePlane();
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
                  changeImagePlane();
                  break;
               case 3: // no fly in all up
                  showData(minutes,seconds,round,heat + 'A',flight,STOP_MIDDLE,RED);
                  changeImagePlane();
                  break;
               case 4: // break between rounds
                  showData(minutes,seconds,round,heat + 'A',0,FLY_TOP,CYAN);
                  changeImagePlane();
                  break;
               case 5: // prep time, no flying
                  showData(minutes,seconds,round,heat + 'A',flight,STOP_TOP,YELLOW);
                  changeImagePlane();
                  break;
               case 6:
                  // announcement time no clock running
                  showRoundAndHeat(round,heat,WHITE);
                  changeImagePlane();
                  break;
            }
         }
         break;
      case 'S': // obsolete maybe
         changeImagePlane();
         break;
   }
}








/**
 * @brief Sit on the XBEE port and wait for commands.
 *
 * 
 * @author Charles "Mickey" Nowell (12/31/2019)
 * 
 * @param argument 
 */
void radioReceiverTask(void const *argument)
{
   static char inputBuffer[80];
   uart2Init(9600);
   while(1)
   {
      RTOS_MSEC_DELAY(1);
      uart2Gets((uint8_t *)inputBuffer);
      uart2Puts((uint8_t *)inputBuffer);
      uart2Puts((uint8_t *)"\r");
      parseCommand(inputBuffer);
   }
}


/**
 * @brief This task listens on one of the serial ports for
 *        incoming commands. This one on uart2. This one also
 *        sets up the display mode. Then it launches the other
 *        two listener threads.
 *
 * 
 * @author Charles "Mickey" Nowell (12/28/2019)
 * 
 * @param argument 
 */
void receiverTask(void const *argument)
{
   static char inputBuffer[80];

   // Enable RS485 Receive.
   INITGPIOOUT(RS485RX);
   // Disable transmit
   INITGPIOOUT(RS485TX);
   uart1Init(9600);

   if(!recoverMode())
   {
      setMode(0);
   }
   #if 0 // sampler code
      #define DEMO_TIME 5000
   while(1)
   {
      showRoundAndHeat(9,3,WHITE);
      changeImagePlane();
      RTOS_MSEC_DELAY(DEMO_TIME);
      showRoundAndHeat(10,0,WHITE);
      changeImagePlane();
      RTOS_MSEC_DELAY(DEMO_TIME);

      showData(3,0,10,'A',0,STOP_TOP,YELLOW);
      changeImagePlane();
      RTOS_MSEC_DELAY(DEMO_TIME);
      showData(9,45,10,'A',0,FLY_MIDDLE,GREEN);
      changeImagePlane();
      RTOS_MSEC_DELAY(DEMO_TIME);
      showData(0,30,10,'A',0,LAND_BOTTOM,RED);
      changeImagePlane();
      RTOS_MSEC_DELAY(DEMO_TIME);

      showTimeOnly(12,15,CYAN);
      changeImagePlane();
      RTOS_MSEC_DELAY(DEMO_TIME);
   }
   #else
   while(1)
   {
      uart1Gets((uint8_t *)inputBuffer);
      uart2Puts((uint8_t *)inputBuffer);
      uart2Puts((uint8_t *)"\r");
      parseCommand(inputBuffer);
   }
   #endif
}




/**
 * @brief Start the two threads to do everything. Then just
 *        idle.
 *
 * 
 * @author Charles "Mickey" Nowell (12/8/2019)
 * 
 * @param argument 
 */
void StartDefaultTask(void const *argument)
{
   static char inputBuffer[80];
   // start the scanner task
   osThreadDef(scannerTask,scannerTask,(osPriority)15,0,128);
   scannerTaskHandle = osThreadCreate(osThread(scannerTask),NULL);
   osThreadDef(receiverTask,receiverTask,(osPriority)10,0,128);
   receiverTaskHandle = osThreadCreate(osThread(receiverTask),NULL);
   osThreadDef(radioReceiverTask,radioReceiverTask,(osPriority)9,0,128);
   radioReceiverTaskHandle = osThreadCreate(osThread(radioReceiverTask),NULL);


   osThreadDef(buttonTask,buttonTask,(osPriority)8,0,128);
   buttonTaskHandle = osThreadCreate(osThread(buttonTask),NULL);

   uart4Init(115200);

   while(1)
   {
      uart4Gets((uint8_t *)inputBuffer);
      parseCommand(inputBuffer);
   }
}



void startDisplayDriver(void)
{
   /* USER CODE BEGIN Init */
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

#if 0 // Test code
void animate(void)
{
   int i;
   int j = 0;
   int x;
   int number;
   // uint32_t rgb[3];
   //
   // rgb[0] = RED;
   // rgb[1] = WHITE;
   // rgb[2] = BLUE;

   #if 0
   while(1)
   {
      clearImage(0);
      changeImagePlane();
      for(i=320-64;i<320;i++)
      {
         displayBufferOneEighthScan[showFrame][0][i] |= 3; // color << (shift * 3);
         RTOS_MSEC_DELAY(50);
      }
   }
   #endif
   // j = 0;
   // while(1)
   // {
   //    clearImage(0xFFF);
   //    changeImagePlane();
   //    RTOS_MSEC_DELAY(1000);
   //    j++;
   // }

   j = 7;
   number = 0;
   while(1)
   {
      x = 0;

      if(displayMode.doInverted)
      {
         for(i = 32 - 24;i < 64;i++)
         {
            clearImage(0);


            putValueXY(number,i,x++,j & 7);
            number++;
            if(number > 9)
            {
               number = 0;
            }
            changeImagePlane();
            RTOS_MSEC_DELAY(500);
         }

         j++;
         if((j & 7) == 0)
         {
            j++;
         }
      }
      else
      {
         //for(i = -32;i < 32;i++)
         {
            clearImage(0);
            // putValueXY(number,3*32+ x++,0,j&7);
            // putValueXY(number,4*32,0,1);
            // putValueXY(j&7,4*32-WIDTH-2,i,);
            // putValueXY(number,4*32,0,rgb[j]);
            // putValueXY(j&7,4*32-WIDTH-2,i,rgb[j]);
            putValueXY(number % 10,4 * 32,0,j & 7);
            putValueXY(number / 10,4 * 32 - SMALL_WIDTH - 2,i,j & 7);

            changeImagePlane();
            RTOS_MSEC_DELAY(1000);
            number++;
            number %= 60;
            // number++;
            // if(number > 9)
            // {
            //    number = 0;
            // }
         }
         j++;
         // if(j >= 3)
         // {
         //    j = 0;
         // }
         if((j & 7) == 0)
         {
            j++;
         }
      }
   }
}
#endif

