/**
 * @file IRController.c
 * @author Hubert Lubański (h.lubanski@student.uw.edu.pl)
 * 
 * @brief Przykładowe użycie biblioteki dla kontrolera pilota podczerwieni
 * 
 */
#include <stdint.h>
#include <stdbool.h>

#include <stm32.h>
#include <delay.h>

#include <my_gpio.h>
#include <leds.h>
#include <buttons.h>
#include <timers.h>
#include <ir_protocols.h>

__NO_RETURN
void error(void){
    for (;;) {
        blink_led(RED_LED, 1000000);
        Delay(500000);
    }
}


void blink_red(void){
    blink_led(RED_LED, 200000);
    Delay(100000);
}

void blink_blue(void){
    blink_led(BLUE_LED, 200000);
}

void blink_green(void){
    blink_led(GREEN_LED, 50000);
}

void Hello(void){
    for (int i = 0; i < 3; ++i) {
        blink_led(BLUE_LED, 750000);
        Delay(500000);
    }
}

#define NULL ((void*)0)
#define NO_CALLBACK NULL
// Definiujemy parametry naszego pilota

#define MASTER_TIMER timer2     ///< Licznik nadrzędny
#define MASTER_LINE 1           ///< Linia sterująca licznikiem podrzędnym

#define FREQ_TIMER timer3       ///< Licznik podrzędny
#define IR_LED_LINE 4           ///< Linia sterująca diodą podczerowną

#define DMA_ARR_STREAM  7       ///< Strumień DMA obsługujący rejestr ARR
#define DMA_ARR_CHANNEL 3       ///< Kanał do odbierania zdarzenia uaktualnienia

#define DMA_CCRx_STREAM 5       ///< Strumień DMA obsługujący rejestr CCR
#define DMA_CCRx_CHANNEL 3      ///< Kanał do odbierania zdarzenia zgodności

#define PROTOCOL NEC_protocol   ///< Protokół IR, z którego korzystamy


#define __DMA1_STREAM_HANDLER(x) DMA1_Stream ## x ## _IRQHandler
/// @brief Makro automatycznie tworzące nazwę odpowiedniej procedury obsługi
///        przerwania DMA1 dla zadanego strumienia
#define DMA1_STREAM_HANDLER(x) __DMA1_STREAM_HANDLER(x)

#define __DMA1_IRQ_NUMBER(x) DMA1_Stream ## x ## _IRQn
#define DMA1_IRQ_NUMBER(x) __DMA1_IRQ_NUMBER(x)

#define BUTTON_MUFFLER_TIMER_INFO timer5
#define BUTTON_MUFFLER_TIMER TIM5
#define __miliseconds 1000ull
#define MUFFLER_BASE_FREQ BASE_HSI_HZ
#define MUFFLER_WAIT_TIME_MS 40
#define MUFFLER_PSC 1600ull
// 1 * MUFFLER_ARR_MULT = 1 milisekunda; = 16
#define MUFFLER_ARR_MULT 10


#define POWER_OFF_BUTTON_NAME JOYSTICK_DOWN
#define POWER_OFF_BUTTON buttons[POWER_OFF_BUTTON_NAME]
#define POWER_ON_BUTTON_NAME JOYSTICK_UP
#define POWER_ON_BUTTON  buttons[POWER_ON_BUTTON_NAME]

#define NO_INITIAL_STATE 0

// Głowny kontroler pilota
static IR_controller_t *main_controller;

// Licznik tłumiący drgania styków
static muffler_timer_t *muffler;



// Dostępne akcje przycisków

void power_on_action(void) {
    int ret = IR_send_message(main_controller, POWER_ON, PROJECTOR);
    if (ret == IR_SEND_MESSAGE_NREADY)
        blink_led(RED_LED, 250000);
}

void power_off_action(void) {
    int ret = IR_send_message(main_controller, POWER_OFF, PROJECTOR);
    if (ret == IR_SEND_MESSAGE_NREADY)
        blink_led(RED_LED, 250000);
}

void no_action(void){
    return;
}

// Przypisanie akcji do odpowiedniego przycisku

static void (*const button_actions[])(void) = {
    [POWER_OFF_BUTTON_NAME] = power_off_action,
    [POWER_ON_BUTTON_NAME]  = power_on_action
};

void do_button_action(button_type btn){
    if (btn == NO_BUTTON)
        error();
    else
        button_actions[btn]();      // piękny syntax :)
}

void long_press_action(button_type _){
    (void)_;
    IR_send_repeat(main_controller);
    blink_green();
}


muffler_timer_t example_muffler(void){
    return (muffler_timer_t){
        .base_timer         = BUTTON_MUFFLER_TIMER,
        .muffler_arr_mult   = MUFFLER_ARR_MULT,

        .last_pressed       = NO_BUTTON,
        .already_handled    = false,

        .muffle_time_cycles = MUFFLER_WAIT_TIME_MS, // traktujemy cykl jak ms
        .on_press_action    = do_button_action,

        .time_left          = 0,
        .long_press_ccr     = 1,
        .long_press_cycles  = 1000,
        //.long_press_cycles  = PROTOCOL.repeat_wait_ms, // traktujemy cykl jak ms
        .long_press_action  = long_press_action,
    };
}


