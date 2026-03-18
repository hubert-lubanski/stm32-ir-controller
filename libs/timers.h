/**
 * @file timers.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl) indeks: 421329
 * 
 * @brief Biblioteka implementująca podstawowe zarządzanie licznikami oraz
 *        umożliwiająca pracę z licznikami na trochę wyższym poziomie.
 * 
 * 
 */

#ifndef TIMERS_H
#define TIMERS_H

#include "my_preprocessor.h"

#ifndef NO_STM
#include <stm32.h>
#include <stdbool.h>


#define TIMER_DIR_UP    0                               ///< Zliczanie w góre
#define TIMER_DIR_DOWN  TIM_CR1_DIR                     ///< Zliczanie w dół

/** @brief Zliczanie w góre i w dół */
#define TIMER_DIR_BOTH  (TIM_CR1_CMS_0 | TIM_CR1_CMS_1) 

/// Bit włączenia przerwania na linii @p x
#define TIMER_CC_ENABLE(x)  TIM_DIER_CC ## x ## IE
/// Bit wyłączenia przerwania na linii @p x
#define TIMER_CC_DISABLE(x) TIM_DIER_CC ## x ## DE
/// Bit flagi przerwania na linii @p x
#define TIMER_SR_CCF(x)     TIM_SR_CC ## x ## IF

/// Bit włączenia zdarzenia zgodności na linii @p x
#define TIM_CCER_CC_ENABLE(x) TIM_CCER_CC ## x ## E
/// Bit stanu aktywacji podczas zdarzenia zgodności na linii @p x
#define TIM_CCER_CC_POLARITY(x) TIM_CCER_CC ## x ## P


/**
 * @struct timer_info_struct
 * @brief Struktrua przechowująca podstawowe informacje o liczniku
 * 
 * 
 * @var timer_info_struct::tim
 *      @brief Wskaźnik na zadany licznik
 * 
 * @var timer_info_struct::update_interrupt_number
 *      @brief Numer przerwania tego licznika
 * 
 * @var timer_info_struct::clock_freq_hz
 *      @brief Bazowa częstotliwość taktowania licznika
 * 
 * @var timer_info_struct::gpio_af
 *      @brief Docelowy numer funkcji alternatywnej związanej z tym licznikiem
 * 
 * @var timer_info_struct::enable_clock
 *      @brief Wskaźnik na funkcję włączającą taktowanie układu licznika
 */
typedef struct timer_info_struct {
    TIM_TypeDef * const tim; 
    const IRQn_Type update_interrupt_number;
    uint32_t clock_freq_hz;
    const uint8_t gpio_af;
    void (* const enable_clock)(void);
    uint8_t psc_bit_width, other_bit_width;
} timer_info_t;

/// @brief Bazowe taktowanie mikrokontrolera
#define BASE_HSI_HZ 16000000ull

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void enable_tim1(void) { RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; }
void enable_tim2(void) { RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; }
void enable_tim3(void) { RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; }
void enable_tim4(void) { RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; }
void enable_tim5(void) { RCC->APB1ENR |= RCC_APB1ENR_TIM5EN; }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/// @brief Definicja parametrów dla licznika numer 1
const timer_info_t timer1 =
    {TIM1, TIM1_UP_TIM10_IRQn, BASE_HSI_HZ, GPIO_AF_TIM1, enable_tim1, 16, 16};
/// @brief Definicja parametrów dla licznika numer 2
const timer_info_t timer2 =
    {TIM2, TIM2_IRQn,          BASE_HSI_HZ, GPIO_AF_TIM2, enable_tim2, 16, 32};
/// @brief Definicja parametrów dla licznika numer 3
const timer_info_t timer3 =
    {TIM3, TIM3_IRQn,          BASE_HSI_HZ, GPIO_AF_TIM3, enable_tim3, 16, 16};
/// @brief Definicja parametrów dla licznika numer 4
const timer_info_t timer4 =
    {TIM4, TIM4_IRQn,          BASE_HSI_HZ, GPIO_AF_TIM4, enable_tim4, 16, 16};
/// @brief Definicja parametrów dla licznika numer 5
const timer_info_t timer5 =
    {TIM5, TIM5_IRQn,          BASE_HSI_HZ, GPIO_AF_TIM5, enable_tim5, 16, 32};


