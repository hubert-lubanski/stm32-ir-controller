/**
 * @file interrupts.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl)
 * 
 * @brief Procedury związane z innymi przerwaniami
 * 
 */
#ifndef INT_H
#define INT_H

#include <stm32.h>
#include <irq.h>

/**
 * @brief Czyści flagę przerwania modułu EXTI na linii @p line_number
 * 
 * @param line_number   Numer linii EXTI
 */
inline void EXTI_Interrupt_Clear(uint8_t line_number){
    EXTI->PR = 1 << line_number;
}

/**
 * @brief Czy flaga przerwania modułu EXTI na linii @p line_number jest
 *        ustawiona?
 * 
 * @param line_number   Numer linii EXTI
 */
inline bool EXTI_Interrupt_Check(uint32_t pr, uint8_t line_number){
    return pr & (1 << line_number);
}

#endif /* INT_H */