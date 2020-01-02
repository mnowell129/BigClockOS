/**
 * Display driver. 
 * DisplayDriver.c 
 * Panel driver for the big timer. 
 *  
 * Charles M Nowell, Jr. 
 * 2019 
 *  
 * This application uses the startup code from the sample 
 * freertos example. 
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
#include "DisplayDriverInternal.h"
#include "DisplayDriver.h"


// Free RTOS task junk.
osThreadId scannerTaskHandle;



#if 0
/**
 * @brief Boilerplate function, part of the ST micro library
 *        stuff. Uses the cycles counter to delay a small amount
 *        of time much smaller than the RTOS tick
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 * 
 * @param CLOCKTs 
 * 
 * @return __STATIC_INLINE void 
 */
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

#endif


/*
These are definitions for the STM32F446 using a STM32F446RE Nucleo board as a base. 
PORT A controls the bank bits, Port C the colors. 
This driver will control either a 1x4 32x32 panel, two 1x4 32x32 panels, or one 2x5 32x32 panels. 
Both 1/16 and 1/8 scan panels can be used. 
For 1/8 scan the putPixel routine maps the pixels in the odd way needed. 
*/
/* 
   Additional notes:
   Port C drives the color. There are 12 bits. Bits 0-2 are RGB:0 on the top panel, Bits 3-5 are RGB:1 on the top panel.
   In the prototype the display is mounted upside down, so the top connector is physically on the bottom.
   Bits 6-8 are RGB:0 on the bottom connector, 9-11 are RGB:1 on the bottom connector.
 
   You can drive the screen from the right hand end, from the back, this is the left hand end from the front.
   To do this you have to invert the display so that from the back the arrows on the panels point down and left.
   Then all the pixels have to be mapped inverted and swapped left to right. Assuming that row 0 and column 0
   is the upper left hand corner. This is actually the last row scanned, and the last pixel scanned out.
   In other words pixel 0,0 is the 4th pixel up on the 12 bits, in row 15 for a 1/16 scan panel.
   For 1/8 scan, it's row 7 but mapped to a different X position because of the strange mapping of the scan
   sequence on the 1/8 scan panels.
   To make the display upright a couple things have to be reversed. Exactly what? It is confusing.
   Pixel 0,0 becomes row 0, x position 0, because it is the very first pixel scanned on the first row.
   So the first reversing step is to not subract from x and y maximums, so x and y are unaltered.
   I think this works, but needs to be confirmed.
 
*/
/*
Some explanation. 
For the 1/8 scan panels the scan order is funny. 
The display is 32 pixels wide, row 0 and row 8 (that is row 0..7 and rows 8..15) are one 
continous shift sequence. But the shift is not one row then the next. First 16 pixels of row 
0, this is followed by 32 pixels of row 8, followed by 16 pixels of row 0. 
This makes chaining panels together strange. This code maps an XY pixel to the proper scan location 
in a scan output. There are 320 pixels in one scan row. This covers two rows of the panel, e.g. 0 and 7. 
The color is selected by R0,B0 and G0 pins. On the same connector R1,B1 and G1 make a 320 (for 5 panels) 
pixel set that is row 16 and 24. Changing the bank produces row 1,9,27 and 25, etc. 
On the other connector is the same, this makes the lower row of panels. 
The bank is common for both upper and lower panels. The pixels are unique. 
It takes 3 bits to make one color on one row, that is Upper:R0B0G0. Likewise 3 bits for the lower rows of 
the upper panel Upper:R1B1G1. It takes 6 bits to make the colors for the lower panel. 
To save memory the array is 16 bits. This holds the 12 bits needed. 
The putpixel routing knows how to unwind all this packing and the weird scan order so that the scanner, 
that is refreshing the display just outputs the 12 color bits to the panels by scanning through 
a pixel row. 
For 1/16 scan panels the putpixel routine has less work to do as the pixel rows are not 
interleaved. 
For a dual single high display the putpixel routine will just make the upper and lower pixels the same. 
For a single double high display the putpixel routine doesnt duplicate the pixel. 
For single or double high, the display driver calls the correct sized font routines and icons 
for either the half high or double high display. 
 
*/
/* 
This is the electrical scan order for physical rows 0/8, 1/9, 2/10, etc. 
Bits R0:B0:G0 Are driven with clock, latch and OE. 
00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 
For Physical rows 16/24, 17/25... Another set of bits are used R1:B1:G1. 
Note that this makes the mapping odd for series panels, for example consider how to reference 
the middle pixel of row 8 on the last panel. 
It gets slightly easier because their are 64 pixels per panel, so only the funny math applies to 
the last panel being considered. 
 
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
Single high 1/8 scan mapping Upright panel
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
 
1/8 scan mapping Upright panel             pixels scanned out in time >>>>   i.e. pixel 0 is the first one shifted out 
                                           physical coordinate colum
                                           00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
                                                                                           |----------------------------------<<<----------------¬
scan row 00:RGB0 Shift 0 Physical Row 00 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 -->   |
                                                                                        |------------------------------>>---------------------¬  |
scan row 01:RGB0 Shift 0 Physical Row 01 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 02:RGB0 Shift 0 Physical Row 02 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 0 Physical Row 03 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 0 Physical Row 04 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 0 Physical Row 05 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 0 Physical Row 06 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 0 Physical Row 07 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 0 Physical Row 08 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47------->
scan row 01:RGB0 Shift 0 Physical Row 09 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 0 Physical Row 10 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 0 Physical Row 11 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 0 Physical Row 12 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 0 Physical Row 13 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 0 Physical Row 14 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 0 Physical Row 15 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
scan row 00:RGB0 Shift 1 Physical Row 16 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 1 Physical Row 17 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 1 Physical Row 18 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 1 Physical Row 19 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 1 Physical Row 20 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 1 Physical Row 21 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 1 Physical Row 22 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 1 Physical Row 23 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 1 Physical Row 24 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 1 Physical Row 25 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 1 Physical Row 26 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 1 Physical Row 27 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 1 Physical Row 28 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 1 Physical Row 29 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 1 Physical Row 30 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 1 Physical Row 31 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
 
 
 
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
Double high 1/8 scan mapping Upright panel
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
 
1/8 scan mapping Upright panel             pixels scanned out in time >>>>   i.e. pixel 0 is the first one shifted out 
                                           physical coordinate colum
                                           00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
                                                                                           |----------------------------------<<<----------------¬
scan row 00:RGB0 Shift 0 Physical Row 00 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 -->   |
                                                                                        |------------------------------>>---------------------¬  |
scan row 01:RGB0 Shift 0 Physical Row 01 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 02:RGB0 Shift 0 Physical Row 02 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 0 Physical Row 03 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 0 Physical Row 04 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 0 Physical Row 05 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 0 Physical Row 06 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 0 Physical Row 07 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 0 Physical Row 08 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47------->
scan row 01:RGB0 Shift 0 Physical Row 09 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 0 Physical Row 10 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 0 Physical Row 11 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 0 Physical Row 12 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 0 Physical Row 13 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 0 Physical Row 14 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 0 Physical Row 15 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
scan row 00:RGB0 Shift 1 Physical Row 16 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 1 Physical Row 17 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 1 Physical Row 18 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 1 Physical Row 19 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 1 Physical Row 20 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 1 Physical Row 21 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 1 Physical Row 22 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 1 Physical Row 23 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 1 Physical Row 24 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 1 Physical Row 25 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 1 Physical Row 26 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 1 Physical Row 27 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 1 Physical Row 28 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 1 Physical Row 29 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 1 Physical Row 30 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 1 Physical Row 31 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
 
 
scan row 00:RGB0 Shift 2 Physical Row 32 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 -->   |
                                                                                        |------------------------------>>---------------------¬  |
scan row 01:RGB0 Shift 2 Physical Row 33 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 02:RGB0 Shift 2 Physical Row 34 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 2 Physical Row 35 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 2 Physical Row 36 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 2 Physical Row 37 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 2 Physical Row 38 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 2 Physical Row 39 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 2 Physical Row 40 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47------->
scan row 01:RGB0 Shift 2 Physical Row 41 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 2 Physical Row 42 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 2 Physical Row 43 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 2 Physical Row 44 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 2 Physical Row 45 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 2 Physical Row 46 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 2 Physical Row 47 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
scan row 00:RGB0 Shift 3 Physical Row 48 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 3 Physical Row 49 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 3 Physical Row 50 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 3 Physical Row 51 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 3 Physical Row 52 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 3 Physical Row 53 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 3 Physical Row 54 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 3 Physical Row 55 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 3 Physical Row 56 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 3 Physical Row 57 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 3 Physical Row 58 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 3 Physical Row 59 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 3 Physical Row 60 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 3 Physical Row 61 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 3 Physical Row 62 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 3 Physical Row 63 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 <--- 
The last pixel above is physical row (y) 63, x = 31 
 
 
2.0 Alternate mapping 
1/8 scan mapping Upright panel             pixels scanned out in time >>>>   i.e. pixel 0 is the first one shifted out 
                                           physical coordinate colum
                                           000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 020 021 022 023 024 025 026 027 028 029 030 031
  
scan row 01:RGB0 Shift 0 Physical Row 01 : 000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 032 033 034 035 036 037 038 039 040 041 042 043 044 045 046 047    |  | 
scan row 08:RGB0 Shift 0 Physical Row 08 : 016 017 018 019 020 021 022 023 024 025 026 027 028 029 030 031 048 049 050 051 052 053 054 055 056 057 058 059 060 061 062 063-------> 
 
// Second panel                            032 033 034 035 036 037 038 039 040 041 042 043 044 045 046 047 048 049 050 051 052 053 054 055 056 057 058 059 060 061 062 063
scan row 01:RGB0 Shift 0 Physical Row 01 : 064 065 066 067 068 069 070 071 072 073 074 075 076 077 078 079 096 097 098 099 100 101 102 103 104 105 106 107 108 109 110 111 
scan row 08:RGB0 shift 0 Physcial Row 08 : 080 081 082 083 084 085 086 087 088 089 090 091 092 093 094 095 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127 
 
// third panel                             064                                                         79  80                                                          95
scan row 01:RGB0 Shift 0 Physical Row 01 : 128                                                        143  160                                                         175 
scan row 08:RGB0 shift 0 Physcial Row 08 : 144                                                        159  176                                                         191
 
 
 
scan row 02:RGB0 Shift 0 Physical Row 02 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 0 Physical Row 03 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 0 Physical Row 04 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 0 Physical Row 05 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 0 Physical Row 06 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 0 Physical Row 07 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 0 Physical Row 08 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63------->
scan row 01:RGB0 Shift 0 Physical Row 09 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 0 Physical Row 10 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 0 Physical Row 11 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 0 Physical Row 12 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 0 Physical Row 13 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 0 Physical Row 14 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 0 Physical Row 15 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
scan row 00:RGB0 Shift 1 Physical Row 16 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 1 Physical Row 17 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 1 Physical Row 18 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 1 Physical Row 19 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 1 Physical Row 20 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 1 Physical Row 21 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 1 Physical Row 22 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 1 Physical Row 23 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 1 Physical Row 24 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 1 Physical Row 25 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 1 Physical Row 26 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 1 Physical Row 27 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 1 Physical Row 28 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 1 Physical Row 29 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 1 Physical Row 30 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 1 Physical Row 31 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
 
 
scan row 00:RGB0 Shift 2 Physical Row 32 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 -->   |
                                                                                        |------------------------------>>---------------------¬  |
scan row 01:RGB0 Shift 2 Physical Row 33 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 02:RGB0 Shift 2 Physical Row 34 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 2 Physical Row 35 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 2 Physical Row 36 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 2 Physical Row 37 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 2 Physical Row 38 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 2 Physical Row 39 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 2 Physical Row 40 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47------->
scan row 01:RGB0 Shift 2 Physical Row 41 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 2 Physical Row 42 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 2 Physical Row 43 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 2 Physical Row 44 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 2 Physical Row 45 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 2 Physical Row 46 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 2 Physical Row 47 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
scan row 00:RGB0 Shift 3 Physical Row 48 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 3 Physical Row 49 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 3 Physical Row 50 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 3 Physical Row 51 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 3 Physical Row 52 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 3 Physical Row 53 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 3 Physical Row 54 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 3 Physical Row 55 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 3 Physical Row 56 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 3 Physical Row 57 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 3 Physical Row 58 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 3 Physical Row 59 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 3 Physical Row 60 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 3 Physical Row 61 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 3 Physical Row 62 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 3 Physical Row 63 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 <--- 
The last pixel above is physical row (y) 63, x = 31 
 
 
 
 
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
Single high 1/8 scan mapping Inverted panel
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
 
1/8 scan mapping Inverted panel            pixels scanned out in time >>>>   i.e. pixel 0 is the last one shifted out 
                                           physical coordinate colum
Shift 0 ends up on REGB0 of the top connector, i.e. the top 16 rows 

 
                                           31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00  
                                                                                           |----------------------------------<<<----------------¬
scan row 00:RGB0 Shift 0 Physical Row 31 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 -->   |
                                                                                        |------------------------------>>---------------------¬  |
scan row 01:RGB0 Shift 0 Physical Row 30 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 02:RGB0 Shift 0 Physical Row 29 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 0 Physical Row 28 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 0 Physical Row 27 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 0 Physical Row 26 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 0 Physical Row 25 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 0 Physical Row 24 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           physical coordinate colum                                                                          |  |
                                           31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 0 Physical Row 23 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47------->
scan row 01:RGB0 Shift 0 Physical Row 22 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 0 Physical Row 21 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 0 Physical Row 20 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 0 Physical Row 19 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 0 Physical Row 18 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 0 Physical Row 17 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 0 Physical Row 16 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
Shift 1 ends up on REGB1 of the top connector, i.e. the 2nd set 16 rows 
 
scan row 00:RGB0 Shift 1 Physical Row 15 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 1 Physical Row 14 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 1 Physical Row 13 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 1 Physical Row 12 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 1 Physical Row 11 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 1 Physical Row 10 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 1 Physical Row 09 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 1 Physical Row 08 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 1 Physical Row 07 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 1 Physical Row 06 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 1 Physical Row 05 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 1 Physical Row 04 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 1 Physical Row 03 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 1 Physical Row 02 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 1 Physical Row 01 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 1 Physical Row 00 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
 
 
 
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
Double high 1/8 scan mapping Inverted panel
**************************************************************************************************************************************** 
**************************************************************************************************************************************** 
 
1/8 scan mapping Inverted panel            pixels scanned out in time >>>>   i.e. pixel 0 is the last one shifted out 
                                           physical coordinate colum
Shift 0 ends up on REGB0 of the top connector, i.e. the top 16 rows 

 
                                           31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00  
                                                                                           |----------------------------------<<<----------------¬
scan row 00:RGB0 Shift 0 Physical Row 63 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 -->   |
                                                                                        |------------------------------>>---------------------¬  |
scan row 01:RGB0 Shift 0 Physical Row 62 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 02:RGB0 Shift 0 Physical Row 61 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 0 Physical Row 60 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 0 Physical Row 59 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 0 Physical Row 58 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 0 Physical Row 57 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 0 Physical Row 56 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           physical coordinate colum                                                                          |  |
                                           31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 0 Physical Row 55 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47------->
scan row 01:RGB0 Shift 0 Physical Row 54 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 0 Physical Row 53 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 0 Physical Row 52 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 0 Physical Row 51 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 0 Physical Row 50 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 0 Physical Row 49 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 0 Physical Row 48 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
Shift 1 ends up on REGB1 of the top connector, i.e. the 2nd set 16 rows 
 
scan row 00:RGB0 Shift 1 Physical Row 47 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 1 Physical Row 46 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 1 Physical Row 45 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 1 Physical Row 44 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 1 Physical Row 43 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 1 Physical Row 42 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 1 Physical Row 41 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 1 Physical Row 40 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 1 Physical Row 39 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 1 Physical Row 38 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 1 Physical Row 37 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 1 Physical Row 36 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 1 Physical Row 35 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 1 Physical Row 34 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 1 Physical Row 33 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 1 Physical Row 32 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
 
 
Shift 2 ends up on REGB0 of the bottom connector, i.e. the top 16 rows 
 
scan row 00:RGB0 Shift 2 Physical Row 31 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 -->   |
                                                                                        |------------------------------>>---------------------¬  |
scan row 01:RGB0 Shift 2 Physical Row 30 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 02:RGB0 Shift 2 Physical Row 29 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 03:RGB0 Shift 2 Physical Row 28 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 04:RGB0 Shift 2 Physical Row 27 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 05:RGB0 Shift 2 Physical Row 26 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 06:RGB0 Shift 2 Physical Row 25 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
scan row 07:RGB0 Shift 2 Physical Row 24 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63    |  |
                                           physical coordinate colum                                                                          |  |
                                           31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00    |  |
                                           |-----------------------------------------------------------------------------------------------<--|  |
scan row 00:RGB0 Shift 2 Physical Row 23 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47------->
scan row 01:RGB0 Shift 2 Physical Row 22 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 02:RGB0 Shift 2 Physical Row 21 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 03:RGB0 Shift 2 Physical Row 20 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 04:RGB0 Shift 2 Physical Row 19 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 05:RGB0 Shift 2 Physical Row 18 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 06:RGB0 Shift 2 Physical Row 17 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 07:RGB0 Shift 2 Physical Row 16 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
 
Shift 3 ends up on REGB1 of the bottom connector, i.e. the bottom 16 rows 
 
 
scan row 00:RGB0 Shift 3 Physical Row 15 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 01:RGB0 Shift 3 Physical Row 14 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 02:RGB0 Shift 3 Physical Row 13 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 03:RGB0 Shift 3 Physical Row 12 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 04:RGB0 Shift 3 Physical Row 11 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 05:RGB0 Shift 3 Physical Row 10 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 06:RGB0 Shift 3 Physical Row 09 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
scan row 07:RGB0 Shift 3 Physical Row 08 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
                                           
scan row 00:RGB0 Shift 3 Physical Row 07 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
scan row 01:RGB0 Shift 3 Physical Row 06 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 02:RGB0 Shift 3 Physical Row 05 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 03:RGB0 Shift 3 Physical Row 04 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 04:RGB0 Shift 3 Physical Row 03 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 05:RGB0 Shift 3 Physical Row 02 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 06:RGB0 Shift 3 Physical Row 01 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  
scan row 07:RGB0 Shift 3 Physical Row 00 : 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47  <-- this is physical 0,0 
 
So position 0,0 should end up on scan row 7, in position 47
 
 
 
 
 
1/16 scan mapping Upright panel            pixels scanned out in time >>>>   i.e. pixel 0 is the first one shifted out 
                                           physical coordinate colum 
                                           00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 00:RGB0 Shift 0 Physical Row 00 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 01:RGB0 Shift 0 Physical Row 01 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 02:RGB0 Shift 0 Physical Row 02 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 03:RGB0 Shift 0 Physical Row 03 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 04:RGB0 Shift 0 Physical Row 04 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 05:RGB0 Shift 0 Physical Row 05 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 06:RGB0 Shift 0 Physical Row 06 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 07:RGB0 Shift 0 Physical Row 07 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 08:RGB0 Shift 0 Physical Row 08 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 09:RGB0 Shift 0 Physical Row 09 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 10:RGB0 Shift 0 Physical Row 10 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 11:RGB0 Shift 0 Physical Row 11 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 12:RGB0 Shift 0 Physical Row 12 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 13:RGB0 Shift 0 Physical Row 13 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 14:RGB0 Shift 0 Physical Row 14 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 15:RGB0 Shift 0 Physical Row 15 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 00:RGB1 Shift 1 Physical Row 16 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 01:RGB1 Shift 1 Physical Row 17 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 02:RGB1 Shift 1 Physical Row 18 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 03:RGB1 Shift 1 Physical Row 19 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 04:RGB1 Shift 1 Physical Row 20 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 05:RGB1 Shift 1 Physical Row 21 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 06:RGB1 Shift 1 Physical Row 22 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 07:RGB1 Shift 1 Physical Row 23 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 08:RGB1 Shift 1 Physical Row 24 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 09:RGB1 Shift 1 Physical Row 25 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 10:RGB1 Shift 1 Physical Row 26 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 11:RGB1 Shift 1 Physical Row 27 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 12:RGB1 Shift 1 Physical Row 28 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 13:RGB1 Shift 1 Physical Row 29 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 14:RGB1 Shift 1 Physical Row 30 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
scan row 15:RGB1 Shift 1 Physical Row 31 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
 
1/16 scan mapping inverted panel           pixels scanned out in time >>>>   i.e. pixel 0 is the LAST one shifted out 
                                           physical coordinate colum
                                           31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00  
 
scan row  0:RGB0 Shift 0 Physical Row 31 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  1:RGB0 Shift 0 Physical Row 30 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  2:RGB0 Shift 0 Physical Row 29 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  3:RGB0 Shift 0 Physical Row 28 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  4:RGB0 Shift 0 Physical Row 27 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  5:RGB0 Shift 0 Physical Row 26 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  6:RGB0 Shift 0 Physical Row 25 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  7:RGB0 Shift 0 Physical Row 24 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  8:RGB0 Shift 0 Physical Row 23 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  9:RGB0 Shift 0 Physical Row 22 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 10:RGB0 Shift 0 Physical Row 21 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 11:RGB0 Shift 0 Physical Row 20 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 12:RGB0 Shift 0 Physical Row 19 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 13:RGB0 Shift 0 Physical Row 18 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 14:RGB0 Shift 0 Physical Row 17 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 15:RGB0 Shift 0 Physical Row 16 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  0:RGB1 Shift 1 Physical Row 15 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  1:RGB1 Shift 1 Physical Row 14 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  2:RGB1 Shift 1 Physical Row 13 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  3:RGB1 Shift 1 Physical Row 12 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  4:RGB1 Shift 1 Physical Row 11 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  5:RGB1 Shift 1 Physical Row 10 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  6:RGB1 Shift 1 Physical Row 09 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  7:RGB1 Shift 1 Physical Row 08 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  8:RGB1 Shift 1 Physical Row 07 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row  9:RGB1 Shift 1 Physical Row 06 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 10:RGB1 Shift 1 Physical Row 04 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 11:RGB1 Shift 1 Physical Row 04 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 12:RGB1 Shift 1 Physical Row 03 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 13:RGB1 Shift 1 Physical Row 02 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 14:RGB1 Shift 1 Physical Row 01 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
scan row 15:RGB1 Shift 1 Physical Row 00 : 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 
 
 
*/
// 320 pixels across, in a funny order
// 7 rows, 4 subrows per row i.e. 4 pixels per output word, 12 bits
// 16 bits is enough
/*
This is the display buffer 
16 bits holds the upper and lower pixels of a dual display. 
8 scan Rows per half panel (16 physical rows) 
So a row of 320 pixels holds 8 physical rows of pixels. 
4 of the rows are the 4 sets of 3 color bits, but doubled because 
the scan order does two rows in one shift sequence. 
So this buffer holds 64 physical rows by 160 pixels or a 2x5 array of 
32x32 pixels. 
For 1x4 small panels this memory will be organized differently 
in that the upper pixels will just be duplicates of the lower pixels. 
Then both panels will drive the same. 
For 1/16 scan panels the organization will be treated differently as the scan rows 
are simpler, but this is adjusted for in the putpixel routine. 
*/