/* ************************************************************************** **
                    Timer activation functions
** ************************************************************************** */
/// @brief Uruchomienie zliczania licznika @p *t @param *t wskaźnik na licznik
void timer_enable(TIM_TypeDef *t)  { t->CR1 |= TIM_CR1_CEN; }
/// @brief Zaprzestanie zliczania licznika @p *t @param *t wskaźnik na licznik
void timer_disable(TIM_TypeDef *t) { t->CR1 &= ~(TIM_CR1_CEN); }

/// @brief Wygenerowanie zdarzenia aktualizacji licznika @p *t
/// @param *t wskaźnik na licznik
void timer_update(TIM_TypeDef *t) { t->EGR = TIM_EGR_UG; }
/**
 * @brief Wygenerowanie zdarzenia zgodności na linii @p ccr licznika @p *t
 * 
 * @param *t    Wskaźnik na licznik
 * @param ccr   Numer linii 
 */
void timer_generate_cc_event(TIM_TypeDef *t, uint8_t ccr) {
    t->EGR = ((1ul) << ccr);
}


/* ************************************************************************** **
                        Basic timer setup functions
** ************************************************************************** */
/**
 * @enum timer_dir_enum
 * @brief Enumerator kierunków zliczania licznika
*/
typedef enum timer_dir_enum {
    Count_Up    = TIMER_DIR_UP,
    Count_Down  = TIMER_DIR_DOWN,
    Count_Both  = TIMER_DIR_BOTH
} timer_direction;

/**
 * @brief Ustawienie początkowych parametrów zliczania licznika
 * 
 * @param *t            Wskaźnik na licznik 
 * @param prescaler     Wartość prescalera do załadowania (rejestr PSC)
 * @param auto_reload   Wartość do której zliczamy (rejestr ARR)
 */
void timer_set_counting(TIM_TypeDef *t, uint32_t prescaler, uint32_t auto_reload){
    t->ARR = auto_reload;
    t->PSC = prescaler;
}

/**
 * @brief Ustawienie wartości licznika na zadaną wartość
 * 
 * @param *t        Wskaźnik na licznik 
 * @param initial   Nowa wartość licznika
 */
void timer_set_initial(TIM_TypeDef *t, uint32_t initial){
    t->CNT = initial;
}

/**
 * @brief Ustawienie wartości porównywanej w rejestrze o zadanym numerze
 * 
 * @param *t        Wskaźnik na licznik 
 * @param ccr       Numer rejestru/linii porównywania
 * @param val       Wartość porównywana
 */
void timer_set_ccr(TIM_TypeDef *t, uint8_t ccr, uint32_t val){
    #define func(x) t->CCR ## x = val
    CONST_SWITCH(4, ccr, func);
    #undef func
}

void timer_set_ccr_16(TIM_TypeDef *t, uint8_t ccr, uint16_t val){
    #define func(x) t->CCR ## x = val
    CONST_SWITCH(4, ccr, func);
    #undef func
}

/**
 * @brief Ustawienie kierunku zliczania licznika
 * 
 * @param *t        Wskaźnik na licznik 
 * @param dir       Kierunek zliczania
 */
void timer_set_direction(TIM_TypeDef *t, timer_direction dir){
    t->CR1 |= (uint32_t)dir;
}

/**
 * @brief Ustawienie licznika w tryb One Pulse Mode
 *        Licznik w tym trybie wyłączy się przy następnym zdarzeniu aktualizacji
 * 
 * @param *t        Wskaźnik na licznik 
 */
void timer_set_one_pulse(TIM_TypeDef *t){
    t->CR1 |= TIM_CR1_OPM;
}

/**
 * @brief Wyłączenie licznika z trybu One Pulse Mode
 * 
 * @param *t        Wskaźnik na licznik 
 */
void timer_disable_one_pulse(TIM_TypeDef *t){
    t->CR1 &= ~TIM_CR1_OPM;
}

/* ************************************************************************** **
                            Timer interrupts
** ************************************************************************** */


/**
 * @brief Włącza generowanie przerwania związanego ze zdarzeniam uaktualnienia
 * 
 * @param *t Wskaźnik na licznik 
 */
void timer_enable_update_interrupt(TIM_TypeDef *t){
    t->SR = ~TIM_SR_UIF;
    t->DIER |= TIM_DIER_UIE;
}

