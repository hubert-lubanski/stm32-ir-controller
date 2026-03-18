/**
 * @file dma.h
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl)
 * 
 * @brief   Prosta bibliotek implementująca delikatnie wyższy interfejs dla
 *          zarządzania dostępnymi kontrolerami DMA.
 * 
 * 
 */
#ifndef DMA_H
#define DMA_H


#include <stdint.h>
#include <stm32.h>
#include <stdbool.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define DMA_CHANNEL_BIT(ch) ((uint8_t)(ch) << DMA_SxCR_CHSEL_Pos)

#define DMA_LOW_PRIO        ((0ul) << DMA_SxCR_PL_Pos)
#define DMA_MEDIUM_PRIO     ((1ul) << DMA_SxCR_PL_Pos)
#define DMA_HIGH_PRIO       ((2ul) << DMA_SxCR_PL_Pos)
#define DMA_VERY_HIGH_PRIO  ((3ul) << DMA_SxCR_PL_Pos)

#define DMA_INC_MEMORY  DMA_SxCR_MINC
#define DMA_INC_PERIPH  DMA_SxCR_PINC

#define DMA_PERIPH_2_MEMORY ((0ul) <<  DMA_SxCR_DIR_Pos)
#define DMA_MEMORY_2_PERIHP ((1ul) <<  DMA_SxCR_DIR_Pos)
#define DMA_MEMORY_2_MEMORY ((2ul) <<  DMA_SxCR_DIR_Pos)

#define DMA_MEM_DATA_WIDTH_8    ((0ul) << DMA_SxCR_MSIZE_Pos)
#define DMA_MEM_DATA_WIDTH_16   ((1ul) << DMA_SxCR_MSIZE_Pos)
#define DMA_MEM_DATA_WIDTH_32   ((2ul) << DMA_SxCR_MSIZE_Pos)

#define DMA_PER_DATA_WIDTH_8    ((0ul) << DMA_SxCR_PSIZE_Pos)
#define DMA_PER_DATA_WIDTH_16   ((1ul) << DMA_SxCR_PSIZE_Pos)
#define DMA_PER_DATA_WIDTH_32   ((2ul) << DMA_SxCR_PSIZE_Pos)

#define DMA_TRANS_COMPL_INTERRUPT DMA_SxCR_TCIE

#endif /* DOXYGEN_SHOULD_SKIP_THIS */ 

/**
 * @struct dma_info_struct
 * 
 * @brief Struktura przechowująca podstawowe dane o kontrolerze DMA
 * 
 * @var dma_info_struct::dma
 *      @brief Wskaźnik na kontroler DMA
 * 
 * @var dma_info_struct::stream
 *      @brief Tablica wskaźników na kolejne strumienie DMA
 * 
 * @var dma_info_struct::enable_clock
 *      @brief Wskaźnik na funkcję włączająca taktowanie tego podzespołu
 */
typedef struct dma_info_struct {
    DMA_TypeDef *dma;
    DMA_Stream_TypeDef *stream[8];
    void (*enable_clock)(void);
} dma_info;


#ifndef DOXYGEN_SHOULD_SKIP_THIS
void enable_dma_1(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
}

