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

/**
 * Macro abstraction of GPIO pins.
 * Can be rewritten easily to different pins and/or micro.
 */


#ifndef GPIOAPI_H
#define GPIOAPI_H

/*
 Assume each gpio has a name.
 For example EN_POWER
 The we need to do the following initalization:
 Turn on the clock.
 Set the pin mode, pullup etc.
 If gpio set the direction.
 If alternate function then set the alternate function.
 */

// macro to define the pin function
// CMN : For some reason this macro would not accept
// ENABLE_PIN(a); initGPIO....
// it would lose the symbol.
// Compiler bug
// So had to expand it here
// #define INITGPIOIO(a)          ENABLE_PIN(a);\
//                                initGPIOPin(a##_GROUP,a##_MODE,a##_PIN,\
//                                a##_GPIO_OTYPE,a##_PUPD,a##_SPEED,a##_DEFAULT)


// Macro to enable clock for the GPIO generally
// NOTE: all ports are on AHB2_GRP1 for stm32f4xx
// LL_AHB2_GRP1_PERIPH_GPIOA
// #define LL_APB2_GRP1_PERIPH_USART1       RCC_APB2ENR_USART1EN
// ultimate goal LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)
// LL_APB1_GRP1_EnableClock
// LL_AHB2_GRP1_EnableClock
//#define MAKE_RCC(b,a)            LL_##b##_GRP1_PERIPH_##a
//#define RCC_FUN(b,a)             LL_##b##_GRP1_EnableClock(MAKE_RCC(b,a))
#define MAKE_RCC(b,a)            LL_##b##_GRP1_EnableClock(a)
#define RCC_FUN(b,a)             MAKE_RCC(b,a)

#define ENABLE_CLOCK(a)        RCC_FUN(a##_BUS,a##_BUS_ID)

#define INITGPIOIO(a)          RCC_FUN(a##_BUS,a##_BUS_ID);\
                               initGPIOPin(a##_GROUP,a##_MODE,a##_PIN,\
                               a##_GPIO_OTYPE,a##_PUPD,a##_SPEED,a##_DEFAULT);

#define INITGPIOOUT(a)         RCC_FUN(a##_BUS,a##_BUS_ID);\
                               initGPIOPin(a##_GROUP,LL_GPIO_MODE_OUTPUT,a##_PIN,\
                               a##_GPIO_OTYPE,a##_PUPD,a##_SPEED,a##_DEFAULT);

#define INITGPIOIN(a)          RCC_FUN(a##_BUS,a##_BUS_ID);\
                               initGPIOPin(a##_GROUP,LL_GPIO_MODE_INPUT,a##_PIN,\
                               a##_GPIO_OTYPE,a##_PUPD,a##_SPEED,a##_DEFAULT);

#define INITGPIOAF(a)          RCC_FUN(a##_BUS,a##_BUS_ID);\
                               initAFPin(a##_GROUP,a##_PIN,\
                               a##_GPIO_OTYPE,a##_PUPD,a##_SPEED, a##_AF)


#define INITGPIOAN(a)          RCC_FUN(a##_BUS,a##_BUS_ID);\
                               initANPin(a##_GROUP,a##_PIN,\
                               a##_GPIO_OTYPE,a##_PUPD,a##_SPEED);

#define _PORT(a)               GPIO##a
#define PORT(a)                _PORT(a)
#define _GROUPID(a,b)            LL_AHB##b##_GRP1_PERIPH_GPIO##a
#define GROUPID(a,b)             _GROUPID(a,b)
#define _PIN(a)                LL_GPIO_PIN_##a
#define PIN(a)                 _PIN(a)

#define __PORTNAME(a)          a
#define _PORTNAME(a)           __PORTNAME(a##_GROUP)
#define _PINNUMBER(a)          a##_PIN_NUMBER

#define _HIGHGPIO(a)           _PORTNAME(a)->BSRR = 1 << _PINNUMBER(a)
#define HIGHGPIO(a)            _HIGHGPIO(a)
#define _LOWGPIO(a)            _PORTNAME(a)->BSRR = (1 << _PINNUMBER(a)) <<16
#define LOWGPIO(a)             _LOWGPIO(a)

// how many bits to shift for assert high, set is the low part the register                                  
#define ACTIVE_HIGH            (0)
#define ACTIVE_LOW             (16)

#define _ASSERTED(a)             a##_ASSERTED
#define ASSERTED(a)              _ASSERTED(a)
#define _DEASSERTED(a)           a##_DEASSERTED
#define DEASSERTED(a)            _DEASSERTED(a)

#define ASSERTGPIO(a)          _PORTNAME(a)->BSRR = (1 << _PINNUMBER(a)) << ASSERTED(a)
#define DEASSERTGPIO(a)        _PORTNAME(a)->BSRR = (1 << _PINNUMBER(a)) << DEASSERTED(a)
#define SETGPIO(a)      
#define CLEARGPIO(a)    



#define ISHIGH(a)              (((a##_GROUP->IDR)&(a##_PIN)) == a##_PIN)
#define ISLOW(a)               (!(((a##_GROUP->IDR)&(a##_PIN)) == a##_PIN))

#define ISTRUE(a)              (((a##_GROUP->IDR)&(a##_PIN)) == a##_IN_TRUE)
#define ISFALSE(a)             (((a##_GROUP->IDR)&(a##_PIN)) == a##_IN_FALSE)




void initGPIOPin(GPIO_TypeDef *group, uint32_t mode, uint32_t pin,
                 uint32_t otype, uint32_t pupd, uint32_t speed,
                 uint32_t defaultValue);
void initAFPin(GPIO_TypeDef *group, uint32_t pin, 
               uint32_t otype, uint32_t pupd, uint32_t speed,
               uint32_t af);
void initANPin(GPIO_TypeDef *group, uint32_t pin, 
               uint32_t otype, uint32_t pupd, uint32_t speed);
void GPIO_WriteBit(GPIO_TypeDef *group, uint32_t pin, uint32_t defaultValue);


#endif
