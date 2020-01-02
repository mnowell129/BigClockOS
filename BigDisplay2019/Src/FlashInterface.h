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
