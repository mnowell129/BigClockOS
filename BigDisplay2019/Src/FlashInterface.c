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
#include "type.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "flashinterface.h"

// ADDR_FLASH_SECTOR_11
// 
// FLASH_SECTOR_1
// 0x08004000 // address
// sectorSize = 16 * 1024



static FLASH_EraseInitTypeDef EraseInitStruct;


bool flashUnlock(void)                 
{
   if((FLASH->CR & FLASH_CR_LOCK) != RESET)
   {
      /* Authorize the FLASH Registers access */
      FLASH->KEYR = FLASH_KEY1;
      FLASH->KEYR = FLASH_KEY2;
   }
   else
   {
      return(false);
   }

   return(true);
}


bool sectorErase(uint32_t sectorNumber)
{
   uint32_t SECTORError;
   // static so it will be all 0's
   static FLASH_OBProgramInitTypeDef options;
   HAL_FLASH_Unlock();

   /* Fill EraseInit structure*/
   EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
   EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_1; // was 3
   EraseInitStruct.Sector        = sectorNumber;
   EraseInitStruct.NbSectors     = 1;


   options.OptionType = OPTIONBYTE_WRP;
   options.Banks = 1;
   options.WRPSector = sectorNumber;
   options.WRPState = OB_WRPSTATE_DISABLE;
   HAL_FLASHEx_OBProgram(&options);
   /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
      you have to make sure that these data are rewritten before they are accessed during code
      execution. If this cannot be done safely, it is recommended to flush the caches by setting the
      DCRST and ICRST bits in the FLASH_CR register. */
   if(HAL_FLASHEx_Erase(&EraseInitStruct,&SECTORError) != HAL_OK)
   {
      /*
        Error occurred while sector erase.
        User can add here some code to deal with this error.
        SECTORError will contain the faulty sector and then to know the code error on this sector,
        user can call function 'HAL_FLASH_GetError()'
      */
      /* Infinite loop */
      HAL_FLASH_Lock();
      return(false);
   }
   HAL_FLASH_Lock();
   return(true);
}

bool flashWrite(uint32_t address, uint32_t data)
{
   HAL_FLASH_Unlock();
   if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,address,data) == HAL_OK)
   {
      HAL_FLASH_Lock();
      return(true);
   }
   else
   {
      HAL_FLASH_Lock();
      return(false);
   }
}

static uint8_t *currentWriteAddress;

void eraseParameters(void)
{
   sectorErase(PARAMETER_SECTOR);
   currentWriteAddress = (uint8_t *)PARAMETER_ADDRESS;
}

bool writeFlash(uint8_t *data,uint32_t length)
{
   HAL_FLASH_Unlock();
   while(length--)
   {
      if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,(uint32_t)(currentWriteAddress),*data++) != HAL_OK)
      {
         HAL_FLASH_Lock();
         return(false);
      }
      currentWriteAddress++;
   }
   HAL_FLASH_Lock();
   return(true);
}

