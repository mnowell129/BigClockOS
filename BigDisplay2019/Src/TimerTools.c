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
#include <intrinsics.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#include "rtos.h"
#include "type.h"
#include "ascii.h"

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_tim.h"

#define DO_DECLARE
#include "timertools.h"
#undef DO_DECLARE

#define TTOOLSTIMERNUMBER   11
//#else
//#error Need to define a timer for timer tools
//#endif
#define PREFIX(a)        a

#define CONCAT(a,b)      a##b
#define _TTOOLSTIMER(a,b)   CONCAT(a,b)
#define TTOOLSTIMER         _TTOOLSTIMER(TIM,PREFIX(TTOOLSTIMERNUMBER))

// TIM4_IRQn
#define CONSTRUCT(a,b,c) a##b##c
#define _THEIRQ(a,b)     CONSTRUCT(a,b,_IRQn)

// For function name and irq number
#if (TTOOLSTIMERNUMBER == 11)
// ST can't manage to come up with one interrupt for each timer
#define THEIRQ          TIM1_TRG_COM_TIM11_IRQn
#define CONVERT(a) void TIM1_TRG_COM_TIM##a##_IRQHandler(void)
#else
#define THEIRQ          _THEIRQ(TIM,PREFIX(TTOOLSTIMERNUMBER))
#define CONVERT(a) void TIM##a##_IRQHandler(void)
#endif
#define IRQFUNC(a) CONVERT(a)
#define IRQNAME    PREFIX(TTOOLSTIMERNUMBER)

///*************************************************************
///*************************************************************

//typedef void (*TimerClockEnableFunction)(uint32_t RCC_APB1Periph, FunctionalState NewState);
typedef void (*TimerClockEnableFunction)(uint32_t RCC_APB1Periph);
typedef struct
{
   TimerClockEnableFunction  functionToCall;
   uint32_t                  enableValue;
}ClockEnableData;

///*************************************************************
///*************************************************************

#define MAX_TIMER 14

///*************************************************************
///*************************************************************
const ClockEnableData  clockEnableData[MAX_TIMER+1] =  // use a dummy 0
{
   // 1,8,9,10,11 on domain 2
   [ 0].functionToCall = LL_APB2_GRP1_EnableClock,   [ 0].enableValue = LL_APB2_GRP1_PERIPH_TIM1, // RCC_APB2Periph_TIM1,
   [ 1].functionToCall = LL_APB2_GRP1_EnableClock,   [ 1].enableValue = LL_APB2_GRP1_PERIPH_TIM1, // RCC_APB2Periph_TIM1,
   [ 2].functionToCall = LL_APB1_GRP1_EnableClock,   [ 2].enableValue = LL_APB1_GRP1_PERIPH_TIM2, //RCC_APB1Periph_TIM2,
   [ 3].functionToCall = LL_APB1_GRP1_EnableClock,   [ 3].enableValue = LL_APB1_GRP1_PERIPH_TIM3, // RCC_APB1Periph_TIM3,
   [ 4].functionToCall = LL_APB1_GRP1_EnableClock,   [ 4].enableValue = LL_APB1_GRP1_PERIPH_TIM4, // RCC_APB1Periph_TIM4,
   [ 5].functionToCall = LL_APB1_GRP1_EnableClock,   [ 5].enableValue = LL_APB1_GRP1_PERIPH_TIM5, // RCC_APB1Periph_TIM5,
   [ 6].functionToCall = LL_APB1_GRP1_EnableClock,   [ 6].enableValue = LL_APB1_GRP1_PERIPH_TIM6, // RCC_APB1Periph_TIM6,
   [ 7].functionToCall = LL_APB1_GRP1_EnableClock,   [ 7].enableValue = LL_APB1_GRP1_PERIPH_TIM7, // RCC_APB1Periph_TIM7,
   [ 8].functionToCall = LL_APB2_GRP1_EnableClock,   [ 8].enableValue = LL_APB2_GRP1_PERIPH_TIM8, // RCC_APB2Periph_TIM8,
   [ 9].functionToCall = LL_APB2_GRP1_EnableClock,   [ 9].enableValue = LL_APB2_GRP1_PERIPH_TIM9, // RCC_APB2Periph_TIM9,
   [10].functionToCall = LL_APB2_GRP1_EnableClock,   [10].enableValue = LL_APB2_GRP1_PERIPH_TIM10, // RCC_APB2Periph_TIM10,
   [11].functionToCall = LL_APB2_GRP1_EnableClock,   [11].enableValue = LL_APB2_GRP1_PERIPH_TIM11, // RCC_APB2Periph_TIM11,
   [12].functionToCall = LL_APB1_GRP1_EnableClock,   [12].enableValue = LL_APB1_GRP1_PERIPH_TIM12, // RCC_APB1Periph_TIM12,
   [13].functionToCall = LL_APB1_GRP1_EnableClock,   [13].enableValue = LL_APB1_GRP1_PERIPH_TIM13, // RCC_APB1Periph_TIM13,
   [14].functionToCall = LL_APB1_GRP1_EnableClock,   [14].enableValue = LL_APB1_GRP1_PERIPH_TIM14, // RCC_APB1Periph_TIM14
};