/***************************************************************************************/
/***************************************************************************************/
/* Module data */
/***************************************************************************************/
/***************************************************************************************/
/**
 * @brief Pointer to call the correct set pixel function.
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 */
static   PutPixelFunction volatile putPixelPointer;
volatile uint32_t currentMode = 14;

volatile int32_t seconds;
volatile int32_t minutes;
// Experiment with sliding numbers
volatile int32_t _milliseconds;
volatile int32_t milliseconds;

// Make these settable by jumpers on pullups, perhaps override by command
// But this caused some confusion so perhaps better to make it fixed by hardware
// configuration.

volatile DisplayModeType displayMode =
{
   .doScan16 = false,
   .doInverted = false,
   .do2x5 = true,
   .alternate8Scan = true
};

#define ENTRY(a,b,c,d) {.doScan16 = a,.alternate8Scan = b,.doInverted = c,.do2x5 = d}
// Table for all possible displays, panel types and mounting.
DisplayModeType  displayModes[16] =
{

    ENTRY(false,false,false,false),
    ENTRY(false,false,false,true),
    ENTRY(false,false,true,false),
    ENTRY(false,false,true,true), 
    ENTRY(false,true,false,false),
    ENTRY(false,true,false,true),
    ENTRY(false,true,true,false),
    ENTRY(false,true,true,true),
    ENTRY(true,false,false,false),
    ENTRY(true,false,false,true),
    ENTRY(true,false,true,false),
    ENTRY(true,false,true,true),
    ENTRY(true,true,false,false),
    ENTRY(true,true,false,true),
    ENTRY(true,true,true,false),
    ENTRY(true,true,true,true),
};

