#ifndef USART_H
#define USART_H



#include <gpio.h>
#include <stm32.h>
#include <stdbool.h>

/* USART CR1 REGISTER BITS */
#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE

#define USART_WordLength_8b 0x0000
#define USART_WordLength_9b USART_CR1_M
#define USART_Parity_No 0x0000
#define USART_Parity_Even USART_CR1_PCE
#define USART_Parity_Odd (USART_CR1_PCE | USART_CR1_PS)

/* USART CR2 REGISTER BITS */
#define USART_StopBits_1 0x0000
#define USART_StopBits_0_5 0x1000
#define USART_StopBits_2 0x2000
#define USART_StopBits_1_5 0x3000

/* USART CR3 REGISTER BITS */
#define USART_FlowControl_None 0x0000
#define USART_FlowControl_RTS USART_CR3_RTSE
#define USART_FlowControl_CTS USART_CR3_CTSE


typedef struct {
    USART_TypeDef * usart;
    GPIO_TypeDef * GPIO;
    uint8_t tx_pin : 4;
    uint8_t rx_pin : 4;
    uint8_t usart_af;
    void (*enable_clock)(void);
} usart_info;

void enable_usart_1(void) { RCC->APB2ENR |= RCC_APB2ENR_USART1EN; }
void enable_usart_2(void) { RCC->APB1ENR |= RCC_APB1ENR_USART2EN; }


const usart_info USART1_INFO = {USART1, GPIOA, 9, 10, GPIO_AF_USART1, enable_usart_1};
const usart_info USART2_INFO = {USART2, GPIOA, 2, 3, GPIO_AF_USART2, enable_usart_2};

inline void configure_usart_gpio(usart_info ui){
    GPIOafConfigure(ui.GPIO,
                    ui.rx_pin,
                    GPIO_OType_PP,
                    GPIO_Fast_Speed,
                    GPIO_PuPd_NOPULL,
                    ui.usart_af);
    GPIOafConfigure(ui.GPIO,
                    ui.tx_pin,
                    GPIO_OType_PP,
                    GPIO_Fast_Speed,
                    GPIO_PuPd_NOPULL,
                    ui.usart_af);
}

inline void
usart_setup(usart_info ui,
            uint32_t CR1, uint32_t CR2, uint32_t CR3, uint32_t BRR){

    ui.usart->CR1 = CR1;
    ui.usart->CR2 = CR2;
    ui.usart->CR3 = CR3;
    ui.usart->BRR = BRR;
}

inline void usart_enable(usart_info ui){
    ui.usart->CR1 |= USART_Enable;
}

inline bool usart_can_read(usart_info ui){
    return (ui.usart->SR & USART_SR_RXNE);
}

inline bool usart_can_write(usart_info ui){
    return (ui.usart->SR & USART_SR_TXE);
}

#endif /* USART_H */