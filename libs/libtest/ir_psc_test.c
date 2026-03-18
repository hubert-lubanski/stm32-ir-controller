#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../my_preprocessor.h"

static __attribute__((always_inline)) uint32_t calc_psc(uint32_t timer_freq, uint32_t desired_freq){
    return (timer_freq + (desired_freq / 2u)) / desired_freq;
}

static inline double diff(uint32_t orig, uint32_t calc){
    return  fabs((double)orig - (double)calc)/(double)orig;
}

/**
 * @brief Znajduje parę liczb A, B, należących odpowiednio do zadanych
 * przedziałów, tak aby iloczyn A*B najlepiej przybliżał zadaną wartość X.
 * Liczby A oraz B są przekazywane przez wskaźniki.
 *        
 * 
 * @param X     - Liczba, którą chcemy rozbić na iloczyn A i B
 * @param num1  - Wskaźnik na liczbę A
 * @param num2  - Wskaźnik na liczbę B
 * @param max1  - Maksymalna wartość liczby A
 * @param max2  - Maksymalna wartość liczby B
 * @param div   - Wymagany dzielnik jednej z liczb A lub B
 * @return int 
 */
int find_best_product(uint32_t X, uint32_t *num1, uint32_t *num2,
                      uint32_t max1, uint32_t max2, uint8_t div) {
    // Sprawdzamy czy to wykonalne
    if ((uint64_t)max1 * (uint64_t)max2 < X){
        return -1;
    }

    _Bool swap_values = false;
    uint32_t a_max = max1;
    uint32_t b_max = max2;

    if (max1 > max2){
        swap(a_max, b_max);
        swap_values = true;
    }
    // a <= b

    // Obcinanie zakresów
    uint32_t floor_sqrt_X = floor(sqrt(X));
    if (a_max > floor_sqrt_X) {
        a_max = floor_sqrt_X;
    }
    uint32_t a_min = max(1, (X + b_max - 1) / b_max);
    
    
    // Szukamy najbliższej pary dzielników X lub pary dającej najmniejszy błąd
    // wyrażenia X - a * b. 
    uint32_t a = a_max, b;
    uint32_t best_a = 0, best_b = 0;
    uint32_t difference = UINT32_MAX;

    do {
        b = X / a;
        uint32_t new_difference = X % a; // <=> X - a * b
        if (new_difference < difference && (a % div == 0 || b % div == 0)){
            best_a = a;
            best_b = b;
            difference = new_difference;
        }
        a--;
    }  while (difference != 0 && a >= a_min);

    if (swap_values){
        swap(best_a, best_b);
    }

    *num1 = best_a;
    *num2 = best_b;
    return 0;
}



int get_psc_arr(uint32_t timer_freq, uint32_t proto_freq,
                uint32_t *psc, uint32_t psc_bit_width,
                uint32_t *arr, uint32_t arr_bit_width,
                uint32_t arr_div){
    
    uint32_t allowed_psc_max = UINT32_MAX;
    uint64_t psc_max_value = (1ull << (psc_bit_width))-1;
    if (psc_max_value < (uint64_t)allowed_psc_max)
        allowed_psc_max = psc_max_value;
    
    uint32_t allowed_arr_max = UINT32_MAX;
    uint64_t arr_max_value = (1ull << (arr_bit_width))-1;
    if (arr_max_value < (uint64_t)allowed_arr_max)
        allowed_arr_max = arr_max_value;

    
    uint32_t desired_count_freq = proto_freq;
    uint32_t resulting_psc =
        calc_psc(timer_freq, desired_count_freq);
    
    printf("Ideal PSC := %0.3f\n", (double)timer_freq / proto_freq);
    

    
    uint32_t ret = find_best_product(resulting_psc, arr, psc,
                                     allowed_arr_max, allowed_psc_max,
                                     arr_div);
    printf("ret := %lu | psc := %lu, arr := %lu\n", ret, *psc, *arr);

    if (ret != 0)
        return ret;

    printf("*arr %% arr_div := %lu\n", (*arr) % arr_div);

    if ((*arr) % arr_div != 0){
        *psc /= arr_div;
        *arr *= arr_div;
    }
    return 0;
}




const uint32_t timer_freq = 16000000ul;
const uint32_t proto_freq = 1000ul;

static inline void printpsc(uint32_t psc, uint32_t mult){
    printf("PSC := %lu,\tmultiplier := %lu\n", psc, mult);
    printf("PSC * freq * mult := %lu\n", psc * proto_freq * mult);
    printf("difference is %0.2f%%\n", 100.0 * diff(timer_freq, psc * proto_freq * mult));
}

int main(void){


    uint32_t psc_bits = 16, arr_bits = 16;
    uint32_t psc = 0, arr = 0;
    uint32_t ret = get_psc_arr(timer_freq, proto_freq , &psc, psc_bits, &arr, arr_bits, 1);
    printf("Timer frequency: %lu\nProtocol frequency: %lu\nPSC max: %lu\n\n",
            timer_freq, proto_freq, (1ul << psc_bits)-1);
    printpsc(psc, arr);

    uint32_t step = 2;
    uint32_t fpsc = 0, farr;
    ret = get_psc_arr(timer_freq, proto_freq , &fpsc, psc_bits, &farr, arr_bits, 2);
    printf("Fixed step of %lu:\n", step);
    printpsc(fpsc, farr);


}