/**
 * @brief Helper functions to remap colors based on the two
 *        different types of 1/8 scan displays (found so far).
 *        The alternate colors ones appear to be 2.0 displays.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param void 
 */
void setupColors(void)
{
   colors[0] = 0;
   colors[1] = _RED;
   colors[2] = _GREEN;
   colors[3] = _RED | _GREEN; // _YELLOW
   colors[4] = _BLUE;
   colors[5] = _BLUE | _RED; // _MAGENTA;
   colors[6] = _BLUE | _GREEN; // Cyan
   colors[7] = _RED | _GREEN | _BLUE;

}

void setupAlternateColors(void)
{
   colors[0] = 0;
   colors[1] = _ALT_RED;
   colors[2] = _ALT_GREEN;
   colors[3] = _ALT_RED | _ALT_GREEN; // _ALT_YELLOW
   colors[4] = _ALT_BLUE;
   colors[5] = _ALT_BLUE | _ALT_RED; // _ALT_MAGENTA;
   colors[6] = _ALT_BLUE | _ALT_GREEN; // Cyan
   colors[7] = _ALT_RED | _ALT_GREEN | _ALT_BLUE;

}



/**
 * @brief Setup the putPixel function call pointer. Use the
 *        current setup in the module variables.
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 * 
 * @param void 
 */
