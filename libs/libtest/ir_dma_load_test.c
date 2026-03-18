#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../my_preprocessor.h"
#include "../common.h"

#define PROTOCOL NEC_protocol
// #define NO_STM
#include <ir_protocols.h>

static const IRCode messages[] = {
    POWER_OFF, POWER_ON, __TEST_CODE__, /*
    STANDBY, CHANNEL_0,
    VOLUME_UP, VOLUME_DOWN,
    PROGRAM_UP, PROGRAM_DOWN,
    CHANNEL_1, CHANNEL_1, CHANNEL_2
/**/};

int main(){
    uint32_t state;
    PROTOCOL.initialize_protocol_state(&state);

    const uint32_t mbs = PROTOCOL.minimal_buffer_size,
                   fqs = sizeof(freq_template_t);

    freq_template_t memory[array_size(messages)][mbs / fqs];

    for (int i = 0 ; i < sizeof(messages)/sizeof(*messages); ++i){
        PROTOCOL.create_message(&state, memory[i], messages[i], PROJECTOR);
    }

    for (int i = 0 ; i < sizeof(messages)/sizeof(*messages); ++i){
        int j = 0;
        while (!is_zero_freq_template(memory[i][j])){
            printf("{%hu,%hu}\n", memory[i][j].state, memory[i][j].cycles);
            j++;   
        }
        printf("Compacting()...\n\n");
        IR_compact_message(memory[i]);

        j = 0;
        do {
            printf("{%hu,%hu}\n", memory[i][j].state, memory[i][j].cycles);
        } while (!is_zero_freq_template(memory[i][j++]));
        printf("\n---\n\n");
    }

    uint16_t value_buffers[4][2][mbs / fqs / 2];
    for (int i = 0; i < sizeof(messages)/sizeof(*messages); ++i){
        IR_compact_setup_buffers_16(memory[i], 1, value_buffers[i][0], value_buffers[i][1]);
        printf("Message %d\nARR := ",i);
        int j = 0; 
        do {
            printf("%hd, ", value_buffers[i][0][j]);
        } while (value_buffers[i][1][j++] != (uint16_t)0);
        printf("length := %d\n\n", j);
        printf("\nCCR := ");
        j = 0;
        do {
            printf("%hd, ", value_buffers[i][1][j]);
        } while (value_buffers[i][1][j++] != (uint16_t)0);
        printf("length := %d\n\n", j);
        
    }
}

/*
Message 0
ARR := 63, 95, 63, 63, 63, 63, 63, 63, 95, 63, 95, 63, 1, 
CCR := 32, 64, 32, 32, 32, 32, 32, 32, 32, 32, 64, 32, 0, 

Message 1
ARR := 63, 63, 95, 63, 63, 63, 63, 63, 95, 63, 95, 63, 1, 
CCR := 32, 32, 64, 32, 32, 32, 32, 32, 32, 32, 64, 32, 0, 

Message 2
ARR := 63, 95, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 1, 
CCR := 32, 64, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 0, 

Message 3
ARR := 63, 63, 95, 63, 63, 63, 95, 95, 63, 63, 63, 63, 1, 
CCR := 32, 32, 64, 32, 32, 32, 32, 64, 32, 32, 32, 32, 0, 

Message 4
ARR := 63, 95, 63, 63, 63, 63, 63, 63, 63, 95, 95, 63, 1, 
CCR := 32, 64, 32, 32, 32, 32, 32, 32, 32, 32, 64, 32, 0, 
*/