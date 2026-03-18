#include <stdint.h>
typedef struct cyclic_buffer_struct {
    uint32_t size;
    uint32_t begin, end;
    char *data;
} buffer_t;

inline uint32_t space_used(buffer_t *b){
    if (b->begin <= b->end) {
        return b->end - b->begin;
    }
    else {
        return b->size - b->begin + b->end;
    }
}

inline uint32_t space_left(buffer_t *b) {
    return b->size - space_used(b) - 1;
}

inline uint32_t next(buffer_t *b, uint32_t ind) {
    ind += 1;
    ind %= b->size;
    return ind;
}

// Dodaje element na koniec bufora o ile jest jeszcze miejsce
inline void add_to_buffer(buffer_t *b, char *what, uint32_t size){
    if (space_left(b) < size)
        return;
    else {
        // Kopiujemy dane
        for (uint32_t i = 0; i < size; ++i){
            b->data[b->end] = what[i];
            b->end = next(b, b->end);
        }
    }
}

inline char pop_from_buffer(buffer_t *b){
    char retval = b->data[b->begin];
    b->begin = next(b, b->begin);
    return retval;
}

inline uint32_t buffer_swap(buffer_t *b, char **new_buffer){
    uint32_t data_begin;
    if (b->begin <= b->end) {
        // Swap is faster
        char *tmp = b->data;
        b->data = *new_buffer;
        *new_buffer = tmp;

        data_begin = b->begin;
    }
    else {
        //     E      B
        // [xxx|------|xxxxxx]
        // Move elements to new_buffer
        uint32_t j = 0;
        for (uint32_t i = b->begin; i < b->size; ++i, ++j)
            (*new_buffer)[j] = b->data[i];
        for (uint32_t i = 0; i < b->end; ++i, ++j)
            (*new_buffer)[j] = b->data[i];
        // No swapping
        data_begin = 0;
    }

    // reset
    b->begin = b->end = 0;

    return data_begin;
}