///*************************************************************
///*************************************************************
const ClockEnableData  clockDisableData[MAX_TIMER+1] =  // use a dummy 0
{
   // 1,8,9,10,11 on domain 2
   [ 0].functionToCall = LL_APB2_GRP1_DisableClock,   [ 0].enableValue = LL_APB2_GRP1_PERIPH_TIM1, // RCC_APB2Periph_TIM1,
   [ 1].functionToCall = LL_APB2_GRP1_DisableClock,   [ 1].enableValue = LL_APB2_GRP1_PERIPH_TIM1, // RCC_APB2Periph_TIM1,
   [ 2].functionToCall = LL_APB1_GRP1_DisableClock,   [ 2].enableValue = LL_APB1_GRP1_PERIPH_TIM2, //RCC_APB1Periph_TIM2,
   [ 3].functionToCall = LL_APB1_GRP1_DisableClock,   [ 3].enableValue = LL_APB1_GRP1_PERIPH_TIM3, // RCC_APB1Periph_TIM3,
   [ 4].functionToCall = LL_APB1_GRP1_DisableClock,   [ 4].enableValue = LL_APB1_GRP1_PERIPH_TIM4, // RCC_APB1Periph_TIM4,
   [ 5].functionToCall = LL_APB1_GRP1_DisableClock,   [ 5].enableValue = LL_APB1_GRP1_PERIPH_TIM5, // RCC_APB1Periph_TIM5,
   [ 6].functionToCall = LL_APB1_GRP1_DisableClock,   [ 6].enableValue = LL_APB1_GRP1_PERIPH_TIM6, // RCC_APB1Periph_TIM6,
   [ 7].functionToCall = LL_APB1_GRP1_DisableClock,   [ 7].enableValue = LL_APB1_GRP1_PERIPH_TIM7, // RCC_APB1Periph_TIM7,
   [ 8].functionToCall = LL_APB2_GRP1_DisableClock,   [ 8].enableValue = LL_APB2_GRP1_PERIPH_TIM8, // RCC_APB2Periph_TIM8,
   [ 9].functionToCall = LL_APB2_GRP1_DisableClock,   [ 9].enableValue = LL_APB2_GRP1_PERIPH_TIM9, // RCC_APB2Periph_TIM9,
   [10].functionToCall = LL_APB2_GRP1_DisableClock,   [10].enableValue = LL_APB2_GRP1_PERIPH_TIM10, // RCC_APB2Periph_TIM10,
   [11].functionToCall = LL_APB2_GRP1_DisableClock,   [11].enableValue = LL_APB2_GRP1_PERIPH_TIM11, // RCC_APB2Periph_TIM11,
   [12].functionToCall = LL_APB1_GRP1_DisableClock,   [12].enableValue = LL_APB1_GRP1_PERIPH_TIM12, // RCC_APB1Periph_TIM12,
   [13].functionToCall = LL_APB1_GRP1_DisableClock,   [13].enableValue = LL_APB1_GRP1_PERIPH_TIM13, // RCC_APB1Periph_TIM13,
   [14].functionToCall = LL_APB1_GRP1_DisableClock,   [14].enableValue = LL_APB1_GRP1_PERIPH_TIM14, // RCC_APB1Periph_TIM14
};

///*************************************************************
///*************************************************************

/**
 * @brief enableTimerClock 
 * Enable timer clock 
 * note: 1 based 
 * 
 * @author nowelc2 (1/24/2017)
 * 
 * @param timerNumber 1..14
 */
