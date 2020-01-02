
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