/**
 * @brief Wyłącza generowanie przerwania związanego ze zdarzeniam uaktualnienia
 * 
 * @param *t Wskaźnik na licznik 
 */
void timer_disable_update_interrupt(TIM_TypeDef *t){
    t->CR1 |= TIM_CR1_UDIS;
}

/**
 * @brief Czy flaga przerwania związanego ze zdarzeniem uaktualnienia jest
 *        ustawiona?
 * 
 * @param *t    Wskaźnik na licznik 
 */
bool timer_check_update_interrupt_flag(TIM_TypeDef *t){
    return ((t->SR & t->DIER) & TIM_SR_UIF);
}
/**
 * @brief Czyści flagę przerwania związanego ze zdarzeniem uaktualnienia
 * 
 * @param *t    Wskaźnik na licznik 
 */
void timer_clear_update_interrupt_flag(TIM_TypeDef *t){
    t->SR = ~(TIM_SR_UIF);
}

/**
 * @brief Włącza generowanie przerwania związanego ze zdarzeniem zgodności na
 *        linii @p cc
 * 
 * @param *t     Wskaźnik na licznik 
 * @param cc     Numer linii zgodności
 */
void timer_enable_cc_interrupt(TIM_TypeDef *t, uint8_t cc){
    #define func(x)                     \
        t->SR   = ~(TIMER_SR_CCF(x));   \
        t->DIER |= TIMER_CC_ENABLE(x);
        
    CONST_SWITCH(4, cc, func)
    #undef func
}

/**
 * @brief Wyłącza generowanie przerwania związanego ze zdarzeniem zgodności na
 *        linii @p cc
 * 
 * @param *t     Wskaźnik na licznik 
 * @param cc     Numer linii zgodności
 */
void timer_disable_cc_interrupt(TIM_TypeDef *t, uint8_t cc){
    #define func(x) \
        t->DIER |= TIMER_CC_DISABLE(x);
    
    CONST_SWITCH(4, cc, func);
    #undef func
}

/**
 * @brief Czy flaga przerwania związana ze zdarzeniem zgodności na linii @p cc
 *        została ustawiona?
 * 
 * @param *t        Wskaźnik na licznik 
 * @param cc        Numer linii zgodności
 */
bool timer_check_cc_interrupt_flag(TIM_TypeDef *t, uint8_t cc){
    // #define func(x) return ((t->SR & t->DIER) & (TIM_SR_CC##x##IF))
    // CONST_SWITCH(4, cc, func);
    // #undef func
    uint32_t it_status = t->SR;
    switch (cc){
        case 1:
            return it_status & TIM_SR_CC1IF;
            
        case 2:
            return it_status & TIM_SR_CC2IF;
            
        case 3:
            return it_status & TIM_SR_CC3IF;
            
        case 4:
            return it_status & TIM_SR_CC4IF;
            
        default:
            for(;;);
    }
}

/**
 * @brief Czyści flagę przerwania zwiazaną ze zdarzeniem zgodności na linii
 *        @p cc .
 * 
 * @param *t    Wskaźnik na licznik 
 * @param cc    Numer linii zgodności
 */
void timer_clear_cc_interrupt_flag(TIM_TypeDef *t, uint8_t cc){
    // #define func(x) t->SR = ~(TIM_SR_CC ## x ## IF)
    // CONST_SWITCH(4, cc, func);
    // #undef func
    switch (cc){
        case 1:
            t->SR = ~TIM_SR_CC1IF;
            break;
        case 2:
            t->SR = ~TIM_SR_CC2IF;
            break;
        case 3:
            t->SR = ~TIM_SR_CC3IF;
            break;
        case 4:
            t->SR = ~TIM_SR_CC4IF;
            break;
        default:
            for(;;);
    }
}