void setupPixelPutFunction(void)
{
   setupColors();
   if(displayMode.do2x5)
   {
      if(!displayMode.doInverted)
      {
         if(displayMode.doScan16)
         {
            putPixelPointer = putPixelScan16;
         }
         else
         {
            if(displayMode.alternate8Scan)
            {
               putPixelPointer = putPixelScan8Alternate;
               setupAlternateColors();
            }
            else
            {
               putPixelPointer = putPixelScan8;
            }
         }
      }
      else
      {
         if(displayMode.doScan16)
         {
            putPixelPointer = putPixelScan16Inverted;
         }
         else
         {
            if(displayMode.alternate8Scan)
            {
               putPixelPointer = putPixelScan8AlternateInverted;
               setupAlternateColors();
            }
            else
            {
               putPixelPointer = putPixelScan8Inverted;
            }

         }
      }
   }
   else
   {
      // 1x4 display
      if(!displayMode.doInverted)
      {
         if(displayMode.doScan16)
         {
            putPixelPointer = putPixelScan16SmallDisplay;
         }
         else
         {
            if(displayMode.alternate8Scan)
            {
               putPixelPointer = putPixelScan8AlternateSmallDisplay;
               setupAlternateColors();
            }
            else
            {
               putPixelPointer = putPixelScan8SmallDisplay;
            }
         }
      }
      else
      {
         if(displayMode.doScan16)
         {
            putPixelPointer = putPixelScan16InvertedSmallDisplay;
         }
         else
         {
            if(displayMode.alternate8Scan)
            {
               putPixelPointer = putPixelScan8AlternateInvertedSmallDisplay;
               setupAlternateColors();
            }
            else
            {
               putPixelPointer = putPixelScan8InvertedSmallDisplay;
            }

         }
      }
   }
}

