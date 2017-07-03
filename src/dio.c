
#include "hal.h"
#include "dio.h"

#define OUT_BASE_ADDR   0x08
#define IN_BASE_ADDR    0x10

#define IN_DEBOUNCE     3
#define IN_WORDS        (INPUTS_MAX >> 4)
#define OUT_WORDS       (OUTPUTS_MAX >> 4)

typedef struct {
    unsigned state;
    unsigned current;
    int count : 6;
}
D_IN;

D_IN inputs[INPUTS_MAX];
uint16_t out_data[OUT_WORDS];
uint16_t f_out_data_changed;

void dio_init(void) {
    int i, j;
    for (i = 0; i < OUT_WORDS; i++) out_data[i] = 0;
    f_out_data_changed = 0xFFFF;
    dio_flush();
    for (i = 0; i < INPUTS_MAX; i++) inputs[i].count = 0;
    for (;;) {
        dio_task();
        j = 0;
        for (i = 0; i < INPUTS_MAX; i++) if (inputs[i].count < IN_DEBOUNCE) j++;
        if (j == 0) break;
    }
}

void dio_task(void) {
    uint16_t data;
    int n, i, j;
    dio_flush();
    n = 0;
    for (i = 0; i < IN_WORDS; i++) {
        data = bus_read(IN_BASE_ADDR + i);
        for (j = 0; j < 16; j++, n++, data >>= 1) {
            unsigned in = (data & 1);
            if (in != inputs[n].current) inputs[n].count = 0;
            inputs[n].current = in;
            inputs[n].count++;
            if (inputs[n].count == IN_DEBOUNCE) inputs[n].state = in;
        }
    }
}

unsigned dio_in(uint8_t num) {
    if (num >= INPUTS_MAX) return 0;
    return inputs[num].state;
}

void dio_out(uint8_t num, unsigned v) {
    if (num >= OUTPUTS_MAX) return;
    uint16_t mask = 1 << (num & 15);
    num >>= 4;
    switch (v) {
        case 2:
            out_data[num] ^= mask;
            break;
        case 1:
            if ((out_data[num] & mask) > 0) return;
            out_data[num] |= mask;
            break;
        default:
            if ((out_data[num] & mask) == 0) return;
            mask ^= 0xFFFF;
            out_data[num] &= mask;
            break;
    }
    f_out_data_changed |= (1 << num);
}

unsigned dio_out_state(uint8_t num) {
    if (num >= OUTPUTS_MAX) return 0;
    uint16_t mask = 1 << (num & 15);
    return (out_data[num >> 4] & mask) ? 1 : 0;
}

void dio_flush(void) {
    int i;
    if (!f_out_data_changed) return;
    for (i = 0; i < OUT_WORDS; i++) {
        if (f_out_data_changed & 1)
            bus_write(OUT_BASE_ADDR + i, out_data[i]);
        f_out_data_changed >>= 1;
    }
    f_out_data_changed = 0;
}