void enableTimerClock(uint32_t timerNumber)
{
   if(timerNumber < 1)
   {
      return;
   }
   if(timerNumber > MAX_TIMER)
   {
      return;
   }
   (*(clockEnableData[timerNumber].functionToCall))(clockEnableData[timerNumber].enableValue);
}

/**
 * @brief disableTimerClock 
 * Disable timer clock on the correct domain
 * 
 * @author nowelc2 (1/24/2017)
 * 
 * @param timerNumber 
 */
void disableTimerClock(uint32_t timerNumber)
{
   if(timerNumber < 1)
   {
      return;
   }
   if(timerNumber > MAX_TIMER)
   {
      return;
   }
   (*(clockDisableData[timerNumber].functionToCall))(clockEnableData[timerNumber].enableValue);
}

///*************************************************************
///*************************************************************
// From RCC_CFGR register docs
static const uint8_t prescalerTable[8] = {1,1,1,1,2,4,8,16};

///*************************************************************
///*************************************************************

/**
 * @brief getTimerClockFrequency 
 * 
 * 
 * @author nowelc2 (1/24/2017)
 * 
 * @param TIMx 
 * 
 * @return uint32_t 
 */
uint32_t getTimerClockFrequency(TIM_TypeDef *TIMx)
{
   uint32_t  frequency;
   uint32_t apb1Divisor,apb2Divisor;
   LL_RCC_ClocksTypeDef                       rccClocksTelN; //RCC_ClocksTypeDef                       rccClocksTelN;
   /* Check the parameters */
   // assert_param(IS_TIM_ALL_PERIPH(TIMx));

   LL_RCC_GetSystemClocksFreq(&rccClocksTelN);       // RCC_GetClocksFreq(&rccClocksTelN);


   /* Get PCLK1 prescaler */
   apb1Divisor = prescalerTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> 10];
   apb2Divisor = prescalerTable[(RCC->CFGR & RCC_CFGR_PPRE2) >> 13];
   // Timer 1 and timer 8, 9,10,11 are domain APB2, i.e. pclk2
   // Timers 2,3,4,5,6,7,12,13 ,14 are APB1

   if((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM9) || (TIMx == TIM10)
      || (TIMx == TIM11))
   {
      // apb2 domain
      frequency = rccClocksTelN.PCLK2_Frequency;
      if(apb2Divisor != 1)
      {
         frequency *= 2;
      }
   }
   else
   {
      // apb1 domain
      frequency = rccClocksTelN.PCLK1_Frequency;
      if(apb1Divisor != 1)
      {
         frequency *= 2;
      }
   }
   return(frequency);
}

///*************************************************************
///*************************************************************
//static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//static TIM_OCInitTypeDef  TIM_OCInitStructure;
// static NVIC_InitTypeDef NVIC_InitStructure;
static LL_TIM_InitTypeDef     TIM_TimeBaseStructure;
static LL_TIM_OC_InitTypeDef  TIM_OCInitStructure;

///*************************************************************
///*************************************************************

/**
 * @brief irq handler for timer TTOOLSTIMERNUMBER
 * 
 * @author byk1 (6/15/2018)
 * 
 * @param IRQNAME 
 */
IRQFUNC(IRQNAME)
{
   ISRCONTEXT;

   // clear the irq
   // if(TIM_GetITStatus(TTOOLSTIMER,TIM_IT_CC1) != RESET)
   // old function looked for flag and enable
   if(LL_TIM_IsActiveFlag_CC1(TTOOLSTIMER) && LL_TIM_IsEnabledIT_CC1(TTOOLSTIMER))
   {
      /* Clear TTOOLSTIMER Capture Compare1 interrupt pending bit*/
      // TIM_ClearITPendingBit(TTOOLSTIMER,TIM_IT_CC1);
      LL_TIM_ClearFlag_CC1(TTOOLSTIMER);
   }
   // stop the timer
   // TIM_Cmd(TTOOLSTIMER,DISABLE);
   LL_TIM_DisableCounter(TTOOLSTIMER);
   ISR_SEMA_PUT(timerToolsCompleteSema);
   ISR_EXIT();
}


/**
 * @brief setupTimer 
 * Do the initial timer setup, 1 Usec ticks, not running
 * 
 * @author nowelc2 (1/24/2017)
 */
