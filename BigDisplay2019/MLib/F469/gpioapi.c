#include "type.h"
#include "rtos.h"

#include "stm32F4xx.h"
#include "stm32F446deviceall.h"


#include "gpioapi.h"
#include "IOConfiguration.h"

static   LL_GPIO_InitTypeDef sGPIOInitStructure;

void initGPIOPin(GPIO_TypeDef *group, uint32_t mode, uint32_t pin,
                 uint32_t otype, uint32_t pupd, uint32_t speed,
                 uint32_t defaultValue)
{
   // write default value ahead of time
   GPIO_WriteBit(group,pin,defaultValue);
   sGPIOInitStructure.Mode  = mode;
   sGPIOInitStructure.Pin   = pin;
   sGPIOInitStructure.OutputType = otype;
   sGPIOInitStructure.Speed = speed;
   sGPIOInitStructure.Pull = pupd;
   LL_GPIO_Init(group,&sGPIOInitStructure);
   GPIO_WriteBit(group,pin,defaultValue);
}

void initAFPin(GPIO_TypeDef *group, uint32_t pin,
               uint32_t otype, uint32_t pupd, uint32_t speed,
               uint32_t af)
{
   sGPIOInitStructure.Mode  = LL_GPIO_MODE_ALTERNATE;
   sGPIOInitStructure.Pin   = pin;
   sGPIOInitStructure.OutputType = otype;
   sGPIOInitStructure.Speed = speed;
   sGPIOInitStructure.Pull = pupd;
   sGPIOInitStructure.Alternate = af;

   if(pin & 0xFFFF)
   {
      if(pin & 0xFF)
      {
         LL_GPIO_SetAFPin_0_7(group,pin,af);
      }
      else if(pin & 0xFF00)
      {
         LL_GPIO_SetAFPin_8_15(group,pin,af);
      }
   }


   LL_GPIO_Init(group,&sGPIOInitStructure);

}

void initANPin(GPIO_TypeDef *group, uint32_t pin, 
               uint32_t otype, uint32_t pupd, uint32_t speed)
{
   sGPIOInitStructure.Mode  = LL_GPIO_MODE_ANALOG;
   sGPIOInitStructure.Pin   = pin;
   sGPIOInitStructure.OutputType = otype;
   sGPIOInitStructure.Speed = speed;
   sGPIOInitStructure.Pull = pupd;
   LL_GPIO_Init(group,&sGPIOInitStructure);
}

void GPIO_WriteBit(GPIO_TypeDef *group, uint32_t pin, uint32_t defaultValue)
{
   if(defaultValue)
   {
      LL_GPIO_SetOutputPin(group,pin);
   }
   else
   {
      LL_GPIO_ResetOutputPin(group,pin);
   }
}

