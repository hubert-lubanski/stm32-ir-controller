/**
 * @file buttons.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl)
 * 
 * @brief Mała biblioteczka do zarządzania przyciskami dostępnymi w
 *        zestawie laboratoryjnym.
 * 
 * 
 */
#ifndef BUTTONS_H
#define BUTTONS_H

#include <gpio.h>
#include <stm32.h>
#include <stdbool.h>

/**
 * @struct  button_info_struct
 * @brief   Struktura przechowująca informacje o danym przycisku
 * 
 * @var button_info_struct::GPIO
 *      @brief GPIO obsługujące dany guzik
 * 
 * @var button_info_struct::pin
 *      @brief Numer wyprowadzenia, do którego podłączony jest guzik
 * 
 * @var button_info_struct::activation
 *      @brief Czy guzik po wciśnięciu generuje stan wysoki (1) czy niski (0)?
 */
typedef struct button_info_struct {
    GPIO_TypeDef * GPIO;
    uint8_t pin : 4;
    uint8_t activation : 1;
} button_info;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define ACTIVE_LOW  0
#define ACTIVE_HIGH 1

#define USER_BTN_GPIO       GPIOC
#define USER_BTN_PIN        13

#define AT_MODE_BTN_GPIO    GPIOA
#define AT_MODE_BTN_PIN     0

#define JOYSTICK_GPIO       GPIOB
#define JOYSTICK_ACT_PIN    10
#define JOYSTICK_UP_PIN     5
#define JOYSTICK_DOWN_PIN   6
#define JOYSTICK_LEFT_PIN   3
#define JOYSTICK_RIGHT_PIN  4
#endif /* DOXYGEN_SHOULD_SKIP_THIS */ 

/// @brief Enumerator dostępnych przycisków na płytce 
enum buttons_enumerator {
    NO_BUTTON = -1,
    USER_BTN = 0,
    AT_MODE_BTN = 1,
    JOYSTICK_ACT = 2,
    JOYSTICK_UP = 3,
    JOYSTICK_DOWN = 4,
    JOYSTICK_LEFT = 5,
    JOYSTICK_RIGHT = 6
};

typedef enum buttons_enumerator button_type;

/// @brief Tablica definiująca parametry dostępnych przycisków
const button_info buttons[] = {
    [USER_BTN]       = {USER_BTN_GPIO, USER_BTN_PIN, ACTIVE_LOW},
    [AT_MODE_BTN]    = {AT_MODE_BTN_GPIO, AT_MODE_BTN_PIN, ACTIVE_HIGH},
    [JOYSTICK_ACT]   = {JOYSTICK_GPIO, JOYSTICK_ACT_PIN,   ACTIVE_LOW},
    [JOYSTICK_UP]    = {JOYSTICK_GPIO, JOYSTICK_UP_PIN,    ACTIVE_LOW},
    [JOYSTICK_DOWN]  = {JOYSTICK_GPIO, JOYSTICK_DOWN_PIN,  ACTIVE_LOW},
    [JOYSTICK_LEFT]  = {JOYSTICK_GPIO, JOYSTICK_LEFT_PIN,  ACTIVE_LOW},
    [JOYSTICK_RIGHT] = {JOYSTICK_GPIO, JOYSTICK_RIGHT_PIN, ACTIVE_LOW}
};

/**
 * @brief Czy przycisk jest w stanie wciśniętym?
 * 
 * @param bi    Struktura z informacjami o przycisku
 */
inline bool button_pressed(button_info bi){
    return ((bi.GPIO->IDR >> bi.pin) & 1) == bi.activation;
}

/**
 * @brief W jakim stanie jest wyprowadzenie podłączone do danego przycisku?
 * 
 * @param bi        Struktura z informacjami o przycisku
 * @return true     Wyprowadzenie jest w stanie wysokim
 * @return false    Wyprowadzenie jest w stanie niskim
 */
inline bool button_state(button_info bi){
    return (bi.GPIO->IDR >> bi.pin) & 1;
}



