/**
 * @file ir_protocols.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl) indeks: 421329
 * 
 * 
 * @brief Biblioteka zawierająca implementację interfejsu dla protokołów IR
 *        realizowanych poprzez użycie dwóch liczników w trybie master-slave
 *        oraz kontrolera DMA 
 * 
 * 
 * 
 */
#ifndef IR_PROTOCOLS_H
#define IR_PROTOCOLS_H

#include <my_preprocessor.h>
#include <common.h>
#include <stdint.h>

#ifndef NO_STM
#include <dma.h>
#include <timers.h>
#endif

__attribute__ ((deprecated))
static const uint32_t __DONT_KNOW__ = (uint32_t)(0); 


/**
 * @typedef freq_template_t
 * @brief Podstawowa struktura opisująca fragment stanu wyjścia protokołu IR
 * 
 * Wykorzystywana do tworzenia opisu 'wiadomości' protokołu IR w postaci
 * ciągu struktur freq_template_t, zakończonego strukturą o polu @p .cycles
 * równym 0.
 * 
 * @attention Informacja o stanie 'wysokim' oznacza, że wyjście protokołu IR
 *            oscyluje z zadaną częstotliwością nośną.
 * 
 * @param .state    Stan wyjścia 0 lub 1
 * @param .cycles   Ile cykli częstotliowści nośnej stan ma być utrzymywany
 */
typedef struct __attribute__((__packed__)) freq_temp_struct {
    uint16_t state  : 1;
    uint16_t cycles : 15;
} freq_template_t;

/**
 * @brief Czy fragment wiadomości jest końcowym zerowym blokiem?
 * 
 * @param ft    Fragment do sprawdzenia
 */
static inline bool is_zero_freq_template(const freq_template_t ft){
    return (ft.cycles == 0);
}

typedef uint16_t code_t;
typedef uint16_t addr_t;

/**
 * @brief Enumerator najczęściej spotykanych kodów w protokołach IR
 */
typedef enum ir_codes_enum {
    CHANNEL_0,
    CHANNEL_1, CHANNEL_2, CHANNEL_3,
    CHANNEL_4, CHANNEL_5, CHANNEL_6,
    CHANNEL_7, CHANNEL_8, CHANNEL_9,
    STANDBY,
    PROGRAM_UP,PROGRAM_DOWN,
    VOLUME_UP, VOLUME_DOWN,
    POWER_ON, POWER_OFF
} IRCode;

/**
 * @brief Enumerator najczęściej spotykanych urządzeń w protokołach IR
 * 
 */
typedef enum ir_addr_enum {
    TV,
    VCR,
    SAT,
    CAMERA,
    TUNER,
    PROJECTOR
} IRAddr;

/**
 * @typedef message_func
 * @brief Typ generycznej funkcji tworzącej wiadomość
 * 
 * @param *state    Wskaźnik na stan protokołu
 * @param *buffer   Wskaźnik na **dostatecznie duży** bufor, do którego ma zostać
 *              załadowana wiadomość
 * @param code      Kod polecenia protokołu IR do przesłania w wiadomości
 * @param addr      Adres urządzenia protokołu IR do przesłania w wiadomości
 * 
 */
typedef void (*message_func)(uint32_t *state,
                             freq_template_t *buffer,
                             IRCode code, IRAddr addr);

/**
 * @typedef repeat_func
 * @brief Typ generycznej funkcji tworzącej powtórzenie wiadomości
 * 
 * @param *state    Wskaźnik na stan protokołu
 * @param *buffer   Wskaźnik na **dostatecznie duży** bufor, w którmy jest
 *              załadowana porzednia wiadomość i do którego należy załadować
 *              wiadomość do powtórzenia.
 * 
 */
typedef void (*repeat_func)(uint32_t *state, freq_template_t *buffer);

/**
 * @typedef state_func
 * @brief Typ generycznej funkcji operującej na stanie wewnętrznym protokołu IR
 * 
 * @param *state    Wskaźnik na stan protokołu
 * 
 */
typedef void (*state_func)(uint32_t *state);



/**
 * @struct ir_protocol_struct
 * @brief Struktura opisująca podstawowe parametry oraz interfejs protokołu IR
 */
typedef struct ir_protocol_struct {
    /// Częstotliwość nośna protokołu podana w Hz
    const uint32_t freq_hz;
    /// Czas oczekiwania w cyklach nośnych pomiędzy wysłaniem powtórzenia
    const uint32_t repeat_wait;   
    /// Czas oczekiwania w milisekundach nośnych pomiędzy wysłaniem powtórzenia
    const uint32_t repeat_wait_ms;    
    /// Minimalna wielkość bufora zdolnego pomieścić każdą wiadomość 
    const uint32_t minimal_buffer_size;

    /// Procedura inicjalizująca stan wewnętrzny protokołu
    state_func initialize_protocol_state;
    /// Procedura niszcząca stan wewnętrzny protokołu
    state_func destroy_protocol_state;

    /// @brief Procedura tworząca opis zadanej wiadomości w postaci ciągu opisu
    ///        stanu wyjścia protokołu (przy użyciu @ref freq_template_t)
    message_func create_message;

    /// Procedura tworząca opis wiadomości powtórzonej
    repeat_func create_repeat;
} IR_protocol;

// Zaślepka dla pustych procedur realizujących interfejs
static inline void __no_function() { return; } 

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Makra ograniczające powtarzalność kodu 
#define ptr_copy_move(ptr, mem, type)               \
    { memcopy((void*)ptr, (void*)mem, sizeof(mem)); \
      ptr += (sizeof(mem)/sizeof(type));            \
    }

#define freq_template_copy_move_ptr(ptr, mem)           \
    ({                                                   \
        memcopy((void*)(ptr), (void*)(mem), sizeof((mem))); \
        (ptr) += (sizeof((mem))/sizeof(freq_template_t));   \
    })

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

                    
#ifndef DOXYGEN_SHOULD_SKIP_THIS
/* ************************************************************************** **
            Protokół testowy na potrzeby debuggowania
** ************************************************************************** */