/**
 * @brief Set the mode for display type.
 *
 * 
 * @author Charles "Mickey" Nowell (1/1/2020)
 * 
 * @param mode 
 */
void setMode(uint32_t mode)
{
   uint32_t newMode;
   newMode = mode;
   newMode &= 0x0F;

   ENTER_CRITICAL();
   displayMode.alternate8Scan = displayModes[newMode].alternate8Scan;
   displayMode.do2x5 = displayModes[newMode].do2x5;
   displayMode.doScan16 = displayModes[newMode].doScan16;
   displayMode.doInverted = displayModes[newMode].doInverted;
   currentMode = newMode;
   setupPixelPutFunction();
   EXIT_CRITICAL();
}
/**
 * @brief Cycle through all the display modes for setup.
 *
 * 
 * @author Charles "Mickey" Nowell (1/1/2020)
 */
void incrementMode(void)
{
   uint32_t newMode;
   newMode = currentMode+1;
   newMode &= 0x0F;

   ENTER_CRITICAL();
   displayMode.alternate8Scan = displayModes[newMode].alternate8Scan;
   displayMode.do2x5 = displayModes[newMode].do2x5;
   displayMode.doScan16 = displayModes[newMode].doScan16;
   displayMode.doInverted = displayModes[newMode].doInverted;
   currentMode = newMode;
   setupPixelPutFunction();
   EXIT_CRITICAL();
}

/**
 * @brief Current numbers being displayed
 *
 */
// uint32_t phase,secondsIn,round,heat,flight;
// Used by display scanner that is refreshing the display
uint32_t bank = 0;

/***************************************************************************************/
/***************************************************************************************/
/* Frame Buffer, two overloads depending on scan type */
/***************************************************************************************/
/***************************************************************************************/

/**
 * @brief Note uint16, because only need 12 bits per element,
 *        this is 4 pixels worth. Most efficient use of space is
 *        16 bit words.
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 */
// This is the actual ping-pong frame buffers.
uint16_t  _displayBuffer[FRAMES][ONE_EIGHT_SCAN_ELECTRONIC_ROWS][NUMPIXELS_2x5_ONE_EIGHTH_SCAN];
// Pointers to point the actual blob of ram with different geometric arrangment. Same
// number of elements either way, just treated as differente organization.
uint16_t(*displayBufferOneEighthScan)[ONE_EIGHT_SCAN_ELECTRONIC_ROWS][NUMPIXELS_2x5_ONE_EIGHTH_SCAN] =\
   &(_displayBuffer[0]);
uint16_t(*displayBufferOneSixtenthScan)[ONE_SIXTEENTH_SCAN_ELECTRONIC_ROWS][NUMPIXELS_2x5_ONE_SIXTEENTH_SCAN] =\
   (uint16_t(*)[ONE_SIXTEENTH_SCAN_ELECTRONIC_ROWS][NUMPIXELS_2x5_ONE_SIXTEENTH_SCAN])(&(_displayBuffer[0]));





/**
 * @brief Clear the fram buffer to 0s. Then all pixel writes can
 *        be simple. Only need to OR for a given pixel in a
 *        word. 
 *
 * 
 * @author Charles "Mickey" Nowell (12/11/2019)
 * 
 * @param buffer 
 */
void clearBuffer(uint32_t buffer, uint32_t color)
{
   // note 2 = 2 bytes per word16
   memset(displayBufferOneEighthScan[buffer],color,ONE_EIGHT_SCAN_ELECTRONIC_ROWS * NUMPIXELS_2x5_ONE_EIGHTH_SCAN * 2);
}


/**
 * @brief These two tables make the special offset that index
 *        into a 1/8 pixel buffer so that the x/y pixel is set
 *        in the correct location. See the paper notes on how
 *        this mapping was derived. Or look at the pixel maps
 *        above. 
 *
 * 
 * @author Charles "Mickey" Nowell (12/8/2019)
 */

