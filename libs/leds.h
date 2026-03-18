/**
 * @file leds.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl)
 * 
 * @brief Prosta biblioteka implementująca interfejs do kontroli diod LED
 *        dostępnych na płytce
 * 
 */
#ifndef LEDS_H
#define LEDS_H

#include <gpio.h>
#include <stm32.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Definicje wyjść
#define RED_LED_GPIO    GPIOA
#define GREEN_LED_GPIO  GPIOA
#define BLUE_LED_GPIO   GPIOB
#define GREEN2_LED_GPIO GPIOA

#define RED_LED_PIN     6
#define GREEN_LED_PIN   7
#define BLUE_LED_PIN    0
#define GREEN2_LED_PIN  5

#define ACTIVE_LOW  0
#define ACTIVE_HIGH 1

#endif /* DOXYGEN_SHOULD_SKIP_THIS */ 

/**
 * @struct led_info_struct
 * @brief  Struktura przechowująca podstawowe informacje o dostępnej diodzie LED
 * 
 * @var led_info_struct::GPIO
 *      @brief Układ GPIO obsługujący wyprowadzenie diody
 * 
 * @var led_info_struct::pin
 *      @brief Numer wyprowadzenia, do którego podłączona jest dioda
 * 
 * @var led_info_struct::activation
 *      @brief Stan wyjścia aktywujący diodę - wysoki (1) albo niski (0)
 */
typedef struct led_info_struct {
    GPIO_TypeDef *GPIO;
    uint8_t pin : 4;
    uint8_t activation : 1;
} led_info;


/// @brief Definicja parametrów diody czerwonej
const led_info RED_LED    = {RED_LED_GPIO,    RED_LED_PIN,    ACTIVE_LOW};
/// @brief Definicja parametrów diody zielonej
const led_info GREEN_LED  = {GREEN_LED_GPIO,  GREEN_LED_PIN,  ACTIVE_LOW};
/// @brief Definicja parametrów diody niebieskiej
const led_info BLUE_LED   = {BLUE_LED_GPIO,   BLUE_LED_PIN,   ACTIVE_LOW};
/// @brief Definicja parametrów małej diody zielonej
const led_info GREEN2_LED = {GREEN2_LED_GPIO, GREEN2_LED_PIN, ACTIVE_HIGH};

/// @brief Włącza wskazaną diodę
/// @param li   Struktura z informacjami o diodzie
inline void turn_led_on(led_info li) {
    
    if (li.activation == ACTIVE_HIGH)
        li.GPIO->BSRR = 1 << (li.pin);
    else
        li.GPIO->BSRR = 1 << (li.pin + 16);
}

/// @brief Wyłącza wskazaną diodę
/// @param li   Struktura z informacjami o diodzie
inline void turn_led_off(led_info li) {
    if (li.activation == ACTIVE_HIGH)
        li.GPIO->BSRR = 1 << (li.pin + 16);
    else
        li.GPIO->BSRR = 1 << (li.pin);
}

/// @brief Przełącza wskazaną diodę między stanem włączonym-wyłączonym
/// @param li   Struktura z informacjami o diodzie
inline void toogle_led(led_info li) {
    uint16_t reg = li.GPIO->IDR;
    if (li.activation == ACTIVE_HIGH){
        if (reg & li.pin)
            turn_led_off(li);
        else
            turn_led_on(li);
    }
    else {
        if (reg & li.pin)
            turn_led_on(li);
        else
            turn_led_off(li);
    }
}

/**
 * @brief Konfiguruje wyprowadzenie GPIO do obłsugi diody
 * 
 * @param li   Struktura z informacjami o diodzie 
 */
void configure_led(led_info li){
    turn_led_off(li);
    GPIOoutConfigure(li.GPIO,
                     li.pin,
                     GPIO_OType_PP,
                     GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL);
}


/**
 * @brief Włącza diodę i wyłącza ją po @p delay obrotach pętli
 *        złożonej z 2 instrukcji NOP
 * 
 * @param led   Struktura z informacjami o diodzie
 * @param delay Liczba obrotów do odczekania
 */
void blink_led(led_info led, uint32_t delay){
    turn_led_on(led);
    Delay(delay);
    turn_led_off(led);
}

#endif /* LEDS_H */