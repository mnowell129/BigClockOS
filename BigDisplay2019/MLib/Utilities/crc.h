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
#ifndef CRC_H
#define CRC_H

#define NOMINAL_CRC_SEED  0xFFFF

Word16 computeCRC(Byte *buffer,int32_t length,Word16 startingValue,Byte doLSB);
Word16 calcCRC(Byte *buffer, int32_t length);
Word16 calcCRCUnlimited(Byte *buffer, int32_t length);
void addCRC16(Byte *buffer,int32_t length);
Word16 runningCRC(Byte *buffer,int32_t length, Word16 partialCRC);
extern const Byte flip[];
Word16 calcCRCLSB(Byte *buffer, int32_t length);

#endif