const uint32_t  x1Delta[] =
{
   0,
   48,
   48 + 16,
   112,
   112 + 16,
   176,
   176 + 16,
   240,
   240 + 16,
   304,
   304
};


const uint32_t  x2Delta[] =
{
   16,
   80,
   144,
   208,
   272
};


// Alternate mapping.
// If x = 0..15 no change
// if x = 16..31 add 16
// so if x & 0x10 add 16 if y is not 8 rows forward

const uint32_t  x3Delta[] =
{

   0 * 64, // 0..15
   0 * 64 + 32, // 16..31
   1 * 64, // 32..47
   1 * 64 + 32, // 48
   2 * 64,
   2 * 64 + 32,
   3 * 64,
   3 * 64 + 32,
   4 * 64,
   4 * 64 + 32,
   5 * 64,
   5 * 64 + 32
};


const uint32_t  x4Delta[] =
{
   0 * 64 + 16, // 0..15
   0 * 64 + 16 + 32, // 16..31
   1 * 64 + 16, // 32..47
   1 * 64 + 16 + 32, // 48
   2 * 64 + 16,
   2 * 64 + 16 + 32,
   3 * 64 + 16,
   3 * 64 + 16 + 32,
   4 * 64 + 16,
   4 * 64 + 16 + 32,
   5 * 64 + 16,
   5 * 64 + 16 + 32

};



/**
 * Macros to map x positions into the correct offsets.
 */
// divide x by 16, get the x1 delta
// then add original x to offset
// These are the odd mapping functions for the old and new 1/8 scan scan orders.
// New 1/2 or the old style, 3/4 are the new style
// They use different lookup tables and masking.

#define NEWX1(x)   (x & 0x0F)+(x1Delta[x>>4])
#define NEWX2(x)   (x & 0x1F)+(x2Delta[x>>5])

#define NEWX3(x)   (x & 0x0F)+(x3Delta[x>>4])
#define NEWX4(x)   (x & 0x0F)+(x4Delta[x>>4])


// Maximum coordinates for big display
static int32_t xMaximum = 159;
static int32_t yMaximum  = 63;
// Maximum coordinates for small display
static int32_t xMaximumSmallDisplay = 127;
static int32_t yMaximumSmallDisplay  = 31;

/**
 * @brief Override function for testing.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param x 
 * @param y 
 */
void putMaximums(int32_t x, int32_t y)
{
   xMaximum = x;
   yMaximum = y;
};


/**
 * @brief Put a pixel in the large display using 1/8 scan
 *        panels. Does all the mapping to set the pixel on at
 *        the given position. Buffer is the ping/pong position
 *        to write to the inactive buffer.
 *
 * 
 * @author Charles "Mickey" Nowell (12/8/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8Alternate(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximum) || (y > yMaximum))
   {
      return;
   }


   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   // y = yMaximum - y;
   // x = xMaximum - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX4(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX3(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
}

/**
 * @brief Draw pixels for dual 1x4 display, altenate scan (new),
 *        1/8 scan. Upright mounting.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8AlternateSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;
   uint32_t savey = y;
   uint32_t savex = x;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximumSmallDisplay) || (y > yMaximumSmallDisplay))
   {
      return;
   }


   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   // y = yMaximum - y;
   // x = xMaximum - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX4(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX3(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);


   // other half of display
   y = yMaximumSmallDisplay - savey;
   x = xMaximumSmallDisplay - savex;
   shift = y >> 4;
   shift += 2;

   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX4(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX3(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);


}

/**
 * @brief Put pixels for 1/8 scan, old scan order, upright
 *        display, large panel
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximum) || (y > yMaximum))
   {
      return;
   }


   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   // y = yMaximum - y;
   // x = xMaximum - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      localx = NEWX2(x);
   }
   else
   {
      localx = NEWX1(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
}
/**
 * @brief Draw pixels for 1/8 scan, dual 1x4 mounting, upright
 *        display, old style scan order.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8SmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;
   uint32_t savey = y;
   uint32_t savex = x;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximumSmallDisplay) || (y > yMaximumSmallDisplay))
   {
      return;
   }


   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   // y = yMaximum - y;
   // x = xMaximum - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      localx = NEWX2(x);
   }
   else
   {
      localx = NEWX1(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
   // other half of display
   y = yMaximumSmallDisplay - savey;
   x = xMaximumSmallDisplay - savex;
   shift = y >> 4;
   shift += 2;

   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX2(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX1(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
}

/**
 * @brief Draw pixels for large display, inverted mounting, 1/8
 *        scan, older scan order. 
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8Inverted(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximum) || (y > yMaximum))
   {
      return;
   }

   // From the discussion
   // So position 0,0 should end up on scan row 7, in position 47

   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // do the sample math on a 32 bit wide display
   // y ends up being 63, 0x3f
   // x = 31  0x1f
   y = yMaximum - y;
   x = xMaximum - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   // sample calculation
   // shift is 3, so the sample pixel ends up on the RGB1 pixel position of the bottom panel
   // this is the correct panel, labelled bottom connector, but physically the top
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   if(y & 0x08)
   {
      // sample x is 31, y and 8 is true
      // shift x is 0, index adds 16 to x, it was 31, now is 47, this is correct.
      localx = NEWX2(x);
   }
   else
   {
      localx = NEWX1(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // sample pixel of 0,0 ends up in scan row 7 (y = 63 & 7 >> = 7), in scan position 47
   // for a 32 bit wide display this is the 47 pixel out on the last row.
   // So this checks out
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
}

/**
 * @brief Draw pixels, 1/8 scan, inverted dual 1x4 arrangement
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8InvertedSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;
   uint32_t savey = y;
   uint32_t savex = x;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximumSmallDisplay) || (y > yMaximumSmallDisplay))
   {
      return;
   }

   // From the discussion
   // So position 0,0 should end up on scan row 7, in position 47

   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // do the sample math on a 32 bit wide display
   // y ends up being 63, 0x3f
   // x = 31  0x1f
   y = yMaximumSmallDisplay - y;
   x = xMaximumSmallDisplay - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   // sample calculation
   // shift is 3, so the sample pixel ends up on the RGB1 pixel position of the bottom panel
   // this is the correct panel, labelled bottom connector, but physically the top
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   if(y & 0x08)
   {
      // sample x is 31, y and 8 is true
      // shift x is 0, index adds 16 to x, it was 31, now is 47, this is correct.
      localx = NEWX2(x);
   }
   else
   {
      localx = NEWX1(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // sample pixel of 0,0 ends up in scan row 7 (y = 63 & 7 >> = 7), in scan position 47
   // for a 32 bit wide display this is the 47 pixel out on the last row.
   // So this checks out
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
   // now write the other small display
   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   y = savey;
   x = savex;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4; 
   // add 2 to shift to use the upper display
   shift += 2;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX4(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX3(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
}

/**
 * @brief Draw pixels on large display, 1/8 scan alternate scan
 *        pattern, inverted mounting.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8AlternateInverted(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximum) || (y > yMaximum))
   {
      return;
   }


   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   y = yMaximum - y;
   x = xMaximum - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX4(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX3(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);
}

/**
 * @brief Draw pixels, 1/8 scan small display (2 of 1x4
 *        arrangement), with panels mounted inverted.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan8AlternateInvertedSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t localx; // , localy;
   uint32_t savey = y;
   uint32_t savex = x;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximumSmallDisplay) || (y > yMaximumSmallDisplay))
   {
      return;
   }


   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   y = yMaximumSmallDisplay - y;
   x = xMaximumSmallDisplay - x;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX4(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX3(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);

   // now write the other small display
   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // Not needed for upright display.
   y = savey;
   x = savex;

   // so 64 rows divided by 16 determines 0..3, or which pixel in
   // the 12 bits we are operating on.
   // Also works for a half high display with 1x4 arrangement.
   shift = y >> 4; 
   // add 2 to shift to use the upper display
   shift += 2;
   // figure out the odd tweak to X based on whether we are not on the prime row
   // or the alternate row , physical row that is, for a given scan row of pixels.
   // for the upright version this is backwards from the prototype
   if(y & 0x08)
   {
      //localx = NEWX2(x);
      localx = NEWX4(x);
   }
   else
   {
      // localx = NEWX1(x);
      localx = NEWX3(x);
   }
   // now turn on the pixel, y & 7 picks one of the 8 scan rows
   // x is the position in the scan row
   // shift moves the pixel to row 16 + y, 32+y or 48+y physically respectively.
   // also upright the shift is the other direction, that is the
   // 0th pixel ends up in the upper bits of the word???
   /**
    * @todo figure out if the shift needs to be << (12 - shift*3) 
    *       or not.
    */
   displayBufferOneEighthScan[buffer][y & 7][localx] |= color << (shift * 3);



}

