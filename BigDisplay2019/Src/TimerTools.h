/**
 ***************************************************************
 ***************************************************************
 * MEDTRONIC CONFIDENTIAL - This document is the property of 
 * Medtronic, Inc.,and must be accounted for. Information herein 
 * is confidential. Do not reproduce it, reveal it to 
 * unauthorized persons, or send it outside Medtronic without 
 * proper authorization.
 * 
 * Copyright Medtronic, Inc. 2018
 ***************************************************************
 ***************************************************************
 *  
 * @brief Timer Tools
 * Mostly to make a precision delay function
 *  
 * @author nowelc2 
 * 
 ***************************************************************
 ***************************************************************
 */

#ifndef TIMERTOOLS_H
#define TIMERTOOLS_H

///*************************************************************
///*************************************************************

#ifdef DO_DECLARE
RTOS_SEMA_OBJECT(timerToolsMutexSema);
RTOS_SEMA_OBJECT(timerToolsCompleteSema);
#else
EXTERN_RTOS_SEMA_OBJECT(timerToolsMutexSema);
EXTERN_RTOS_SEMA_OBJECT(timerToolsCompleteSema);
#endif

///*************************************************************
///*************************************************************

#define ONE_MILLION   1000000u

///*************************************************************
///*************************************************************

void enableTimerClock(uint32_t timerNumber);
void disableTimerClock(uint32_t timerNumber);

void initTimerTools(void);
uint32_t getTimerClockFrequency(TIM_TypeDef *TIMx);
void timerToolsPrecisionDelay(uint32_t delayInMicroSeconds);

///*************************************************************
///*************************************************************


#endif // #ifndef TIMERTOOLS_H