#define TP_0 {0, 1}, {0, 2} // ****__
#define TP_1 {1, 1}, {1, 2} // **____
void TProtocol_message(uint32_t *state, freq_template_t *buff,
                       IRCode c, IRAddr a){
    static const freq_template_t header[] = {TP_1, TP_1, TP_0, TP_0};
    static const freq_template_t channel[] = {TP_1, TP_1, TP_1};
    static const freq_template_t other[] = {TP_1, TP_0, TP_0};

    ptr_copy_move(buff, header, freq_template_t);

    switch (c){
        case CHANNEL_0 ... CHANNEL_9:
            ptr_copy_move(buff, channel, freq_template_t);
            break;
        default:
            ptr_copy_move(buff, other, freq_template_t);
            break;
    }
    
    memfill((void*)buff, 0, sizeof(freq_template_t));
}

const IR_protocol Test_Protocol = (const IR_protocol){
    .freq_hz                    = 2,
    .repeat_wait                = 8,
    .repeat_wait_ms             = 125,
    .minimal_buffer_size        = (8 + 6 + 1) * sizeof(freq_template_t),
    .initialize_protocol_state  = (state_func)__no_function,
    .destroy_protocol_state     = (state_func)__no_function,
    .create_message             = TProtocol_message,
    .create_repeat              = (repeat_func)__no_function
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */ 

/* ************************************************************************** **
                             RC-5 Protocol
** ************************************************************************** */
/// @brief Definicja transmisji bitu 0 w protokole RC5:
///        32 cykle stan wysoki, 32 cykle stan niski
#define RC5_0 {1, 32}, {0, 32}

/// @brief Definicja transmisji bitu 1 w protokole RC5:
///        32 cykle stan niski, 32 cykle stan wysoki
#define RC5_1 {0, 32}, {1, 32}

/**
 * @brief Tablica translacji enumerator kodu wiadomości na wartość liczbową
 *        do zrealizowania w wiadomości poprzez odpowiednią sekwencję bitów
 *        0 i 1.
 */
const uint8_t RC5_code_translation[] = {
    [CHANNEL_0]     = 0x0,
    [CHANNEL_1]     = 0x1,
    [CHANNEL_2]     = 0x2,
    [CHANNEL_3]     = 0x3,
    [CHANNEL_4]     = 0x4,
    [CHANNEL_5]     = 0x5,
    [CHANNEL_6]     = 0x6,
    [CHANNEL_7]     = 0x7,
    [CHANNEL_8]     = 0x8,
    [CHANNEL_9]     = 0x9,
    [STANDBY]       = 0xC,
    [PROGRAM_UP]    = 0x20,
    [PROGRAM_DOWN]  = 0x21,
    [VOLUME_UP]     = 0x10,
    [VOLUME_DOWN]   = 0x11
};

/**
 * @brief Tablica translacji enumeratora adresu urządzenia docelowego na wartość
 *        liczbową do zapisania w wiadomości.
 * 
 */
const uint8_t RC5_addr_translation[] = {
    [TV]        = 0x0,
    [VCR]       = 0x5,
    [SAT]       = 0x8,
    [CAMERA]    = 0x9,
    [TUNER]     = 0x11
};


/**
 * @brief Procedura realizująca ładowanie bitów zadanej wartości w postaci
 *        odpowiednich ramek @ref freq_template_t dla protokołu RC5
 * 
 * @param number    Wartość do załadowania
 * @param buffer    Wskaźnik do miejsca w buforze, gdzie należy załadować ramki
 *                  wskaźnik ten zostanie przesunięty o odpowiednią wartość    
 * @param max_bit   Maksymalna liczba bitów do sprawdzenia
 *                  (w przypadku RC5 to 5 dla adresu, 6 dla kodu)
 */
void RC5_encode_message(uint32_t number, freq_template_t **buffer, uint8_t max_bit){
    static const freq_template_t RC5_one[] = {RC5_1}, RC5_zero[] = {RC5_0};

    // Most Significat Bit
    uint32_t bit = max_bit - 1;
    for (int i = 0; i < max_bit; ++i){
        if (number & (1ul << bit))
            freq_template_copy_move_ptr(*buffer, RC5_one);
        else
            freq_template_copy_move_ptr(*buffer, RC5_zero);
        --bit;
    }
}

/**
 * @brief Procedura realizująca interfejs tworzenia wiadomości protokołu RC5
 *        zgodnie z opisem @ref message_func
 * 
 * @param state     Wskaźnik na stan wewnętrzny protokołu
 *                  (interpretowany jako bit toggle)
 */
void RC5_message(uint32_t *state, freq_template_t *buff, IRCode c, IRAddr a){
    static const freq_template_t header1[] = {RC5_1, RC5_1, RC5_1};
    static const freq_template_t header0[] = {RC5_1, RC5_1, RC5_0};

    if (*state == 1) {
        freq_template_copy_move_ptr(buff, header1);
    }
    else {
        freq_template_copy_move_ptr(buff, header0);
    }

    // Kopiujemy addres
    RC5_encode_message(RC5_addr_translation[a], &buff, 5);
    
    // Kopiujemy kod rozkazu
    RC5_encode_message(RC5_code_translation[c], &buff, 6);

    // Zerowy szablon oznacza koniec wiadomości
    memfill((void*)buff, 0, sizeof(freq_template_t));

    *state = (*state == 0 ? 1 : 0);
}

/**
 * @brief Procedura inicjalizująca stan protokołu RC5
 * 
 * @param state     Wskaźnik na stan wewnętrzny protokołu, interpretowany jako
 *                  bit toggle. Wartość wskaźnika jest zerowana.
 */
void RC5_state_init(uint32_t *state) { *state = 0; }

/**
 * @brief Definicja parametrów protokołu RC5
 * \n @p freq_hz - 36000 Hz
 * \n @p repeat_wait - 4104 cykli \f$\left(114ms \cdot 36000Hz\right)\f$
 * \n @p repeat_wait_ms - 114ms
 * \n @p minimal_buffer_size - 14 bitów po 2 ramki + ramka końcowa
 * \n @p initialize_protocol_state - @ref RC5_state_init
 * \n @p destroy_protocol_state - Pusta procedura
 * \n @p create_message  - @ref RC5_message
 * \n @p create_repeat   - Wiadomość nie ulega zmianie
 * 
 */
const IR_protocol RC5_protocol = {
    .freq_hz                    = 36000,
    .repeat_wait                = 4104, // 114ms * 36kHz
    .repeat_wait_ms             = 114,
    .minimal_buffer_size        = ((14*2 + 1) * sizeof(freq_template_t)),
    .initialize_protocol_state  = RC5_state_init,
    .destroy_protocol_state     = (state_func) __no_function,
    .create_message             = RC5_message,
    .create_repeat              = (repeat_func) __no_function,
};

/* ************************************************************************** **
                                NEC Protocol
** ************************************************************************** */
/*
Częstotliwość nośna to 38kHz (podobno najlepiej 38222 Hz) 
Wartości przy założeniu 38Khz:
    Logiczne zero:  562.5us stan wysoki, 562.5us stan niski
                    razem 1.125ms
    Logiczne jeden: 562.5us stan wysoki, 1687.5us stan niski
                    razem 2.25ms
38kHz --> 26.31578 us
 562.5us / 26.31578 us = 21.375 cykli
1.6875ms / 26.31578 us = 64.125 cykli
9ms aktywne na początku, 4.5ms stanu niskiego, razem 13.5ms
   9ms  / 26.31578 us = 342 cykli
 4.5ms  / 26.31578 us = 171 cykli
2.25ms  / 26.31578 us = 85.5 cykli
*/

/// @brief Definicja transmisji bitu 0 w protokole NEC
///        21 cykli stanu wysokiego, 21 cykli stanu niskiego
///        (zaokrąglenie z 21.375)
#define NEC_0 {1, 21}, {0, 21} 

/// @brief Definicja transmisji bitu 1 w protokole NEC
///        21 cykli stanu wysokiego, 64 cykle stanu niskiego
///        (zaokrąglenie z 21.375 i 64.125)
#define NEC_1 {1, 21}, {0, 64}

/// Definicja transmisji stanu wysokiego przez 9ms
#define NEC_9000us1 {1, 342}
/// Definicja transmisji stanu niskiego przez 4.5ms
#define NEC_4500us0 {0, 171}
/// Definicja transmisji stanu niskiego przez 2.25ms
#define NEC_2250us0 {0, 85}
/// Definicja końcowej transmisji stanu wysokiego przez 562.5us
#define NEC_EndBurst {1, 21}


/**
 * @brief Tablica translacji enumeratora kodu polecenia na wartość
 *        liczbową do zapisania w wiadomości dla protokołu NEC.
 */
static const uint16_t NEC_code_translation[] = {
    [POWER_ON] = 0x08,
    [POWER_OFF] = 0x14,
    [VOLUME_UP] = 0xA8,
    [VOLUME_DOWN] = 0xE0
};


/**
 * @brief Tablica translacji enumeratora adresu urządzenia docelowego na wartość
 *        liczbową do zapisania w wiadomości dla protokołu NEC.
 */
static const uint32_t NEC_addr_translation[] = {
    [TV] = 0x0,
    [PROJECTOR] = 0xE918,
};

/**
 * @brief Procedura realizująca ładowanie bitów zadanej wartości w postaci
 *        odpowiednich ramek @ref freq_template_t dla protokołu NEC
 * 
 * @param number    Wartość do załadowania
 * @param buffer  Wskaźnik do miejsca w buforze, gdzie należy załadować ramki
 *                  wskaźnik ten zostanie przesunięty o odpowiednią wartość    
 */
void NEC_encode_message(uint32_t number, freq_template_t **buffer){
    static const freq_template_t NEC_one[] = {NEC_1}, NEC_zero[] = {NEC_0};

    if (number > 0xff) { // Extended NEC Protocl
        // Least Significat Bit
        for (uint32_t bit = 0; bit < 16; ++bit){
            if (number & (1ul << bit))
                freq_template_copy_move_ptr(*buffer, NEC_one);
            else
                freq_template_copy_move_ptr(*buffer, NEC_zero);
        }
    }
    else {
        // Least Significat Bit
        for (uint32_t bit = 0; bit < 8; ++bit){
            if (number & (1ul << bit))
                freq_template_copy_move_ptr(*buffer, NEC_one);
            else
                freq_template_copy_move_ptr(*buffer, NEC_zero);
        }

        // Odwrotność
        for (uint32_t bit = 0; bit < 8; ++bit){
            if (number & (1ul << bit))
                freq_template_copy_move_ptr(*buffer, NEC_zero);
            else
                freq_template_copy_move_ptr(*buffer, NEC_one);
        }
    }
}


/**
 * @brief Procedura realizująca interfejs tworzenia wiadomości protokołu NEC
 *        zgodnie z opisem @ref message_func
 */
void NEC_message(uint32_t *state, freq_template_t *buffer, IRCode c, IRAddr a){
    static const freq_template_t header[] = {NEC_9000us1, NEC_4500us0};
    static const freq_template_t ending[] = {NEC_EndBurst};
    (void)state;

    freq_template_copy_move_ptr(buffer, header);

    NEC_encode_message(NEC_addr_translation[a], &buffer);
    NEC_encode_message(NEC_code_translation[c], &buffer);

    freq_template_copy_move_ptr(buffer, ending);
    
    memfill((void*)buffer, 0, sizeof(freq_template_t));
}


/**
 * @brief Procedura realizująca interfejs powtórzenia wiadomości protokołu NEC
 *        zgodnie z opisem @ref message_func
 */
void NEC_repeat(uint32_t *state, freq_template_t *buffer){
    static const freq_template_t repeat[] = {
        NEC_9000us1, NEC_2250us0 , NEC_EndBurst
    };
    (void)state;

    freq_template_copy_move_ptr(buffer, repeat);
    memfill((void*)buffer, 0, sizeof(freq_template_t));
}

/**
 * @brief Definicja parametrów protokołu NEC
 * \n @p freq_hz - 38000 Hz
 * \n @p repeat_wait - 4104 cykli \f$\left(108ms \cdot 38000Hz\right)\f$
 * \n @p repeat_wait_ms - 108ms
 * \n @p minimal_buffer_size - 32 bity po 2 ramki + 3 stany specjalne po 2 ramki
 *                           \+ ramka końcowa
 * \n @p initialize_protocol_state - Pusta procedura
 * \n @p destroy_protocol_state - Pusta procedura
 * \n @p create_message  - @ref NEC_message
 * \n @p create_repeat   - @ref NEC_repeat
 */
const IR_protocol NEC_protocol = {
    .freq_hz                    =  38000,
    .repeat_wait                = 4104,
    .repeat_wait_ms             = 108,
    .minimal_buffer_size        = (32*2 + 3*2 + 1) * sizeof(freq_template_t),
    .initialize_protocol_state  = (state_func)__no_function,
    .destroy_protocol_state     = (state_func)__no_function,
    .create_message             = NEC_message,
    .create_repeat              = NEC_repeat
};


/* ************************************************************************** **
                            MESSAGE UTILITIES
** ************************************************************************** */

/**
 * @brief Procedura kompaktująca wiadomość przekazaną w @p *message
 * 
 * Sąsiednie ramki o tym samym stanie są zamieniane na pojedynczą ramkę o
 * odpowiedniej liczbie cykli. Usuwane są wiodące ramki o stanie zerowym.
 * 
 * @param message   Wskaźnik na tablicę ramek wiadomości
 */
void IR_compact_message(freq_template_t *message){
    if (!message)
        return;
    
    freq_template_t *ptr = message, *next = ptr;
    /* Przejście wskaźnikiem next do pierwszego fragmentu o stanie 1 */
    while (!is_zero_freq_template(*next) && next->state == 0)
        next++;

    *ptr = *next;
    /* Konsolidacja fragmentów o tym samym stanie */
    while (!is_zero_freq_template(*next)){
        next++;
        while (ptr->state == next->state && !is_zero_freq_template(*next)){
            ptr->cycles += next->cycles;
            next++;
        }
        ptr++;
        *ptr = *next;
    }
}

/**
 * @brief Generuje kolejkę wartości rejestrów ARR i CCRx tak, że po
 *        przesłaniu tych wartości do rejestrów licznkia, wiadomość zakodowana w
 *        @p *msg zostanie poprawnie wykonana przez wybrany licznik.
 *        Wartości rejestrów są 32 bitowe.  
 * 
 * Bufory można następnie przekazać do kontrolera DMA w celu zautomatyzowania
 * procesu przekazywania wartości licznikowi.
 * 
 * @note Patrz @ref IR_compact_setup_buffers_16 dla 16 bitowych buforów.
 * @attention Wiadomość w @p *msg musi zostać wcześniej poddana skompaktowaniu
 * 
 * @return uint32_t - liczba wartości w każdym z buforów do przesłania 
 */
uint32_t IR_compact_setup_buffers_32(const freq_template_t *msg,
                                  uint32_t multiplier,
                                  uint32_t *arr_buffer, uint32_t *ccr_buffer){
    __label__ load_endvalues;   // Deklaracja lokalnej etykiety 'load_endvalues'
    uint32_t index = 0;


    if (!msg || is_zero_freq_template(*msg))
        goto load_endvalues;

    const freq_template_t *ptr = msg;

    while(!is_zero_freq_template(*ptr) && !is_zero_freq_template(*(ptr + 1))){
        /* Konsolidacja => ptr->state == 1, next->state == 0
            => ARR  = ptr->cycles + next->cycles - 1
            => CCR_ = ptr->cycles
        */
        arr_buffer[index] = multiplier * (ptr->cycles + (ptr + 1)->cycles) - 1;
        ccr_buffer[index] = multiplier * (ptr->cycles);
        ptr += 2; // next block (1 przez X, 0 przez Y)
        index++;
    }
    // Ostatni blok nieparzystych wiadomości
    // Tutaj wymuszamy Y > 0, aby zainicjować ostatni CCR Event
    if (!is_zero_freq_template(*ptr)) {
        arr_buffer[index] = multiplier * (ptr->cycles) + 1;
        ccr_buffer[index] = multiplier * (ptr->cycles);
        index++;
    }

    load_endvalues: /* Ładowanie wartości wyłączających nadawanie */
    arr_buffer[index] = 1;     //<- Zliczaj niewiele
    ccr_buffer[index] = 0;     //<- Nie emituj żadnego sygnału

    return index + 1;
}

/**
 * @brief Analogicznie jak w przypadku @ref IR_compact_setup_buffer_32 z tym, że
 *        generowane wartości są 16 bitowe.
 * 
 * @note Patrz @ref IR_compact_setup_buffers_32 dla 32 bitowych buforów.
 * @attention Wiadomość w @p *msg musi zostać wcześniej poddana skompaktowaniu
 */
uint32_t IR_compact_setup_buffers_16(const freq_template_t *msg,
                                  uint32_t multiplier,
                                  uint16_t *arr_buffer, uint16_t *ccr_buffer){
    __label__ load_endvalues;   // Deklaracja lokalnego 'labela'
    uint32_t index = 0;


    if (!msg || is_zero_freq_template(*msg))
        goto load_endvalues;

    const freq_template_t *ptr = msg;

    while(!is_zero_freq_template(*ptr) && !is_zero_freq_template(*(ptr + 1))){
        /* Konsolidacja => ptr->state == 1, next->state == 0
            => ARR  = ptr->cycles + next->cycles - 1
            => CCR_ = ptr->cycles
        */
        arr_buffer[index] = multiplier * (ptr->cycles + (ptr + 1)->cycles) - 1;
        ccr_buffer[index] = multiplier * (ptr->cycles);
        ptr += 2; // next block (1 przez X, 0 przez Y)
        index++;
    }
    // Ostatni blok nieparzystych wiadomości
    // Tutaj wymuszamy Y > 0, aby zainicjować ostatni CCR Event
    if (!is_zero_freq_template(*ptr)) {
        arr_buffer[index] = multiplier * (ptr->cycles) + 1;
        ccr_buffer[index] = multiplier * (ptr->cycles);
        index++;
    }

    load_endvalues: /* Ładowanie wartości wyłączających nadawanie */
    arr_buffer[index] = 1;     //<- Zliczaj niewiele
    ccr_buffer[index] = 0;     //<- Nie emituj żadnego sygnału

    return index + 1;
}


/* ************************************************************************** **
                            TIMERS SETUP
** ************************************************************************** */

#ifndef NO_STM
/**
 * @brief Procedura konfigurująca licznik nadrzędny @p *bit_timer (master), tak
 *        aby przy odpowiednim ustawieniu rejestrów ARR i CCRx 
 *        (x = @p bit_line )
 *        generował odpowiednie przebiegi syngału aktywującego licznik 
 *        podrzędny, adekwatnie dla zadanego protokołu @p proto
 * 
 * @param bit_timer     Wskaźnik na licznik nadrzędny
 * @param bit_line      Numer wyjścia lini aktywującej licznik podrzędny
 * @param bit_active_low    Czy linia ma być aktywna stanem niskim?
 * @param freq_timer    Wskaźnik na licznik podrzędny (odp. za częst. nośną)
 * @param proto         Protokół IR, definujący konfigurowane parametry
 * @return uint32_t     Wartość mnożnika liczby cykli, by uzyskać żądany czas
 *                      przebiegu
 * 
 * @note Po wykonaniu tej procedury i @ref IR_setup_freq_timer chcąc nadać
 *       syngał 1 przez X cykli a potem 0 przez Y cykli wystarczy ustawić:
 *          + ARR := (X + Y) * mnożnik - 1
 *          + CCR @p bit_line := X * mnożnik
 * 
 * @note Sytuacja gdzie Y == 0 powinna występować wtt. gdy to koniec wiadomości
 *       Możemy wtedy ustawić Y > 0, tak aby doszło do zdarzenia CC           \n
 *       Nową wartość ARR można ładować przy Update Event                     \n
 *       Nową wartość CCR##bit_line można ładować przy Capture Compare Event  \n
 *       Jeżeli bit_line == 1 można spróbować użyć rejestru TIMx->DMAR        \n
 *                                                                            \n
 *       Przy wykryciu zdarzenia DMA Transfer Complete możemy ustawić licznik
 *       w tryb One Pulse Mode, tak aby zakończył swoją pracę od razu po
 *       ostatnim Update Event
 */
uint32_t IR_setup_bit_timer(const timer_info_t *bit_timer,
                            uint8_t bit_line, bool bit_active_low,
                            const timer_info_t *freq_timer,
                            const IR_protocol proto)
{

    uint32_t resulting_psc;
    uint32_t multiplier;
    uint32_t arr_bit_width = bit_timer->other_bit_width;
    uint32_t psc_bit_width = bit_timer->psc_bit_width;

    uint32_t ret = get_psc_mult(bit_timer->clock_freq_hz, proto.freq_hz,
                                &resulting_psc, psc_bit_width,
                                &multiplier,    arr_bit_width, 1);

    if (ret != 0)
        return -1;

    TIM_TypeDef *tim_bit = bit_timer->tim, *tim_freq = freq_timer->tim;
    timer_disable(tim_bit);

    // Konfiguracja licznika nadrzędnego
    timer_set_direction(tim_bit, Count_Up);
    /* częstotliwość zliczania multiplier * proto.freq_hz */
    tim_bit->PSC = resulting_psc - 1;
    tim_bit->ARR = multiplier * 2 - 1;
    timer_set_ccr(tim_bit, bit_line, 0);        // Wymuszamy 0 na wyjściu

    timer_update(tim_bit);
    tim_bit->CCMR1 = 0;
    tim_bit->CCMR2 = 0;

    timer_set_arr_preload(tim_bit);
    
    timer_set_ccr_preload(tim_bit, bit_line);
    timer_set_line_output_mode(tim_bit, bit_line, PWM1);
    timer_enable_output_line_compare(tim_bit, bit_line, bit_active_low);
    
    // Łączymy liczniki w tryb master - gated slave
    timer_setup_master_enable_slave(tim_bit, tim_freq, bit_line);

    timer_enable_update_interrupt(tim_bit);
    // Włączamy przerwanie dla CCR##bit_line, w celu zapewniena
    // wyłączenia licznika częstotliwości nośnej
    timer_enable_cc_interrupt(tim_bit, bit_line);
    NVIC_EnableIRQ(bit_timer->update_interrupt_number);

    return multiplier;
}



/**
 * @brief Procedura konfigurująca parametry licznika podrzędnego (slave),
 *        wyubijającego częstotliwość nośną na lini @p freq_line
 * 
 * @param freq_timer    Wskaźnik na licznik podrzędny
 * @param freq_line     Numer linii wyjściowej licznika
 * @param freq_active_low Czy linia jest aktywna stanem niskim?
 * @param proto         Protokół IR, definujący konfigurowane parametry
 * @return 0    Jeżeli poprawnie udało się skonfigurować licznik
 * @return <>0 Jeżeli wystąpił błąd przy konfiguracji licznika
 * @retval -1   Licznik nie jest wstanie zreprezentować zadanej częstotliwości
 *              (rejestry ARR i PSC są za małe) 
 */
int IR_setup_freq_timer(const timer_info_t *freq_timer,
                         uint8_t freq_line, bool freq_active_low,
                         const IR_protocol proto){
    uint32_t resulting_psc;
    uint32_t multiplier;
    uint32_t arr_bit_width = freq_timer->other_bit_width;
    uint32_t psc_bit_width = freq_timer->psc_bit_width;

    uint32_t ret = get_psc_mult(freq_timer->clock_freq_hz, proto.freq_hz,
                                &resulting_psc, psc_bit_width,
                                &multiplier, arr_bit_width, 2);
    
    if (ret != 0)
        return -1;
    
    TIM_TypeDef *tim_freq = freq_timer->tim;
    timer_disable(tim_freq);

    timer_set_direction(tim_freq, Count_Up);
    timer_set_counting(tim_freq, resulting_psc - 1, multiplier - 1);

    // Gwarantowana podzielność mnożnika na 2 => wypełnienie 50%
    timer_set_ccr(tim_freq, freq_line, multiplier / 2);

    timer_update(tim_freq);

    tim_freq->CCMR1 = 0;
    tim_freq->CCMR2 = 0;

    timer_set_ccr_preload(tim_freq, freq_line);
    timer_set_line_output_mode(tim_freq, freq_line, PWM2);
    timer_enable_output_line_compare(tim_freq, freq_line, freq_active_low);

    return 0;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

static inline void __IR_setup_XXX_transfer(DMA_Stream_TypeDef *stream, uint8_t chnl,
                             void *buffer, TIM_TypeDef *tim,
                             uint32_t target,uint32_t flags){
    stream->CR = flags;
    stream->M0AR = (uint32_t)buffer;
    DMAStream_set_periph(stream, target);
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @brief Procedura konfigurująca transfer bufora wartości ARR do licznika
 *        @p *tim poprzez wskazany strumień i kanał kontrolera DMA
 * 
 * @param arr_stream   Wskaźnik na strumień kontrolera DMA
 * @param arr_chnl      Numer kanału strumienia 
 * @param arr_buffer   Bufor zawierający wartości ARR przygotowane przez
 *                      @ref IR_compact_setup_buffers_32
 * @param tim          Wskaźnik na licznik nadrzędny skonfigurowany wcześniej
 *                      przez @ref IR_setup_bit_timer
 * 
 * @note Patrz @ref IR_setup_arr_transfer_16 dla 16 bitowych wartości oraz
 *       @ref IR_setup_ccr_transfer_32 dla konfiguracji strumienia wartości CCR
 */
void IR_setup_arr_transfer_32(DMA_Stream_TypeDef *arr_stream, uint8_t arr_chnl,
                              uint32_t *arr_buffer, TIM_TypeDef *tim){
    uint32_t cr_flags = DMA_CHANNEL_BIT(arr_chnl)  |
                        DMA_MEMORY_2_PERIHP        |
                        DMA_INC_MEMORY             |
                        DMA_MEM_DATA_WIDTH_32      |
                        DMA_PER_DATA_WIDTH_32      |
                        DMA_TRANS_COMPL_INTERRUPT  ;


    __IR_setup_XXX_transfer(arr_stream, arr_chnl, arr_buffer, tim,
                            (uint32_t)&(tim->ARR), cr_flags);
    timer_enable_update_dma_request(tim);
}

/**
 * @brief Analogiczna procedura jak @ref IR_setup_arr_transfer_32 z tym, że dla
 *        wartości 16 bitowych.
 * 
 * @note Patrz @ref IR_setup_arr_transfer_32 dla 32 bitowych wartości oraz
 *       @ref IR_setup_ccr_transfer_16 dla konfiguracji strumienia wartości CCR
 */
void IR_setup_arr_transfer_16(DMA_Stream_TypeDef *arr_stream, uint8_t arr_chnl,
                              uint16_t *arr_buffer, TIM_TypeDef *tim){
    uint32_t cr_flags = DMA_CHANNEL_BIT(arr_chnl)  |
                        DMA_MEMORY_2_PERIHP        |
                        DMA_INC_MEMORY             |
                        DMA_MEM_DATA_WIDTH_16      |
                        DMA_PER_DATA_WIDTH_16      |
                        DMA_TRANS_COMPL_INTERRUPT  ;


    __IR_setup_XXX_transfer(arr_stream, arr_chnl, arr_buffer, tim,
                            (uint32_t)&(tim->ARR), cr_flags);
    timer_enable_update_dma_request(tim);
}


/**
 * @brief Procedura konfigurująca transfer bufora wartości CCR do licznika
 *        @p *tim poprzez wskazany strumień i kanał kontrolera DMA
 * 
 * @param *ccr_stream   Wskaźnik na strumień kontrolera DMA
 * @param ccr_chnl      Numer kanału strumienia 
 * @param *ccr_buffer   Bufor zawierający wartości CCR przygotowane przez
 *                      @ref IR_compact_setup_buffers_32
 * @param *tim          Wskaźnik na licznik nadrzędny skonfigurowany wcześniej
 *                      przez @ref IR_setup_bit_timer
 * @param ccr           Numer linii aktywującej
 * 
 * @note Patrz @ref IR_setup_ccr_transfer_16 dla 16 bitowych wartości oraz
 *       @ref IR_setup_arr_transfer_32 dla konfiguracji strumienia wartości ARR
 */
void IR_setup_ccr_transfer_32(DMA_Stream_TypeDef *ccr_stream, uint8_t ccr_chnl,
                              uint32_t *ccr_buffer, TIM_TypeDef *tim,
                              uint8_t ccr){
    uint32_t cr_flags = DMA_CHANNEL_BIT(ccr_chnl)  |
                        DMA_MEMORY_2_PERIHP        |
                        DMA_INC_MEMORY             |
                        DMA_MEM_DATA_WIDTH_32      |
                        DMA_PER_DATA_WIDTH_32      |
                        DMA_TRANS_COMPL_INTERRUPT  ;

    #define func(x)                                                 \
    __IR_setup_XXX_transfer(ccr_stream, ccr_chnl, ccr_buffer, tim,  \
                            (uint32_t)&(tim->CCR##x), cr_flags);    
    CONST_SWITCH(4, ccr, func);
    #undef func
    timer_enable_dma_request(tim, ccr);
}

/**
 * @brief Analogiczna procedura jak @ref IR_setup_ccr_transfer_32 z tym, że dla
 *        wartości 16 bitowych.
 * 
 * @note Patrz @ref IR_setup_ccr_transfer_32 dla 32 bitowych wartości oraz
 *       @ref IR_setup_arr_transfer_16 dla konfiguracji strumienia wartości ARR
 */
void IR_setup_ccr_transfer_16(DMA_Stream_TypeDef *ccr_stream, uint8_t ccr_chnl,
                              uint16_t *ccr_buffer, TIM_TypeDef *tim,
                              uint8_t ccr){
    uint32_t cr_flags = DMA_CHANNEL_BIT(ccr_chnl)  |
                        DMA_MEMORY_2_PERIHP        |
                        DMA_INC_MEMORY             |
                        DMA_MEM_DATA_WIDTH_16      |
                        DMA_PER_DATA_WIDTH_16      |
                        DMA_TRANS_COMPL_INTERRUPT  ;

    #define func(x)                                                 \
    __IR_setup_XXX_transfer(ccr_stream, ccr_chnl, ccr_buffer, tim,  \
                            (uint32_t)&(tim->CCR##x), cr_flags);    
    CONST_SWITCH(4, ccr, func);
    #undef func
    timer_enable_dma_request(tim, ccr);
}

/* ************************************************************************** **
                        Wysokopoziomowy interfejs
** ************************************************************************** */

typedef struct IR_controller_struct {
    timer_info_t bit_timer, freq_timer;
    uint8_t bit_line, bit_line_active_low : 1;
    uint8_t freq_line, freq_line_active_low : 1;
    dma_info dma;
    uint8_t dma_arr_stream, dma_ccr_stream;
    uint8_t dma_arr_channel, dma_ccr_channel;
    volatile uint8_t dma_transfer_end_status;

    freq_template_t *message_buffer;
    void *transfer_buffers[2];
    uint32_t bit_timer_multiplier;

    uint32_t protocol_state;
    IR_protocol protocol;

} IR_controller_t;

#define TRANSFER_ONGOING    0
#define TRANSFER_END_SEQ_START  1
#define TRANSFER_END_SEQ_1  2
#define TRANSFER_END_SEQ_2  3
#define TRANSFER_COMPLETE   4
/**
 * @brief Procedura obsługi przerwania zdarzenia uaktualnienia dla licznika
 *        nadrzędnego.
 * 
 * @param *controller       Wskaźnik na strukturę zarządzającą protokołem IR
 * 
 * @param update_callback   Procedura do wywołania podczas obsługi przerwania
 *                          zdarzenia uaktualnienia
 * 
 * @param capture_compare_callback Procedura do wywołania podczas obsługi
 *                                 przerwania zdarzenia zgodności.
 */
void IR_bit_timer_transfer_IRQHandler(IR_controller_t *controller,
                                      void (*update_callback)(void),
                                      void (*capture_compare_callback)(void))
{
    TIM_TypeDef *bit_tim = controller->bit_timer.tim;
    TIM_TypeDef *freq_tim = controller->freq_timer.tim;

    if (timer_check_update_interrupt_flag(bit_tim)){

        timer_clear_update_interrupt_flag(bit_tim);

        if (update_callback)
            update_callback();

        switch (controller->dma_transfer_end_status) {
            case TRANSFER_ONGOING:  // Transfer z DMA trwa
                break;
            
            case TRANSFER_END_SEQ_START:    // Transfer z DMA zakończył się
                controller->dma_transfer_end_status = TRANSFER_END_SEQ_1;
                break;
            
            case TRANSFER_END_SEQ_1: { // Ostatnia para wartości do obsłużenia
                // Jednocześnie, korzystając z preloadu, ładujemy 
                // profilaktyczne 0, aby zawsze (niezależnie od wartości z DMA)
                // wyłączyć linię aktywującą licznik frekwencji
                timer_set_ccr(bit_tim, controller->bit_line, 0);

                controller->dma_transfer_end_status = TRANSFER_END_SEQ_2;
            }
            break;
            
            case TRANSFER_END_SEQ_2: {  // Efektywny koniec transferu
                timer_disable(controller->bit_timer.tim);
                timer_update(controller->freq_timer.tim);
                controller->dma_transfer_end_status = TRANSFER_COMPLETE;
            }        
            break;

            case TRANSFER_COMPLETE: {
                timer_disable(controller->bit_timer.tim);
                break;
            }
        }
    }   

    if (timer_check_cc_interrupt_flag(bit_tim, controller->bit_line)){
        timer_clear_cc_interrupt_flag(bit_tim, controller->bit_line);

        if (capture_compare_callback)
            capture_compare_callback();

        timer_update(freq_tim);    // Wymuszamy 0 na lini wyjściowej licznika
                                    // częstotliwości nośnej
    } 
}


void IR_DMAStream_arr_transfer_IRQHandler
(
    IR_controller_t *controller,
    void (*transfer_complete_callback)(void)
)
{   
    dma_info dma = controller->dma;
    uint8_t stream = controller->dma_arr_stream;
    
    uint32_t isr = DMA_read_interrupts(dma, stream);

    if (DMA_TCI_check(isr, stream)){
        DMA_TCI_clear(dma, stream);

        if (transfer_complete_callback)
            transfer_complete_callback();

        controller->dma_transfer_end_status = TRANSFER_END_SEQ_START;
    }
}

void IR_DMAStream_ccr_transfer_IRQHandler
(
    IR_controller_t *controller,
    void (*transfer_complete_callback)(void)
)
{
    dma_info dma = controller->dma;
    uint8_t stream = controller->dma_ccr_stream;

    uint32_t isr = DMA_read_interrupts(dma,stream);

    if (DMA_TCI_check(isr, stream)){
        DMA_TCI_clear(dma, stream);

        if (transfer_complete_callback)
            transfer_complete_callback();

    }
}


int IR_setup_controller(IR_controller_t *controller){

    int freq_setup_ret = IR_setup_freq_timer(&controller->freq_timer,
                                             controller->freq_line,
                                             controller->freq_line_active_low,
                                             controller->protocol);

    uint32_t bit_setup_ret = IR_setup_bit_timer(&controller->bit_timer,
                                                controller->bit_line,
                                                controller->bit_line_active_low,
                                                &controller->freq_timer,
                                                controller->protocol);
    
    if (freq_setup_ret != 0 || bit_setup_ret == -1)
        return -1;
    
    uint8_t other_bit_width = controller->bit_timer.other_bit_width;
    dma_info dma = controller->dma;

    switch (other_bit_width) {
        case 32: {
            IR_setup_arr_transfer_32(
                dma.stream[controller->dma_arr_stream],
                controller->dma_arr_channel,
                (uint32_t *)(controller->transfer_buffers[0]),
                controller->bit_timer.tim
            );
            
            IR_setup_ccr_transfer_32(
                dma.stream[controller->dma_ccr_stream],
                controller->dma_ccr_channel,
                (uint32_t *)(controller->transfer_buffers[1]),
                controller->bit_timer.tim,
                controller->bit_line
            );
            break;
        }
        case 16: {
            IR_setup_arr_transfer_16(
                dma.stream[controller->dma_arr_stream],
                controller->dma_arr_channel,
                (uint16_t *)(controller->transfer_buffers[0]),
                controller->bit_timer.tim
            );
            
            IR_setup_ccr_transfer_16(
                dma.stream[controller->dma_ccr_stream],
                controller->dma_ccr_channel,
                (uint16_t *)(controller->transfer_buffers[1]),
                controller->bit_timer.tim,
                controller->bit_line
            );
            break;
        }
        default:
            return -1;
    }

    controller->bit_timer_multiplier = bit_setup_ret;
    controller->protocol.initialize_protocol_state(&controller->protocol_state);
    
    return 0;
}

#define IR_SEND_MESSAGE_ERROR -1
#define IR_SEND_MESSAGE_NREADY 1
#define IR_SEND_MESSAGE_OK 0

static int IR_send_inner_message(IR_controller_t *controller){
    uint32_t count = 0;
    dma_info dma = controller->dma;

    switch (controller->bit_timer.other_bit_width) {
        case 32: {
            uint32_t *arr_buffer = controller->transfer_buffers[0];
            uint32_t *ccr_buffer = controller->transfer_buffers[1];

            // Ustawiamy odpowiednie wartości w buforach, które będą
            // przesyłane do rejestrów ARR oraz CCRx
            count = IR_compact_setup_buffers_32(controller->message_buffer,
                                                controller->bit_timer_multiplier,
                                                arr_buffer,
                                                ccr_buffer);

            if (count == 0)
                return IR_SEND_MESSAGE_ERROR;
            
            // Ustawiamy parametry DMA
            DMAStream_set_send(dma.stream[controller->dma_arr_stream],
                               (uint32_t)(arr_buffer + 1),
                               count - 1);
            
            DMAStream_set_send(dma.stream[controller->dma_ccr_stream],
                               (uint32_t)(ccr_buffer + 1),
                               count - 1);

            // Ustawiwamy początkowe wartości licznika ręcznie
            controller->bit_timer.tim->ARR = arr_buffer[0];
            timer_set_ccr(controller->bit_timer.tim,
                          controller->bit_line,
                          ccr_buffer[0]);

            break;
        }
        case 16: {
            // Analogiczna wersja 16 bitowa
            uint16_t *arr_buffer = controller->transfer_buffers[0];
            uint16_t *ccr_buffer = controller->transfer_buffers[1];
            count = IR_compact_setup_buffers_16(controller->message_buffer,
                                                controller->bit_timer_multiplier,
                                                arr_buffer,
                                                ccr_buffer);

            if (count == 0)
                return IR_SEND_MESSAGE_ERROR;
            
            DMAStream_set_send(dma.stream[controller->dma_arr_stream],
                               (uint32_t)(arr_buffer + 1),
                               count - 1);
            
            DMAStream_set_send(dma.stream[controller->dma_ccr_stream],
                               (uint32_t)(ccr_buffer + 1),
                               count - 1);

            controller->bit_timer.tim->ARR = arr_buffer[0];
            timer_set_ccr(controller->bit_timer.tim,
                          controller->bit_line,
                          ccr_buffer[0]);
            break;
        }
        default:
            return IR_SEND_MESSAGE_ERROR;
    }

    // Zerujemy status One Pulse Mode
    timer_disable_one_pulse(controller->bit_timer.tim);
    // Aktualizujemy licznik, aby załadować początkowe wartości
    timer_update(controller->bit_timer.tim);
    
    // Ustawiamy domyślny status końca transferu dma
    controller->dma_transfer_end_status = TRANSFER_ONGOING;

    // Uruchamiamy strumienie DMA z kolejnymi wartościami dla ARR i CCRx
    // Należy pamiętać o preload obu rejestrów.
    DMAStream_enable(dma.stream[controller->dma_ccr_stream]);
    DMAStream_enable(dma.stream[controller->dma_arr_stream]);

    // Uruchamiamy licznik nadrzędny
    timer_enable(controller->bit_timer.tim);

    return IR_SEND_MESSAGE_OK;
}

int IR_send_repeat(IR_controller_t *controller){
    if (controller->dma_transfer_end_status != TRANSFER_COMPLETE) {
        return IR_SEND_MESSAGE_NREADY;
    }

    controller->protocol.create_repeat(&controller->protocol_state,
                                       controller->message_buffer);
    
    IR_compact_message(controller->message_buffer);

    return IR_send_inner_message(controller);
}


int IR_send_message(IR_controller_t *controller, code_t code, addr_t addr){

    if (controller->dma_transfer_end_status != TRANSFER_COMPLETE) {
        return IR_SEND_MESSAGE_NREADY;
    }

    controller->protocol.create_message(&controller->protocol_state,
                                        controller->message_buffer,
                                        code,
                                        addr);

    IR_compact_message(controller->message_buffer);

    return IR_send_inner_message(controller);      
}

#endif /* NO_STM */


#endif /* IR_PROTOCOLS_H */