void enable_dma_2(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */ 

/// @brief Definicja paramterów kontrolera DMA1
const dma_info DMA1_INFO = {
    DMA1,
    { DMA1_Stream0, DMA1_Stream1, DMA1_Stream2,
      DMA1_Stream3, DMA1_Stream4, DMA1_Stream5,
      DMA1_Stream6, DMA1_Stream7 },
    enable_dma_1
};
/// @brief Definicja paramterów kontrolera DMA2
const dma_info DMA2_INFO = {
    DMA2,
    { DMA2_Stream0, DMA2_Stream1, DMA2_Stream2,
      DMA2_Stream3, DMA2_Stream4, DMA2_Stream5,
      DMA2_Stream6, DMA2_Stream7 },
    enable_dma_2
};


/**
 * @brief Ustawienie adresu peryferium strumienia DMA
 * 
 * @param *stream   Strumień DMA
 * @param addr      Adress celu 
 */
inline void DMAStream_set_periph(DMA_Stream_TypeDef *stream, uint32_t addr){
    stream->PAR = addr;
}
/**
 * @brief Ustawienie parametrów przesyłania do peryferium @p count razy 
 *        wartości spod adresu @p from
 * 
 * @param *stream   Strumień DMA
 * @param from      Adres pobierania danych
 * @param count     Liczba powtórzeń
 */
inline void
DMAStream_set_send(DMA_Stream_TypeDef *stream, uint32_t from, uint32_t count){
    stream->M0AR = from;
    stream->NDTR = count;
}

/**
 * @brief Ustawienie parametrów pobierania z peryferium @p count razy 
 *        wartości do adresu @p to
 * 
 * @param *stream   Strumień DMA
 * @param from      Adres ładowania danych
 * @param count     Liczba powtórzeń
 */
inline void
DMAStream_set_recv(DMA_Stream_TypeDef *stream, uint32_t to, uint32_t count){
    stream->M0AR = to;
    stream->NDTR = count;
}

/// @brief Uruchumienie działania strumienia DMA
/// @param *stream  Wskaźnik na strumień
inline void DMAStream_enable(DMA_Stream_TypeDef *stream){
    stream->CR |= DMA_SxCR_EN;
}

/**
 * @brief Odczytanie wartości odopwiedniego rejestru przechowującego
 *        informację o flagach przerwań kontrolera DMA
 * 
 * @param dma       Dane o wybranym kontrolerze DMA
 * @param stream    Numer strumienia DMA
 * @return uint32_t Wartość rejestru LISR albo HISR
 */
inline uint32_t DMA_read_interrupts(const dma_info dma, uint32_t stream){
    if (stream > 4)
        return dma.dma->HISR;
    else
        return dma.dma->LISR;
}

/**
 * @brief Czy flaga przerwania zdarzenia zakończenia transferu wybranego
 *        strumienia jest ustawiona?
 * 
 * @param interrupts    Wartość rejestru z informacjami o przerwaniach
 *                      (patrz: @ref DMA_read_interrupts)
 * @param stream        Numer strumienia DMA 
 */
inline bool DMA_TCI_check(const uint32_t interrupts, uint32_t stream){
    uint32_t check = 0;
    switch (stream) {
        case 0:
            check = DMA_LISR_TCIF0;
            break;
        case 1:
            check = DMA_LISR_TCIF1;
            break;
        case 2:
            check = DMA_LISR_TCIF2;
            break;
        case 3:
            check = DMA_LISR_TCIF3;
            break;
        case 4:
            check = DMA_HISR_TCIF4;
            break;
        case 5:
            check = DMA_HISR_TCIF5;
            break;
        case 6:
            check = DMA_HISR_TCIF6;
            break;
        case 7:
            check = DMA_HISR_TCIF7;
            break;
    }
    return interrupts & check;        
}

/**
 * @brief Czyści flagę przerwania zdarzenia zakończenia transferu wybranego
 *        strumienia DMA
 * 
 * @param dma       Dane o wybranym kontrolerze DMA
 * @param stream    Numer strumienia DMA
 */
inline void DMA_TCI_clear(const dma_info dma, uint32_t stream){
    switch (stream) {
        case 0:
            dma.dma->LIFCR = DMA_LIFCR_CTCIF0;
            break;
        case 1:
            dma.dma->LIFCR = DMA_LIFCR_CTCIF1;
            break;
        case 2:
            dma.dma->LIFCR = DMA_LIFCR_CTCIF2;
            break;
        case 3:
            dma.dma->LIFCR = DMA_LIFCR_CTCIF3;
            break;
        case 4:
            dma.dma->HIFCR = DMA_HIFCR_CTCIF4;
            break;
        case 5:
            dma.dma->HIFCR = DMA_HIFCR_CTCIF5;
            break;
        case 6:
            dma.dma->HIFCR = DMA_HIFCR_CTCIF6;
            break;
        case 7:
            dma.dma->HIFCR = DMA_HIFCR_CTCIF7;
            break;
    }
}

/**
 * @brief Sprawdza czy transfer strumiena się już zakończył i został obsłużony
 * 
 * @param dma       Dane o wybranym kontrolerze DMA
 * @param stream    Numer strumienia DMA
 */
inline bool DMA_Transfer_check(const dma_info dma, uint32_t stream){
    // Najpierw sprawdzamy EN
    bool EN_is_zero = (dma.stream[stream]->CR & DMA_SxCR_EN) == 0;

    if (!EN_is_zero)
        return false;

    // Jeżeli EN == 0 to sprawdzamy czy przerwanie zostało obsłużone
    switch (stream){
        case 0: return !(dma.dma->LISR & DMA_LISR_TCIF0);
        case 1: return !(dma.dma->LISR & DMA_LISR_TCIF1);
        case 2: return !(dma.dma->LISR & DMA_LISR_TCIF2);
        case 3: return !(dma.dma->LISR & DMA_LISR_TCIF3);
        case 4: return !(dma.dma->HISR & DMA_HISR_TCIF4);
        case 5: return !(dma.dma->HISR & DMA_HISR_TCIF5);
        case 6: return !(dma.dma->HISR & DMA_HISR_TCIF6);
        case 7: return !(dma.dma->HISR & DMA_HISR_TCIF7);
    }

    return false;
}

#endif /* DMA_H */