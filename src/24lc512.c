
#include "hal.h"
#include "24lc512.h"

static uint8_t ctl = 0xA0;

void chipSelect(uint8_t addr)
{
    ctl = 0xA0 | ((addr & 7) << 1);
}

uint16_t readArray(uint16_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t l = 0;

    i2c_start();
    if (i2c_send(ctl)) goto readArray_exit;
    if (i2c_send(addr >> 8)) goto readArray_exit;
    if (i2c_send(addr & 0xFF)) goto readArray_exit;
    i2c_rstart();
    if (i2c_send(ctl | 1)) goto readArray_exit;
    while (len--)
    {
        *(buf++) = i2c_recv(len > 0);
        l++;
    }

readArray_exit:
    i2c_stop();
    return l;
}

uint8_t writeArray(uint16_t addr, uint8_t *buf, uint8_t len)
{
    uint8_t l = 0;

    i2c_start();
    if (i2c_send(ctl)) goto writeArray_exit;
    if (i2c_send(addr >> 8)) goto writeArray_exit;
    if (i2c_send(addr & 0xFF)) goto writeArray_exit;
    while (len--)
    {
        if (i2c_send(*(buf++))) break;
        l++;
    }

writeArray_exit:
    i2c_stop();
    return l;
}