void setupTimer(void)
{
   uint32_t timerFrequency;

   enableTimerClock(TTOOLSTIMERNUMBER);

   TIM_TimeBaseStructure.Autoreload = 65535u;                       // .TIM_Period = 65535u;
   TIM_TimeBaseStructure.Prescaler = 1;                             // .TIM_Prescaler = 1u;
   TIM_TimeBaseStructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1; // .TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.CounterMode = LL_TIM_COUNTERMODE_UP;       // .TIM_CounterMode = TIM_CounterMode_Up;

   LL_TIM_Init(TTOOLSTIMER, &TIM_TimeBaseStructure);  // TIM_TimeBaseInit(TTOOLSTIMER,&TIM_TimeBaseStructure);

   // Prescaler configuration
   timerFrequency = getTimerClockFrequency(TTOOLSTIMER);

   LL_TIM_SetPrescaler(TTOOLSTIMER, timerFrequency / ONE_MILLION); // TIM_PrescalerConfig(TTOOLSTIMER,timerFrequency / ONE_MILLION,TIM_PSCReloadMode_Immediate);
   LL_TIM_GenerateEvent_UPDATE(TTOOLSTIMER);

   /* Output Compare Active Mode configuration: Channel1 */

   TIM_OCInitStructure.OCMode = LL_TIM_OCMODE_INACTIVE;     // .TIM_OCMode = TIM_OCMode_Inactive;
   TIM_OCInitStructure.OCState = LL_TIM_OCIDLESTATE_HIGH;   // .TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStructure.CompareValue = 0;                    // .TIM_Pulse = 0;
   TIM_OCInitStructure.OCPolarity = LL_TIM_OCPOLARITY_HIGH; // .TIM_OCPolarity = TIM_OCPolarity_High;

   LL_TIM_OC_Init(TTOOLSTIMER,LL_TIM_CHANNEL_CH1, &TIM_OCInitStructure); //        TIM_OC1Init(THETIMER,&TIM_OCInitStructure);

   LL_TIM_OC_DisablePreload(TTOOLSTIMER,LL_TIM_CHANNEL_CH1); // TIM_OC1PreloadConfig(TTOOLSTIMER,TIM_OCPreload_Disable);
   /* TIM IT enable */
   LL_TIM_EnableIT_CC1(TTOOLSTIMER);                         // TIM_ITConfig(TTOOLSTIMER,TIM_IT_CC1,ENABLE);

   // NVIC_InitStructure.NVIC_IRQChannel = THEIRQ;
   // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
   // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   // NVIC_Init(&NVIC_InitStructure);
   NVIC_SetPriority(THEIRQ,5);
   NVIC_EnableIRQ(THEIRQ);
}

/**
 * @brief initialize timer tools
 * 
 * @author byk1 (6/15/2018)
 * 
 * @param void 
 */
void initTimerTools(void)
{
   SEMA_CREATE_BINARY_STATIC(timerToolsMutexSema);
   SEMA_CREATE_BINARY_STATIC_NO_GIVE(timerToolsCompleteSema);
   setupTimer();
}

/**
 * @brief start timer
 * 
 * @author byk1 (6/15/2018)
 * 
 * @param delayInMicroSeconds 
 */
void startTimer(uint32_t delayInMicroSeconds)
{
   // TIM_OCInitStructure.TIM_Pulse = delayInMicroSeconds;
   TIM_OCInitStructure.CompareValue = delayInMicroSeconds;                         
   // TIM_OC1Init(TTOOLSTIMER,&TIM_OCInitStructure);
   LL_TIM_OC_Init(TTOOLSTIMER,LL_TIM_CHANNEL_CH1, &TIM_OCInitStructure); 
   // TIM_SetCounter(TTOOLSTIMER,0);
   LL_TIM_SetCounter(TTOOLSTIMER,0);                                     
   //  TIM_ClearITPendingBit(TTOOLSTIMER,TIM_IT_CC1); 
   LL_TIM_ClearFlag_CC1(TTOOLSTIMER);                                    
   // TIM_Cmd(TTOOLSTIMER, ENABLE);
   LL_TIM_EnableCounter(TTOOLSTIMER); 
}

/**
 * @brief timer tools precision delay
 * 
 * @author byk1 (6/15/2018)
 * 
 * @param delayInMicroSeconds 
 */
void timerToolsPrecisionDelay(uint32_t delayInMicroSeconds)
{
//   SEMA_GET_WAIT_FOREVER(timerToolsMutexSema);
   startTimer(delayInMicroSeconds);
   SEMA_GET_WAIT_FOREVER(timerToolsCompleteSema);
//   SEMA_PUT(timerToolsMutexSema);
}