/**
 * @brief Draw pixels on large display, 1/16 scan, upright
 *        mounting
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan16(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t indexX,indexY;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximum) || (y > yMaximum))
   {
      return;
   }
   // y = 0..15 0 pixels shifted
   // y = 16..23 = 10001 should be 3 pixels up
   // y = 24..31  = 6 pixels
   // y = 9 pixels shifted
   // apparently geometry differences
   // panels upside down, driven from left hand end
   // Row 32 maps to row 31 on the bottom panel

   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // example put in 32 for row, 63 -32 = 31
   // 0x1F this needs to map to row 15, i.e. y&0xf
   // and the pixel is shifted up
   // Not needed for normal orientation.
   // y = yMaximum - y;
   // x = xMaximum - x;
   shift = y >> 4;
   indexX = x;
   indexY = y & 0x0f;
   displayBufferOneSixtenthScan[buffer][indexY][indexX] |= color << (shift * 3);
}
/**
 * @brief Draw pixels for 1/16 scan small display 1x4
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan16SmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t indexX,indexY;
   uint32_t savey = y;
   uint32_t savex = x;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximum) || (y > yMaximum))
   {
      return;
   }
   // y = 0..15 0 pixels shifted
   // y = 16..23 = 10001 should be 3 pixels up
   // y = 24..31  = 6 pixels
   // y = 9 pixels shifted
   // apparently geometry differences
   // panels upside down, driven from left hand end
   // Row 32 maps to row 31 on the bottom panel

   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // example put in 32 for row, 63 -32 = 31
   // 0x1F this needs to map to row 15, i.e. y&0xf
   // and the pixel is shifted up
   // Not needed for normal orientation.
   // y = yMaximum - y;
   // x = xMaximum - x;
   shift = y >> 4;
   indexX = x;
   indexY = y & 0x0f;
   displayBufferOneSixtenthScan[buffer][indexY][indexX] |= color << (shift * 3);

   y = yMaximumSmallDisplay - savey;
   x = xMaximumSmallDisplay - savex;

   shift = y >> 4;
   shift += 2;
   indexX = x;
   indexY = y & 0x0f;
   displayBufferOneSixtenthScan[buffer][indexY][indexX] |= color << (shift * 3);


}



/**
 * @brief Put a pixel in the designated display frame. Use
 *        scan16 mapping. Inverted display mounting.
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan16Inverted(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t indexX,indexY;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximum) || (y > yMaximum))
   {
      return;
   }
   // y = 0..15 0 pixels shifted
   // y = 16..23 = 10001 should be 3 pixels up
   // y = 24..31  = 6 pixels
   // y = 9 pixels shifted
   // apparently geometry differences
   // panels upside down, driven from left hand end
   // Row 32 maps to row 31 on the bottom panel

   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // example put in 32 for row, 63 -32 = 31
   // 0x1F this needs to map to row 15, i.e. y&0xf
   // and the pixel is shifted up
   y = yMaximum - y;
   x = xMaximum - x;
   shift = y >> 4;
   indexX = x;
   indexY = y & 0x0f;
   displayBufferOneSixtenthScan[buffer][indexY][indexX] |= color << (shift * 3);
}

/**
 * @brief This function puts pixels into the display buffer
 *        using the 1/16 scan function with the small display.
 *        The display is mounted inverted so that the controller
 *        board is mounted on the other end of the panel and the
 *        LED panels are upside down and backwards.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixelScan16InvertedSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   uint32_t shift;
   uint32_t indexX,indexY;
   uint32_t savey = y;
   uint32_t savex = x;

   if((x < 0) || (y < 0))
   {
      return;
   }
   if((x > xMaximumSmallDisplay) || (y > yMaximumSmallDisplay))
   {
      return;
   }
   // y = 0..15 0 pixels shifted
   // y = 16..23 = 10001 should be 3 pixels up
   // y = 24..31  = 6 pixels
   // y = 9 pixels shifted
   // apparently geometry differences
   // panels upside down, driven from left hand end
   // Row 32 maps to row 31 on the bottom panel

   // This gets top to bottom and left to right sorted out
   // Coordinates are 0,0, top left corner.
   // example put in 32 for row, 63 -32 = 31
   // 0x1F this needs to map to row 15, i.e. y&0xf
   // and the pixel is shifted up
   y = yMaximumSmallDisplay - y;
   x = xMaximumSmallDisplay - x;
   shift = y >> 4;
   indexX = x;
   indexY = y & 0x0f;
   displayBufferOneSixtenthScan[buffer][indexY][indexX] |= color << (shift * 3);
   // draw other half
   y = savey;
   x = savex;
   shift = y >> 4;
   shift +=2;
   indexX = x;
   indexY = y & 0x0f;
   displayBufferOneSixtenthScan[buffer][indexY][indexX] |= color << (shift * 3);
}



/**
 * @brief Put a pixel in the display buffer. Hides the scan type
 *        and whether or not the display is inverted or not.
 *
 * 
 * @author Charles "Mickey" Nowell (12/16/2019)
 * 
 * @param buffer 
 * @param y 
 * @param x 
 * @param color 
 */