/*******************************************************************************
        Timer register preload
*******************************************************************************/
/**
 * Włącza wstępne ładowanie rejestru CCRx (gdzie x jest zadany przez @p ccr ) 
 * (do tzw. shadow register)
 * @param *t Wskaźnik na licznik
 * @param ccr Numer linii
*/
void timer_set_ccr_preload(TIM_TypeDef *t, uint8_t ccr){
    switch (ccr) {
        case 1:
            t->CCMR1 |= TIM_CCMR1_OC1PE;
            break;
        case 2:
            t->CCMR1 |= TIM_CCMR1_OC2PE;
            break;
        case 3:
            t->CCMR2 |= TIM_CCMR2_OC3PE;
            break;
        case 4:
            t->CCMR2 |= TIM_CCMR2_OC4PE;
            break;
    }
}
/**
 * Wyłącza wstępne ładowanie rejestru CCRx (gdzie x jest zadany przez @p ccr ) 
 * (do tzw. shadow register)
 * @param *t Wskaźnik na licznik
 * @param ccr Numer linii
*/
void timer_unset_ccr_preload(TIM_TypeDef *t, uint8_t ccr){
    switch (ccr) {
        case 1:
            t->CCMR1 &= ~TIM_CCMR1_OC1PE;
            break;
        case 2:
            t->CCMR1 &= ~TIM_CCMR1_OC2PE;
            break;
        case 3:
            t->CCMR2 &= ~TIM_CCMR2_OC3PE;
            break;
        case 4:
            t->CCMR2 &= ~TIM_CCMR2_OC4PE;
            break;
    }
}
/// @brief Włącza wstępne ładowanie rejestru ARR (do tzw. shadow register)
/// @param *t Wskaźnik na licznik
void timer_set_arr_preload(TIM_TypeDef *t)   { t->CR1 |=  TIM_CR1_ARPE; }

/// @brief Wyłącza wstępne ładowanie rejestru ARR (do tzw. shadow register)
/// @param *t Wskaźnik na licznik
void timer_unset_arr_preload(TIM_TypeDef *t) { t->CR1 &= ~TIM_CR1_ARPE; }

/* ************************************************************************** **
        Timer line output confiugration
** ************************************************************************** */
/// Enumerator możliwych zachowań linii podczas zdarzenia zgodności.
typedef enum line_ouput_modes {
    FROZEN = 0,         ///< dont change state of this line
    MATCH_ACTIVE = 1,   ///< on match set to high
    MATCH_LOW = 2,      ///< on match set to low
    TOGGLE = 3,         ///< on match toggle
    ALWAYS_LOW = 4,     ///< always low state
    ALWAYS_HIGH = 5,    ///< always high state
    PWM1 = 6,           ///< active below
    PWM2 = 7            ///< active above
} line_mode;

/**
 * @brief Włącza reagowanie zadanej linii licznika na zdarzenie zgodności
 * 
 * @param *t        Wskaźnik na licznik 
 * @param line      Numer linii
 * @param active_low    Czy stan aktywny lini to stan niski?
 */
void timer_enable_output_line_compare(TIM_TypeDef *t, uint8_t line, bool active_low){
    #define func(x)                                 \
        t->CCER |= TIM_CCER_CC_ENABLE(x);           \
        if (active_low) {                           \
            t->CCER |= TIM_CCER_CC_POLARITY(x);     \
        } else {                                    \
            t->CCER &= ~(TIM_CCER_CC_POLARITY(x));  \
        }

    CONST_SWITCH(4, line, func);
    #undef func  
}
/**
 * @brief Ustawia zadaną linię licznika w podany tryb działania w przypadku
 *        zdarzenia zgodności.
 * 
 * @param *t    Wskaźnik na licznik
 * @param line  Numer linii
 * @param m     Tryb zachowania linii
 */
void timer_set_line_output_mode(TIM_TypeDef *t, uint8_t line, line_mode m){
    switch (line) {
        case 1:
            t->CCMR1 |= ((uint32_t)m << TIM_CCMR1_OC1M_Pos);
            break;
        case 2:
            t->CCMR1 |= ((uint32_t)m << TIM_CCMR1_OC2M_Pos);
            break;
        case 3:
            t->CCMR2 |= ((uint32_t)m << TIM_CCMR2_OC3M_Pos);
            break;
        case 4:
            t->CCMR2 |= ((uint32_t)m << TIM_CCMR2_OC4M_Pos);
            break;
    }
}

/* ************************************************************************** **
        MASTER - SLAVE SETUP FUNCTIONS
** ************************************************************************** */


static uint8_t tim_pointer_to_number(TIM_TypeDef *t){
    if (t == TIM1) return 1;
    if (t == TIM2) return 2;
    if (t == TIM3) return 3;
    if (t == TIM4) return 4;
    if (t == TIM5) return 5;
    if (t == TIM9) return 9;
    if (t == TIM10) return 10;
    return -1;
}

