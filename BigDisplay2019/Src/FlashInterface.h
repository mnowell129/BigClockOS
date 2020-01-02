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
#ifndef FLASHINTERFACE_H
#define FLASHINTERFACE_H


#define PARAMETER_SECTOR  FLASH_SECTOR_1
#define PARAMETER_ADDRESS 0x08004000
bool flashUnlock(void);
bool sectorErase(uint32_t sectorNumber);
bool flashWrite(uint32_t address, uint32_t data);
void eraseParameters(void);
bool writeFlash(uint8_t *data,uint32_t length);

#endif
