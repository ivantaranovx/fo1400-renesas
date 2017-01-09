
#include "hal.h"
#include "lcd.h"
#include "helper.h"

unsigned lcd_init(void)
{
    _lcd_write(0x30);
    _delay_us(2000);
    _lcd_write(0x30);
    _delay_us(2000);
    _lcd_write(0x30);
    _delay_us(150);
    _lcd_write(0x30);
    _delay_us(50);
    _lcd_write(0x20);
    _delay_us(50);

    if (_lcd_busy()) return 0;
    _lcd_write(0x20);
    if (_lcd_busy()) return 0;
    _lcd_write(0x2C);
    if (_lcd_busy()) return 0;
    _lcd_write(0x0C);
    if (_lcd_busy()) return 0;
    _lcd_write(0x06);
    if (_lcd_busy()) return 0;
    _lcd_write(0x01);
    if (_lcd_busy()) return 0;

    return 1;
}

unsigned lcd_clear(void)
{
    _lcd_set_rs(0);
    _lcd_write(0x01);
    if (_lcd_busy()) return 1;
    return 0;
}

unsigned lcd_clr_str(uint8_t pos)
{
    uint8_t i;
    if (lcd_set_cursor(pos, 0)) return 1;
    for (i = 0; i < 20; i++)
    {
        if (lcd_put_char(0x20)) return 1;
    }
    return 0;
}

unsigned lcd_set_cursor(uint8_t pos, unsigned show)
{
    _lcd_set_rs(0);
    if (show) _lcd_write(0x0E); else _lcd_write(0x0C);
    if (_lcd_busy()) return 1;
    _lcd_write(0x80 | pos);
    if (_lcd_busy()) return 1;
    return 0;
}

unsigned lcd_put_char(char c)
{
    _lcd_set_rs(1);
    _lcd_write((uint8_t)c);
    if (_lcd_busy()) return 1;
    return 0;
}

unsigned lcd_print(uint8_t pos, char *buf)
{
    if (buf == 0) return 1;
    if (lcd_set_cursor(pos, 0)) return 1;
    while(*buf)
    {
        if (lcd_put_char(*buf)) return 1;
        buf++;
    }
    return 0;
}

unsigned lcd_print_rom(uint8_t pos, const char *buf)
{
    if (buf == 0) return 1;
    return lcd_print(pos, (char*)buf);
}

