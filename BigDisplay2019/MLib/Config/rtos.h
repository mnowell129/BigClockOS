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
#ifndef RTOS_H
#define RTOS_H
#include "freertos.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"


#define RTOS_TICKS_PER_SECOND configTICK_RATE_HZ


#define CONTEXT               BaseType_t error

#define ISRCONTEXT            BaseType_t    preemptYes = pdFALSE
#define ENTER_CRITICAL()      taskENTER_CRITICAL()
#define EXIT_CRITICAL()       taskEXIT_CRITICAL()

#define ISR_CRITICAL_CONTEXT     uint32_t  basePriorityValue
#define RTOS_SEMA             void
#define RTOS_SEMA_PTR         SemaphoreHandle_t


#define RTOS_SEMA_PTR         SemaphoreHandle_t
#define RTOS_SEMA_OBJECT(a)   SemaphoreHandle_t a;StaticSemaphore_t a##Data
#define RTOS_COUNTING_SEMA_OBJECT(a,b,c)   SemaphoreHandle_t a;StaticSemaphore_t a##Data;\
const uint32_t a##_INITIAL = b;const uint32_t a##_MAX = c
#define EXTERN_RTOS_SEMA_OBJECT(a)   extern SemaphoreHandle_t a;extern StaticSemaphore_t a##Data
#define RTOS_SEMA_DATA(a)     StaticSemaphore_t  a##Data

#define SEMA_CREATE(a)               a = xSemaphoreCreateCountingStatic(a##_MAX,a##_INITIAL,&a##Data)
#define SEMA_CREATE_STATIC(a)               a = xSemaphoreCreateCountingStatic(a##_MAX,a##_INITIAL,&a##Data)
#define SEMA_CREATE_COUNTING(ptr,max,initial)               ptr = xSemaphoreCreateCounting(max,initial)
#define SEMA_CREATE_BINARY(a)                 a=xSemaphoreCreateBinary();xSemaphoreGive(a)
#define SEMA_CREATE_BINARY_STATIC(a)                 a=xSemaphoreCreateBinaryStatic(&a##Data);xSemaphoreGive(a)
#define SEMA_CREATE_BINARY_STATIC_NO_GIVE(a)         a=xSemaphoreCreateBinaryStatic(&a##Data)

#define SEMA_GET(a,t,e)       e = xSemaphoreTake(a,t)
#define SEMA_GET_WAIT_FOREVER(a)       xSemaphoreTake(a,portMAX_DELAY)
#define SEMA_PUT(a)           xSemaphoreGive(a)
#define ISR_SEMA_GET(a,t,e)   e = xSemaphoreTakeFromISR(a,t)
#define ISR_SEMA_PUT(a)       xSemaphoreGiveFromISR(a,&preemptYes)
#define ISR_EXIT()            portEND_SWITCHING_ISR(preemptYes)

#define WAIT_FOREVER                 portMAX_DELAY
#define WAIT_NOT_AT_ALL       0
#define OS_ERROR_NONE         pdPASS
#define OS_ERROR_TIMEOUT      errQUEUE_EMPTY
#define ISR_EXIT()            portEND_SWITCHING_ISR(preemptYes)

#define FAILTEST(e)           ((e) == pdFALSE)
#define PASSTEST(e)           ((e) != pdFALSE)
#define OSTIME()                 ((int32_t)(xTaskGetTickCount()))

#define RTOS_DELAY(a)            vTaskDelay(a)
#define RTOS_TICKS_PER_SECOND    configTICK_RATE_HZ
#define MSECS_TO_TICKS(a)        (a == WAIT_FOREVER ? WAIT_FOREVER : ((a)*1000/RTOS_TICKS_PER_SECOND))
#define RTOS_MSEC_DELAY(a)       vTaskDelay(MSECS_TO_TICKS(a))


#define SECONDS_FROM_MSEC(v)            ((v)/1000)
#define MINUTES_FROM_SECONDS(v)         ((v)/60)
#define HOURS_FROM_MINUTES(v)           ((v)/60)
#define SECONDS_FROM_MINUTES(v)         ((v)*60)
#define MINUTES_FROM_HOURS(v)           ((V)*60)
#define SECONDS_PER_MINUTE              (60)
#define MINUTES_PER_HOUR                (60)
#define HOURS_PER_DAY                   (24)
#define MINUTES_PER_DAY                 (MINUTES_PER_HOUR * HOURS_PER_DAY)
#define SECONDS_PER_HOUR                (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define MSEC_PER_SEC                    (1000)
#define MSEC_PER_MINUTE                 (MSEC_PER_SEC * SECONDS_PER_MINUTE)
#define MSEC_PER_HOUR                   (MSEC_PER_MINUTE * MINUTES_PER_HOUR)
#define MSEC_PER_DAY                    (HOURS_PER_DAY*MSEC_PER_HOUR)
#define TICKS_PER_MINUTE                (RTOS_TICKS_PER_SECOND * SECONDS_PER_MINUTE)

#define MINUTES_TO_TICKS(v)             ((v) * TICKS_PER_MINUTE)
#define SECONDS_TO_TICKS(v)             ((v) * RTOS_TICKS_PER_SECOND)
#define HOURS_TO_TICKS(v)               ((v) * MINUTES_PER_HOUR * RTOS_TICKS_PER_SECOND * SECONDS_PER_MINUTE)

#define TICKS_TO_MINUTES(v)             ((v)/ TICKS_PER_MINUTE)
#define TICKS_TO_SECONDS(v)             ((v)/ RTOS_TICKS_PER_SECOND)


#endif
