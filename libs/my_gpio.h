/**
 * @file my_gpio.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl)
 * 
 * @brief Dodatkowe definicje dla biblioteki gpio.h
 * 
 */

#ifndef MY_GPIO_H
#define MY_GPIO_H

#include <stm32.h>
#include <gpio.h>

#define GPIOA_enable_clock() \
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN


#define GPIOB_enable_clock() \
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN


#define GPIOC_enable_clock() \
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN

#define SYSCFG_enable_clock() \
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN

#endif /* MY_GPIO_H */