void putPixel(uint32_t buffer, int32_t y, int32_t x, uint32_t color)
{
   if(putPixelPointer)
   {
      (*putPixelPointer)(buffer,y,x,color);
   }
}

// Macros for making sure that the GPIOs have been written
// now just does the sync business with the bus and cpu
#define BANKNOP()  killCLOCKTs(3)
#define CLOCKNOP() __DSB()


#define BANKNOP()  killCLOCKTs(3)
#define CLOCKNOP() __DSB()  // makes sure GPIO's are finished


// Macro-ize the delay between clock pulses on the display panel drive.
// #define NOP()  killCLOCKTs(1);__DSB()
#define NOP()  __DSB()


/**
 * @brief Drive pixels out to the display in a 2x5, i.e. large
 *        display form. This now tested and working.
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 * 
 * @param pixel 
 */
void drivePixelsOut2x5(uint32_t pixel)
{

   // COLOR(PIXEL(a,b,c,d));
   COLOR(pixel);
   CLOCKNOP();
   LOWGPIO(CLOCKT);
   LOWGPIO(CLOCKB);
   CLOCKNOP();
   HIGHGPIO(CLOCKT);
   HIGHGPIO(CLOCKB);
   CLOCKNOP();
   LOWGPIO(CLOCKT);
   LOWGPIO(CLOCKB);
   CLOCKNOP();
}


/**
 * @brief Generic row outputter. Puts one scan row (electronic
 *        row) on on the display.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param columns 
 * @param pixelRow 
 * @param bankMask 
 */
void outputRow(uint32_t columns,uint16_t   *pixelRow,uint32_t bankMask)
{
   int i;
   // Clear the bank
   BANK(0);
   NOP();
   // Preset the latches
   LOWGPIO(LATCHT);
   LOWGPIO(LATCHB);
   NOP();
   // clear the OE
   HIGHGPIO(OET);
   HIGHGPIO(OEB);
   // Preset the clock
   NOP();
   LOWGPIO(CLOCKT);
   LOWGPIO(CLOCKB);
   NOP();

   // Now drive out all the pixels
   for(i = 0;i < columns;i++)
   {
      drivePixelsOut2x5(*pixelRow++); 
   }
   NOP();
   NOP();
   // Latch the data
   HIGHGPIO(LATCHT);
   HIGHGPIO(LATCHB);
   NOP();
   NOP();
   NOP();
   LOWGPIO(LATCHT);
   LOWGPIO(LATCHB);
   NOP();
   // Enable the OE so the pixels come on
   LOWGPIO(OET);
   LOWGPIO(OEB);
   // Drive the correct bank with the pixels
   BANK(bank++);
   // Move to next bank for next time
   bank &= bankMask; 
}



/**
 * @brief Setup all the GPIOs needed to drive the panels.
 *
 * 
 * @author Charles "Mickey" Nowell (12/20/2019)
 * 
 * @param void 
 */
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
// Variable that shows the current display mode
extern volatile uint32_t currentMode;

/**
 * @brief This is the task that scans the pixels out to the
 *        display. It adapts to the current mode, basically it
 *        just changes the number of pixels shifted out every
 *        time. The buffer is corrected to have the pixels
 *        rearranged to fit the current panel type and
 *        arrangement.
 *
 * 
 * @author Charles "Mickey" Nowell (12/30/2019)
 * 
 * @param argument 
 */
void scannerTask(void const *argument)
{
   uint32_t modeValue;
   initGPIO();
   LOWGPIO(OET);
   LOWGPIO(OEB);
   /* Infinite loop */
   modeValue = currentMode;
   while(1)
   {
      if(displayMode.do2x5)
      {
         if(displayMode.doScan16)
         {
            while(1)
            {
               RTOS_MSEC_DELAY(1);
               outputRow(NUMPIXELS_2x5_ONE_SIXTEENTH_SCAN,&(displayBufferOneSixtenthScan[showFrame][bank][0]),0x0F);
               if(modeValue != currentMode)
               {
                  modeValue = currentMode;
                  break;
               }
            }
         }
         else
         {
            while(1)
            {
               RTOS_MSEC_DELAY(1);
               outputRow(NUMPIXELS_2x5_ONE_EIGHTH_SCAN,&(displayBufferOneEighthScan[showFrame][bank][0]),0x07);
               // outputRow2x5OneEigthScan(); // 0,5,NUMPIXELS_2x5_ONE_EIGHTH_SCAN);
               if(modeValue != currentMode)
               {
                  modeValue = currentMode;
                  break;
               }
            }
         }
      }
      else
      {
         // 1x4 panel
         if(displayMode.doScan16)
         {
            while(1)
            {
               RTOS_MSEC_DELAY(1);
               outputRow(NUMPIXELS_1x4_ONE_SIXTEENTH_SCAN,&(displayBufferOneSixtenthScan[showFrame][bank][0]),0x0F);
               if(modeValue != currentMode)
               {
                  modeValue = currentMode;
                  break;
               }
            }
         }
         else
         {
            while(1)
            {
               RTOS_MSEC_DELAY(1);
               //outputRow(NUMPIXELS_1x4_ONE_EIGTH_SCAN,&(displayBufferOneEighthScan[showFrame][bank][0]),0x07);
               outputRow(NUMPIXELS_1x4_ONE_EIGHTH_SCAN,&(displayBufferOneEighthScan[showFrame][bank][0]),0x07);
               if(modeValue != currentMode)
               {
                  modeValue = currentMode;
                  break;
               }
            }
         }
      }
   }
}