IR_controller_t example_controller(freq_template_t *msg_buffer,
                                   void *transfer_buffers[2]) {
    return (IR_controller_t) {
        .bit_timer = MASTER_TIMER,
        .bit_line = MASTER_LINE,
        .bit_line_active_low = false,

        .freq_timer = FREQ_TIMER,
        .freq_line = IR_LED_LINE,
        .freq_line_active_low = false,

        .dma = DMA1_INFO,
        .dma_arr_stream = DMA_ARR_STREAM,
        .dma_arr_channel = DMA_ARR_CHANNEL,
        .dma_ccr_stream = DMA_CCRx_STREAM,
        .dma_ccr_channel = DMA_CCRx_CHANNEL,
        .dma_transfer_end_status = TRANSFER_COMPLETE,

        .message_buffer = msg_buffer,
        .transfer_buffers = {transfer_buffers[0], transfer_buffers[1]},

        .protocol = PROTOCOL,
        .protocol_state = NO_INITIAL_STATE,
    };
}


// Podłączamy procedury obsługujące z modułu IR do procedur przerwań

void TIM2_IRQHandler(void)
{
    IR_bit_timer_transfer_IRQHandler(main_controller, NO_CALLBACK, NO_CALLBACK);
}

void DMA1_STREAM_HANDLER(DMA_ARR_STREAM)(void)
{
    IR_DMAStream_arr_transfer_IRQHandler(main_controller, NO_CALLBACK);
}

void DMA1_STREAM_HANDLER(DMA_CCRx_STREAM)(void)
{
    IR_DMAStream_ccr_transfer_IRQHandler(main_controller, NO_CALLBACK);
}

void TIM5_IRQHandler(void) {
    muffler_update_event_IRQHandler(muffler);
}


void initialize_hardware(void){

    GPIOA_enable_clock();
    GPIOB_enable_clock();
    GPIOC_enable_clock();
    SYSCFG_enable_clock();

    MASTER_TIMER.enable_clock();
    FREQ_TIMER.enable_clock();

    // Licznik do tłumienia drgań styków przycisków
    BUTTON_MUFFLER_TIMER_INFO.enable_clock();
    BUTTON_MUFFLER_TIMER->PSC = MUFFLER_PSC;
    
    NVIC_EnableIRQ(BUTTON_MUFFLER_TIMER_INFO.update_interrupt_number);

    DMA1_INFO.enable_clock();
    NVIC_EnableIRQ(DMA1_IRQ_NUMBER(DMA_ARR_STREAM));
    NVIC_EnableIRQ(DMA1_IRQ_NUMBER(DMA_CCRx_STREAM));

    __NOP();
    __NOP();

    configure_led(GREEN2_LED);  // na potrzeby sygnalizowania, że żyjemy w main
    configure_led(RED_LED);     // dla funckji error()
    configure_led(BLUE_LED);
    configure_led(GREEN_LED);

    GPIOafConfigure(GPIOB, 1, GPIO_OType_PP,
                              GPIO_High_Speed,
                              GPIO_PuPd_NOPULL,
                              FREQ_TIMER.gpio_af);

}

void configure_buttons(void){
    configure_button(POWER_ON_BUTTON, GPIO_PuPd_UP,
                                      EXTI_Mode_Interrupt,
                                      EXTI_Trigger_Falling);
    
    configure_button(POWER_OFF_BUTTON, GPIO_PuPd_UP,
                                       EXTI_Mode_Interrupt,    
                                       EXTI_Trigger_Falling);  


    NVIC_EnableIRQ(EXTI9_5_IRQn);
}


void EXTI9_5_IRQHandler(void) {

    uint32_t pr = EXTI->PR;

    if (EXTI_Interrupt_Check(pr, POWER_OFF_BUTTON.pin)){
        EXTI_Interrupt_Clear(POWER_OFF_BUTTON.pin);
        // blink_led(BLUE_LED, 300000);

        signal_button_press(muffler, POWER_OFF_BUTTON_NAME);
    }

    if (EXTI_Interrupt_Check(pr, POWER_ON_BUTTON.pin)){
        EXTI_Interrupt_Clear(POWER_ON_BUTTON.pin);
        // blink_led(BLUE_LED, 300000);

        signal_button_press(muffler, POWER_ON_BUTTON_NAME);
    }

}

int main(){

    const uint32_t reqiured_buffer_size =
        (PROTOCOL.minimal_buffer_size / sizeof(freq_template_t));

    freq_template_t msg_buffer[reqiured_buffer_size];
    uint32_t local_arr_buffer[reqiured_buffer_size + 2];
    uint32_t local_ccr_buffer[reqiured_buffer_size + 2];

    void *local_transfer_buffers[2] = {
        local_arr_buffer,
        local_ccr_buffer
    };



    initialize_hardware();

    

    IR_controller_t local_controller =
        example_controller(msg_buffer, local_transfer_buffers);

    muffler_timer_t local_muffler = example_muffler();

    main_controller = &local_controller;
    muffler = &local_muffler;

    muffler_init_base_timer(muffler);



    int retval = IR_setup_controller(main_controller);

    if (retval != 0)
        error();



    configure_buttons();



    Hello();

    
    
    for (;;) {
        __WFI();    // Idziemy spać aż do następnego przerwania

        // Migamy 3 razy, aby zasygnalicować, że żyjemy :)
        for (int i = 0; i < 2; ++i){
            blink_led(GREEN2_LED, 300000);
            Delay(300000);
        }
    }

}