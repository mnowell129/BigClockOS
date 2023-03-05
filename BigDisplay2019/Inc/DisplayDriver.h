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
#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

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


extern osThreadId scannerTaskHandle;


typedef struct DisplayModeType
{
   bool       doScan16;
   bool       alternate8Scan;
   bool       doInverted;
   bool       do2x5;
   uint16_t   crc;
}DisplayModeType;

extern volatile DisplayModeType displayMode;
extern volatile uint32_t currentMode;

/***************************************************************************************/
/***************************************************************************************/
/* Module data */
/***************************************************************************************/
/***************************************************************************************/

extern volatile int32_t seconds;
extern volatile int32_t minutes;
// Experiment with sliding numbers
extern volatile int32_t _milliseconds;
extern volatile int32_t milliseconds;

typedef void (*PutPixelFunction)(uint32_t buffer, int32_t y, int32_t x, uint32_t color);

void setupColors(void);
void setupAlternateColors(void);
void setupPixelPutFunction(void);
void setMode(uint32_t mode);
void incrementMode(void);
void clearBuffer(uint32_t buffer, uint32_t color);
void putMaximums(int32_t x, int32_t y);
void putPixelScan8Alternate(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan8AlternateSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan8(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScanTest(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan8SmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan8Inverted(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan8InvertedSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan8AlternateInverted(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan8AlternateInvertedSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan16(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan16SmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan16Inverted(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixelScan16InvertedSmallDisplay(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void putPixel(uint32_t buffer, int32_t y, int32_t x, uint32_t color);
void drivePixelsOut2x5(uint32_t pixel);
void outputRow(uint32_t columns,uint16_t   *pixelRow,uint32_t bankMask);
void initGPIO(void);
void scannerTask(void const *argument);


#endif
