
#include "hal.h"
#include "dio.h"

#define OUT_BASE_ADDR   0x08
#define IN_BASE_ADDR    0x10

#define IN_DEBOUNCE     3
#define INPUTS_MAX      64

typedef struct
{
    unsigned state   :1;
    unsigned current :1;
    unsigned count   :6;
}
D_IN;

D_IN        inputs[INPUTS_MAX];
uint16_t    out_data[4];
int         f_out_data_changed;

void dio_init(void)
{
    int i;
    for (i = 0; i < 4; i++) out_data[i] = 0;
    f_out_data_changed = 1;
    for (i = 0; i < INPUTS_MAX; i++) inputs[i].count = 0;
    for(;;)
    {
        dio_task();
        for (i = 0; i < INPUTS_MAX; i++) if (inputs[i].count < IN_DEBOUNCE) continue;
        break;
    }
    _bus_enable(1);
}

void dio_task(void)
{
    uint16_t data;
    uint16_t mask;
    int n, i;

    dio_flush();

    n = 0;
    for (i = 0; i < 4; i++)
    {
        data = _bus_read(IN_BASE_ADDR + i);
        for (mask = 1; mask > 0; mask <<= 1)
        {
            unsigned in = (data & mask)?1:0;
            if (in != inputs[n].current) inputs[n].count = 0;
            inputs[n].current = in;
            if (inputs[n].count < IN_DEBOUNCE) inputs[n].count++; else inputs[n].state = in;
            n++;
        }
    }
}

unsigned dio_in(uint8_t num)
{
    if (num >= INPUTS_MAX) return 0;
    return inputs[num].state;
}

void dio_out(uint8_t num, unsigned v)
{
    uint16_t mask = 1;
    mask <<= (num & 15);
    if (v)
    {
        out_data[num >> 4] |= mask;
        f_out_data_changed = 1;
        return;
    }
    mask ^= 0xFFFF;
    out_data[num >> 4] &= mask;
    f_out_data_changed = 1;
}

void dio_flush(void)
{
    int i;
    if (!f_out_data_changed) return;
    for (i = 0; i < 4; i++)
    {
        _bus_write(OUT_BASE_ADDR + i, out_data[i]);
    }
    f_out_data_changed = 0;
}