/**
 * @brief Konfiguruje układ GPIO dla zadanego przycisku
 * 
 * @param bi        Struktura z informacjami o przycisku
 * @param pull      Rezystor podpięty do wyprowadzenia.
 *                  Patrz @ref GPIOinCOnfigure w @ref gpio.h
 * @param mode      Tryb działania modułu przerwań zewnętrznych EXTI
 * @param trigger   Zbocze wyzwalające przerwanie EXTI
 */
inline void configure_button(button_info bi, uint8_t pull, uint8_t mode, uint8_t trigger){
    GPIOinConfigure(bi.GPIO,
                    bi.pin,
                    pull,
                    mode,
                    trigger);
}




#include <timers.h>

/**
 * @struct vibration_muffler_struct
 * @brief Struktura danych dla wykorzystywania licznika w celu tłumienia
 *        drgań sytków przycisków
 * 
 */
typedef struct vibration_muffler_struct {
    TIM_TypeDef *base_timer;            ///< Wskaźnik na wykorzystywany licznik
    /// Wymagany mnożnik rejestru ARR (domyślnie = 1)
    const uint32_t muffler_arr_mult;    
    /// Ostatnio wciśnięty przycisk (domyślnie NO_BUTTON) 
    button_type last_pressed;
    /// Czy wciśnięcie ostatnio wciśniętego przycisku zostało już obsłużone
    bool already_handled;
    /// Liczba cykli do odczekania przed sprawdzeniem stanu przycisku
    const uint32_t muffle_time_cycles;
    /// Procedura do wykonania w przypadku, gdy przycisk będzie wciśnięty
    void (*on_press_action)(button_type);

    /// Liczba cykli do odczekania przed sprawdzeniem przytrzymania przycisku
    const uint32_t long_press_cycles;
    /// Numer rejestru CCRx używanego do sprawdzania 
    const uint8_t long_press_ccr;
    /// Ile cykli jeszcze pozostało do odczekania, aby stwerdzić przytrzymanie
    uint32_t time_left;
    /// Procedura do wykonania w przypadku przytrzymania przycisku
    void (*long_press_action)(button_type);

} muffler_timer_t;

/**
 * @brief Inicjalizuje wykorzystywany licznik tłumika drgań styków
 * 
 * @param *mt    Wskaźnik na strukturę opisującą tłumik drgań styków
 */
void muffler_init_base_timer(muffler_timer_t *mt){
    timer_disable(mt->base_timer);
    timer_set_direction(mt->base_timer, Count_Up);
    timer_set_arr_preload(mt->base_timer);
    timer_set_ccr_preload(mt->base_timer, 1);
    
    mt->base_timer->CR1 |= TIM_CR1_URS;
    mt->base_timer->ARR = mt->muffler_arr_mult * mt->muffle_time_cycles - 1;
    mt->base_timer->CCR1 =  mt->base_timer->ARR + 1;

    timer_update(mt->base_timer);
    mt->base_timer->CR1 &= ~TIM_CR1_URS;
    //timer_set_ccr(mt->base_timer, mt->long_press_ccr, 0xffff);

    // timer_set_ccr(mt->base_timer, mt->long_press_ccr,
    //               mt->muffler_arr_mult * mt->muffle_time_cycles + 1);
    timer_enable_update_interrupt(mt->base_timer);
    // timer_enable_cc_interrupt(mt->base_timer, 1);
}

/**
 * @brief Procedura obsługi przerwania zdarzenia ukatualnienia dla licznika
 *        wykorzystywanego przez tłumik drgań styków
 * 
 * @param *mt - Wskaźnik na strukturę opisującą tłumik drgań styków
 */
