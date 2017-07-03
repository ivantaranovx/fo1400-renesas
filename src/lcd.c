
#include <stdarg.h>
#include <stdio.h>

#include "hal.h"
#include "lcd.h"
#include "misc.h"

char utf8_hd44780(uint8_t **buf);

unsigned lcd_init(void)
{
    lcd_write(0x30);
    _delay_us(2000);
    lcd_write(0x30);
    _delay_us(2000);
    lcd_write(0x30);
    _delay_us(150);
    lcd_write(0x30);
    _delay_us(50);
    lcd_write(0x20);
    _delay_us(50);

    if (lcd_busy()) return 0;
    lcd_write(0x20);
    if (lcd_busy()) return 0;
    lcd_write(0x2C);
    if (lcd_busy()) return 0;
    lcd_write(0x0C);
    if (lcd_busy()) return 0;
    lcd_write(0x06);
    if (lcd_busy()) return 0;
    lcd_write(0x01);
    if (lcd_busy()) return 0;

    return 1;
}

unsigned lcd_clear(void)
{
    lcd_set_rs(0);
    lcd_write(0x01);
    if (lcd_busy()) return 1;
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
    lcd_set_rs(0);
    if (show) lcd_write(0x0E);
    else lcd_write(0x0C);
    if (lcd_busy()) return 1;
    lcd_write(0x80 | pos);
    if (lcd_busy()) return 1;
    return 0;
}

unsigned lcd_put_char(char c)
{
    lcd_set_rs(1);
    lcd_write((uint8_t) c);
    if (lcd_busy()) return 1;
    return 0;
}

unsigned lcd_printd(uint8_t pos, char *buf, uint8_t dot)
{
    uint8_t p = 0;
    char c;
    if (buf == 0) return 1;
    if (lcd_set_cursor(pos, 0)) return 1;
    while (*buf)
    {
        if ((dot > 0) && (dot == p)) lcd_put_char('.');
        c = utf8_hd44780((uint8_t**)&buf);
        if (lcd_put_char(c)) return 1;
        p++;
    }
    return 0;
}

unsigned lcd_print_rom(uint8_t pos, const char *buf)
{
    if (buf == 0) return 1;
    return lcd_print(pos, (char*) buf);
}

void lcd_printf(unsigned char pos, const char *fmt, ...)
{
    va_list args;
    char buf[STR_MAX_LENGTH + 1];
    va_start(args, fmt);
    vsnprintf(buf, sizeof (buf), fmt, args);
    va_end(args);
    lcd_print(pos, buf);
}

char utf8_hd44780(uint8_t **buf)
{
    const uint8_t utab[64] = {
        //А, Б, В, Г, Д, Е, Ж, З, И, Й, К, Л, М, Н, О, П,
        0x41, 0xA0, 0x42, 0xA1, 0xE0, 0x45, 0xA3, 0xA4, 0xA5, 0xA6, 0x4B, 0xA7, 0x4D, 0x48, 0x4F, 0xA8,
        //Р, С, Т, У, Ф, Х, Ц, А, Ш, Щ, а, Ы, Ь, Э, Ю, Я,
        0x50, 0x43, 0x54, 0xA9, 0xAA, 0x58, 0xE1, 0xAB, 0xAC, 0xE2, 0xAD, 0xAE, 0x62, 0xAF, 0xB0, 0xB1,
        //а, б, в, г, д, е, ж, з, и, й, к, л, м, н, о, п,
        0x61, 0xB2, 0xB3, 0xB4, 0xE3, 0x65, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0x6F, 0xBE,
        //р, с, т, у, ф, х, ц, ч, ш, щ, ъ, ы, ь, э, ю, я
        0x70, 0x63, 0xBF, 0x79, 0xE4, 0x78, 0xE5, 0xC0, 0xC1, 0xE6, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7
    };
    if ((**buf) < 0x80)
    {
        uint8_t c = (**buf);
        (*buf)++;
        return c;
    }
    if (((**buf) & 0xE0) == 0xC0)
    {
        uint16_t c = (**buf) & 0x1F;
        c <<= 6;
        (*buf)++;
        c += (**buf) & 0x3F;
        (*buf)++;
        if (c == 0x0401) return 0xA2;
        if (c == 0x0451) return 0xB5;
        if (c < 0x0410) return '?';
        if (c > 0x044F) return '?';
        c -= 0x0410;
        return utab[c];
    }
    if (((**buf) & 0xF0) == 0xE0)
    {
        (*buf) += 3;
        return '?';
    }
    if (((**buf) & 0xF8) == 0xF0)
    {
        (*buf) += 4;
        return '?';
    }
    if (((**buf) & 0xFC) == 0xF8)
    {
        (*buf) += 5;
        return '?';
    }
    if (((**buf) & 0xFE) == 0xFC)
    {
        (*buf) += 6;
        return '?';
    }
    return '?';
}
