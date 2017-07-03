
#include <stdio.h>
#include <string.h>

#include "hal.h"
#include "eeprom.h"
#include "lcd.h"

#define DEBUG

static uint8_t addr_m;
static uint16_t addr_w;
static uint8_t page[130];

static uint8_t *io_buf;
static uint16_t io_len = 0;
static int io_state = 0;
static int io_status;

void eeprom_cs(uint8_t bank, uint16_t addr)
{
#ifdef DEBUG
    snprintf((char*) page, sizeof (page) - 1, "eeprom_cs: %i, %04X\r\n", bank, addr);
    uart_print((char*) page);
#endif
    addr_m = 0xA0 | ((bank & 7) << 1);
    addr_w = addr;
}

void eeprom_read(uint8_t *buf, uint16_t len)
{
#ifdef DEBUG
    snprintf((char*) page, sizeof (page) - 1, "eeprom_read: %i\r\n", len);
    uart_print((char*) page);
#endif
    if (io_state > 0) return;
    io_buf = buf;
    io_len = len;
    io_state = 20;
    set_timer(TMR_EEPROM, 50);
}

void eeprom_write(uint8_t *buf, uint16_t len)
{
#ifdef DEBUG
    snprintf((char*) page, sizeof (page) - 1, "eeprom_write: %i\r\n", len);
    uart_print((char*) page);
#endif
    if (io_state > 0) return;
    io_buf = buf;
    io_len = len;
    io_state = 30;
    set_timer(TMR_EEPROM, 50);
}

int eeprom_status_wait(void)
{
    int r;
    for (r = -1; r < 0; r = eeprom_status())
    {
        wdt_reset();
    }
#ifdef DEBUG
    snprintf((char*) page, sizeof (page) - 1, "eeprom_status_wait: %i\r\n", r);
    uart_print((char*) page);
#endif
    return r;
}

int eeprom_status(void)
{
    static int len;
    int r;

    if (get_timer(TMR_EEPROM) == 0)
    {
        io_status = 0;
        io_state = 0;
    }

    switch (io_state)
    {

    case 0:
        return io_status;

    case 20:
        page[0] = addr_w >> 8;
        page[1] = addr_w & 0xFF;
        i2c_io(addr_m, page, 2, 1);
        io_state = 21;
        break;
    case 21:
        r = i2c_io_status();
        if (r < 0) break;
        io_state = 20;
        if (r == 0) break;
        i2c_io(addr_m | 1, io_buf, io_len, 0);
        io_state = 22;
        break;
    case 22:
        r = i2c_io_status();
        if (r < 0) break;
        io_state = 20;
        if (r == 0) break;
        io_status = 1;
        io_state = 0;
        kill_timer(TMR_EEPROM);
        break;

    case 30:
        len = 128 - (addr_w & 0x7F);
        if (io_len < len) len = io_len;
        page[0] = addr_w >> 8;
        page[1] = addr_w & 0xFF;
        memcpy(&page[2], io_buf, len);
        i2c_io(addr_m, page, len + 2, 0);
        io_state = 31;
        break;
    case 31:
        r = i2c_io_status();
        if (r < 0) break;
        io_state = 30;
        if (r == 0) break;
        io_len -= len;
        if (io_len == 0)
        {
            io_status = 1;
            io_state = 0;
            kill_timer(TMR_EEPROM);
            break;
        }
        io_buf += len;
        addr_w += len;
        break;
    }
    return -1;
}

