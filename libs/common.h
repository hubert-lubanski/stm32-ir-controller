/**
 * @file common.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl)
 * 
 * @brief Definicje często spotykanych funkcji
 * 
 */
#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

void memcopy(void * restrict dest, const void * restrict src, uint32_t count) {
    char *to = (char *)dest;
    char *from = (char *)src;
    for (uint32_t i = 0; i < count; ++i){
        to[i] = from[i];
    }
}

void memfill(void * restrict dest, uint8_t val, uint32_t count){
    uint8_t *mem = (uint8_t *)dest;
    for (uint32_t i = 0; i < count; ++i){
        mem[i] = val;
    }
}


#endif /* COMMON_H */