void muffler_update_event_IRQHandler(muffler_timer_t *mt){
    // uint32_t it_status = mt->base_timer->SR & mt->base_timer->DIER;
    // if (it_status & TIM_SR_UIF){
    //     mt->base_timer->SR = ~TIM_SR_UIF;

    // // OK
    if (timer_check_update_interrupt_flag(mt->base_timer)){
        timer_clear_update_interrupt_flag(mt->base_timer);

        // Jeżeli guzik jest nadal wciśnięty to podejmujemy akcję
        // i resetujemy guzik
        if (mt->last_pressed != NO_BUTTON
        && button_pressed(buttons[mt->last_pressed])) {
            if (!mt->already_handled && mt->on_press_action)
                mt->on_press_action(mt->last_pressed);

            mt->already_handled = true;

            // Aktualizujemy time_left 
            if (mt->time_left >= mt->muffle_time_cycles) {
                mt->time_left -= mt->muffle_time_cycles;
            }
            else {
                // Czas minął w trakcie tego obrotu, resetujemy
                // biorąc pod uwagę ile odczekaliśmy
                // uint32_t overcount = mt->muffle_time_cycles - mt->time_left;
                // mt->time_left = mt->long_press_cycles - overcount;
                mt->time_left = mt->long_press_cycles;
            }

            // Powyżej ARR => nigdy nie dojdzie do zdarzenia zgodności
            uint32_t ccrval = mt->muffler_arr_mult * mt->muffle_time_cycles;
            if (mt->time_left < mt->muffle_time_cycles)
                ccrval = mt->muffler_arr_mult * mt->time_left;

            // mt->base_timer->CCR1 = ccrval;
        }
        else { // OK - wywołuje się rzeczywiście przy puszczeniu guzika
            // Jeżeli przycisk nie jest wciśnięty, to resetujemy stan
            timer_disable(mt->base_timer);
            mt->last_pressed = NO_BUTTON;
        }
    }

    // if (it_status & TIM_SR_CC1IF){
    //     mt->base_timer->SR = ~TIM_SR_CC1IF;

    // WARNING - Wykonuje się ZAWSZE, co każdy update event
    if (timer_check_cc_interrupt_flag(mt->base_timer, 1)){
        timer_clear_cc_interrupt_flag(mt->base_timer, 1);
    
        // Jeżeli przycisk nie jest wciśnięty, to resetujemy stan
        if (mt->last_pressed == NO_BUTTON
        || !button_pressed(buttons[mt->last_pressed])){
            timer_disable(mt->base_timer);
            mt->last_pressed = NO_BUTTON;
        }
        else {
            // Tutaj wejdziemy w.t.t gdy mamy obsłużyć long_press
            if (mt->long_press_action)
                mt->long_press_action(mt->last_pressed);
            
            mt->time_left = mt->long_press_cycles;
        }
    }
}

/**
 * @brief Procedura służąca do powiadomienia tłumika drgań styków o 
 *        wykryciu zdarzenia przyciśnięcia przycisku.
 * 
 * Jeżeli jest to ten sam przycisk co poprzedni (np. drganie styków)
 * zdarzenie jest ignorowane. W przeciwnym przypadku licznik jest zerowany.
 * 
 * @param *mt   - Wskaźnik na strukturę opisującą tłumik drgań styków
 * @param btn   - Enumerator przyciśniętego przycisku
 */
void signal_button_press(muffler_timer_t *mt, button_type btn){
    if (mt->last_pressed != btn) {
        // Zatrzymujemy tak szybko jak to możliwe
        timer_disable(mt->base_timer);

        // Resetujemy stan licznika
        mt->base_timer->CR1 |= TIM_CR1_URS;
        
        mt->base_timer->CCR1 =  mt->base_timer->ARR + 1;
        timer_update(mt->base_timer);

        mt->base_timer->CR1 &= ~TIM_CR1_URS;

        // Resetujemy stan tłumika
        mt->last_pressed = btn;
        mt->already_handled = false;
        mt->time_left = mt->long_press_cycles;

        // Resetujemy rejestr CCRx 
        // timer_set_ccr(mt->base_timer,
        //               mt->long_press_ccr,
        //               //mt->muffler_arr_mult * mt->muffle_time_cycles + 1);
        //               -1);
        // mt->base_timer->CCR1 = 0xfff;
        
        // Uruchamiamy z powrotem
        timer_enable(mt->base_timer);
    }
    // W przeciwnym przypadku nic nie robimy
}

#endif /* BUTTONS_H */ 