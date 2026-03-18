#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../my_preprocessor.h"
#include "../common.h"

#define PROTOCOL NEC_protocol
#define NO_STM
#include <ir_protocols.h>
#include <timers.h>

USE_ONCE(unique);

static volatile uint32_t i = 0;
static const IRCode messages[] = {
    POWER_OFF, POWER_ON /*
    STANDBY, CHANNEL_0,
    VOLUME_UP, VOLUME_DOWN,
    PROGRAM_UP, PROGRAM_DOWN,
    CHANNEL_1, CHANNEL_1, CHANNEL_2
/**/};

static freq_template_t *message_buffer;
static uint32_t *arr_buffer, *ccr_buffer;
static uint32_t protocol_state;

void print_func(uint64_t arr_adr, uint64_t ccr_adr, uint32_t count){
    uint32_t *arr_buffer = (uint32_t *)arr_adr,
             *ccr_buffer = (uint32_t *)ccr_adr;
    
    printf("ARR := ");
    int addr_ptr = 0;
    for (int k = count; k > 0; k--){
        printf("%lu, ", arr_buffer[addr_ptr]);
        addr_ptr += 1;
    }
    printf("\n");
    printf("CCR := ");
    addr_ptr = 0;
    for (int k = count; k > 0; k--){
        printf("%lu, ", ccr_buffer[addr_ptr]);
        addr_ptr += 1;
    }
    printf("\n--\n");

}

void other_func(uint32_t multiplier){
    for (int a = 0; a < 5; ++a){
        PROTOCOL.create_message(&protocol_state, message_buffer, messages[i], PROJECTOR);
        i = (i + 1) % (array_size(messages));

        IR_compact_message(message_buffer);

        // int j = 0;
        // do {
        //     printf("{%hu,%hu}\n", memory[j].state, memory[j].cycles);
        // } while (!is_zero_freq_template(memory[j++]));
        // printf("\n---\n\n");

        

        uint32_t count = IR_compact_setup_buffers_32(message_buffer, multiplier,
                                    arr_buffer, ccr_buffer);

        printf("Message %lu (length of %lu)\nARR := ",i,count);
        int j = 0; 
        do {
            printf("%lu, ", arr_buffer[j]);
        } while (ccr_buffer[j++] != (uint32_t)0);
        printf("length := %d\n", j);
        printf("CCR := ");
        j = 0;
        do {
            printf("%lu, ", ccr_buffer[j]);
        } while (ccr_buffer[j++] != (uint32_t)0);
        printf("length := %d\n\nBy hand:\n", j);
        
        printf("First pair: (%lu, %lu)\n", arr_buffer[0], ccr_buffer[0]);
        print_func((uint64_t)(arr_buffer + 1), (uint64_t)(ccr_buffer + 1), (count - 1));
    }
}

#define timer_freq 16000000ull

int main(){

    const uint32_t mbs = PROTOCOL.minimal_buffer_size,
                   fqs = sizeof(freq_template_t);

    protocol_state = 0;
    freq_template_t memory[mbs / fqs + 5];
    uint32_t arr_memory[mbs / fqs / 2 + 2], ccr_memory[mbs / fqs / 2 + 2];
    message_buffer = memory;
    arr_buffer = arr_memory;
    ccr_buffer = ccr_memory;
    
    uint32_t psc = 0;
    uint32_t mult = 0;

    (void)get_psc_mult(timer_freq, PROTOCOL.freq_hz,
                      &psc, 16, &mult, 32, 2);

    

    PROTOCOL.initialize_protocol_state(&protocol_state);

    other_func(mult);



}