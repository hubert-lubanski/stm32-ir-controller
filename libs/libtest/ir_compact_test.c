#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) freq_temp_struct {
    uint16_t state  : 1;
    uint16_t cycles : 15;
} freq_template_t;

bool is_zero_freq_template(const freq_template_t ft){
    return (ft.cycles == 0);
}

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


uint32_t IR_compact_setup_buffers(const freq_template_t *msg,
                              uint16_t *arr_buffer, uint16_t *ccr_buffer){

    uint32_t index = 0;

    if (is_zero_freq_template(*msg))
        goto load_endvalues;

    const freq_template_t *ptr = msg;

    while(!is_zero_freq_template(*(ptr + 1))){
        /* Konsolidacja => ptr->state == 1, next->state == 0
            => ARR  = ptr->cycles + next->cycles - 1
            => CCR_ = ptr->cycles
        */
        arr_buffer[index] = ptr->cycles + (ptr + 1)->cycles - 1;
        ccr_buffer[index] = ptr->cycles;
        ptr += 2; // next block (1 przez X, 0 przez Y)
        index++;
    }
    // Ostatni blok 
    arr_buffer[index] = ptr->cycles + 1;
    ccr_buffer[index] = ptr->cycles;
    index++;

    load_endvalues: /* Ładowanie wartości wyłączających nadawanie */
    arr_buffer[index] = -1; //<- count as long as possible
    ccr_buffer[index] = 0;  //<- never emit any signal

    return index + 1;
}

int main(void) {
    freq_template_t test[] = {
        {0, 1}, {0, 2}, {0, 3}, {1, 3},
        {1, 5}, {1, 6}, {0, 7}, {1, 8},
        {0, 9}, {0, 10}, {1, 20}, {0,0}
    };

    int i = 0;
    while (!is_zero_freq_template(test[i])){
        printf("{%hu,%hu}\n", test[i].state, test[i].cycles);
        i++;   
    }

    printf("Compacting()...\n\n");
    IR_compact_message(test);

    i = 0;
    while (!is_zero_freq_template(test[i])){
        printf("{%hu,%hu}\n", test[i].state, test[i].cycles);
        i++;   
    } 

    uint16_t A[sizeof(test)/2], B[sizeof(test)/2];

    uint32_t len = IR_compact_setup_buffers(test, A, B);

    printf("len := %lu\n", len);
    for (int i = 0; i < len; ++i)
        printf("A[%d] := %lu, B[%d] := %lu\n", i, A[i], i, B[i]);
    
}