static const uint8_t tim2_slave_triggers[] = {[1] = 0,[3] = 2,[4] = 3};
static const uint8_t tim3_slave_triggers[] = {[1] = 0,[2] = 1,[5] = 2,[4] = 3};
static const uint8_t tim4_slave_triggers[] = {[1] = 0,[2] = 1,[3] = 2};
static const uint8_t tim5_slave_triggers[] = {[2] = 0,[3] = 1,[4] = 2};
static const uint8_t *slaves_triggers[] = {
    [2] = tim2_slave_triggers,
    [3] = tim3_slave_triggers,
    [4] = tim4_slave_triggers,
    [5] = tim5_slave_triggers
};
/**
 * @brief Procedura ustawiająca wskazane liczniki w konfiguracji 
 *        master-salve w trybie gate
 * 
 * @param master    Wskaźnik na licznik nadrzędny
 * @param slave     Wskaźnik na licznik podrzędny
 * @param master_line   Numer linii aktywacyjnej licznika nadrzędnego
 */
void timer_setup_master_enable_slave(TIM_TypeDef *master, TIM_TypeDef *slave,
                                     uint8_t master_line){
    
    uint32_t tmp;
    timer_disable(master);
    timer_disable(slave);

    // Ustawienie Master Mode Selection na sygnał z wybranej lini
    uint8_t master_mode_compare = 4 + (master_line - 1);

    tmp = master->CR2;
    tmp &= ~TIM_CR2_MMS_Msk;
    tmp |= ((master_mode_compare) << TIM_CR2_MMS_Pos);
    master->CR2 = tmp;

    // Ustawienie trybu gate oraz odp. triggeru włączania zależnego
    // od licznika nadrzędnego
    uint8_t master_n = tim_pointer_to_number(master);
    uint8_t slave_n  = tim_pointer_to_number(slave);

    const uint8_t *triggers = slaves_triggers[slave_n];
    uint8_t trigger = triggers[master_n];

    tmp = slave->SMCR;
    tmp &= ~TIM_SMCR_SMS_Msk;               // wyzeruj tryb
    tmp |= ((5ul) << TIM_SMCR_SMS_Pos);     // ustaw gate mode
    tmp &= ~TIM_SMCR_TS_Msk;                // wyzeruj trigger
    tmp |= (trigger << TIM_SMCR_TS_Pos);    // ustaw trigger mastera
    slave->SMCR = tmp;

    // synchronizacja
    timer_update(master);
    timer_update(slave);

    timer_enable(slave);
}


/* ************************************************************************** **
        DMA REQUEST SETUP
** ************************************************************************** */


/**
 * @brief Włącza sygnalizowania żądań DMA przy Capture Compare Event
 * 
 * @param t     Wskaźnik na licznik, którego żadania chcemy włączyć
 * @param line  Numer linii, której żądanie chcemy włączyć
 */
void timer_enable_dma_request(TIM_TypeDef *t, uint8_t line){
    #define func(x) t->DIER |= TIM_DIER_CC ## x ## DE;

    CONST_SWITCH(4, line, func);
    #undef func
}

/**
 * @brief Włącza sygnalizowanie żądań DMA przy Udpate Event
 * 
 * @param t     Wskaźnik na licznik, którego żądania chcemy włączyć
 */
void timer_enable_update_dma_request(TIM_TypeDef *t){
    t->DIER |= TIM_DIER_UDE;
}

/// @brief Przekazuje wartość rejestru CNT licznika @p *t
/// @param *t    Wskaźnik na licznik
/// @return t->CNT
uint32_t time_get(TIM_TypeDef *t){
    return t->CNT;
}
#endif // NO_STM

/* ************************************************************************** **
                        Helper functions
** ************************************************************************** */


static inline unsigned char bit_width(unsigned long long x) {
    return x == 0 ? 1 : 64 - __builtin_clzll(x);
}

/**
 * @brief Procedura obliczająca podłogę z pierwiastka z liczby całkowitej
 * 
 * @note Zaczerpnięte z //https://stackoverflow.com/a/63452286/22808363
 * 
 * @param n     Liczba, której pierwiastka szukamy
 * @return floor(sqrt(n))
 */
unsigned floor_sqrt(const unsigned n) {
    unsigned char shift = bit_width(n);
    shift += shift & 1; // round up to next multiple of 2

    unsigned result = 0;

    do {
        shift -= 2;
        result <<= 1; // leftshift the result to make the next guess
        result |= 1;  // guess that the next bit is 1
        result ^= result * result > (n >> shift); // revert if guess too high
    } while (shift != 0);

    return result;
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
    uint32_t floor_sqrt_X = floor_sqrt(X);
    if (a_max > floor_sqrt_X) {
        a_max = floor_sqrt_X + (floor_sqrt_X == 0); // Co najmniej 1
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


static uint32_t calc_psc(uint32_t timer_freq, uint32_t desired_freq){
    return (timer_freq + (desired_freq / 2u)) / desired_freq;
}

/**
 * @brief Oblicza wartość rejestru PSC oraz mnożnik na podstawie zadanych
 *        parametrów
 * 
 * @param timer_freq    Taktowanie zegara licznika 
 * @param proto_freq    Oczekiwana czętotliwość 
 * @param psc           Wskaźnik, gdzie zostanie zapisana obliczona wartosść PSC
 * @param psc_bit_width Szerokość rejestru PSC (w bitach)
 * @param mult          Wskaźnik, gdzie zostanie zapisana wartość mnożnika
 * @param mult_bit_width Maksymalna szerokość licznika (w bitach)
 * @param mult_div      Wymagany dzielnik mnożnika
 * @return int  Nie zerowa wartość w przypadku błędu
 */
int get_psc_mult(uint32_t timer_freq, uint32_t proto_freq,
                 uint32_t *psc, uint32_t psc_bit_width,
                 uint32_t *mult, uint32_t mult_bit_width,
                 uint32_t mult_div){
    
    uint32_t allowed_psc_max = UINT32_MAX;
    uint64_t psc_max_value = (1ull << (psc_bit_width))-1;
    if (psc_max_value < (uint64_t)allowed_psc_max)
        allowed_psc_max = psc_max_value;
    
    uint32_t allowed_mult_max = UINT32_MAX;
    uint64_t mult_max_value = (1ull << (mult_bit_width))-1;
    if (mult_max_value < (uint64_t)allowed_mult_max)
        allowed_mult_max = mult_max_value;

    
    uint32_t desired_count_freq = proto_freq;
    uint32_t resulting_psc =
        calc_psc(timer_freq, desired_count_freq);
    
    // Rozbijamy pożądany podzielnik na kombinację PSC i Multiplier
    uint32_t ret = find_best_product(resulting_psc, mult, psc,
                                     allowed_mult_max, allowed_psc_max,
                                     mult_div);

    if (ret != 0)
        return ret;

    if ((*mult) % mult_div != 0){
        *psc /= mult_div;
        *mult *= mult_div;
    }
    return 0;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/*******************************************************************************
        WAITING
*******************************************************************************/

#ifndef NO_STM
#include <interrupts.h>
#define WAITING_PRIORITY 0
typedef struct waiting_timer_struct {
    TIM_TypeDef *timer;
    uint32_t mult;
} waiting_timer_t;

waiting_timer_t set_waiter(const timer_info_t *t){
    timer_disable(t->tim);
    uint32_t psc, mult;

    get_psc_mult(t->clock_freq_hz, 1000, &psc, 16, &mult, 16, 1);

    t->tim->PSC = psc - 1;
    timer_update(t->tim);

    timer_enable_update_interrupt(t->tim);
    // timer_set_one_pulse(t->tim);
    NVIC_EnableIRQ(t->update_interrupt_number);
    IRQsetPriority(t->update_interrupt_number, WAITING_PRIORITY, 0);
    return (waiting_timer_t){.timer = t->tim, .mult = mult};
}

void wait_on(const waiting_timer_t stopper, uint32_t ms){
    TIM_TypeDef *tim = stopper.timer;
    tim->ARR = stopper.mult * ms -1;
    timer_update(tim);
    irq_level_t level = IRQprotect(WAITING_PRIORITY + 1);

    timer_enable(tim);

    __NOP();    // Ten NOP jest wymagany, aby uniknąć deadlocka!
    // TODO wyłączyć wszystkie pozostałe przerwania!
    __WFI();

    __NOP();

    timer_disable(tim);
    IRQunprotectAll(level);
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
#endif // NO_STM

#endif /* TIMERS_H */