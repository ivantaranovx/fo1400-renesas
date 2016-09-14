
#include <string.h>

#include "helper.h"
#include "hal.h"
#include "lcd.h"

typedef struct
{
    char key;
    char aKey;
    char bKey;
}
ABKEYS;

static ABKEYS abkeys[] ={
    {'1', '0', '9'},
    {'2', 'A', 'C'},
    {'3', 'D', 'F'},
    {'4', 'G', 'I'},
    {'5', 'J', 'L'},
    {'6', 'M', 'O'},
    {'7', 'P', 'S'},
    {'8', 'T', 'V'},
    {'9', 'W', 'Z'},
    {'0', '+', '/'},
    {'*', ' ', ' '},
    {0, 0, 0}
};

void set_char(char key, char *str)
{
    uint8_t i = 0;
    for (;;)
    {
        if (abkeys[i].key == 0) return;
        if (abkeys[i].key == key) break;
        i++;
    }
    (*str)++;
    if (*str < abkeys[i].aKey) *str = abkeys[i].aKey;
    if (*str > abkeys[i].bKey) *str = abkeys[i].aKey;
}

void print_name(uint8_t idx, char *name)
{
    char buf[21];
    memset(buf, 0x20, 20);
    buf[0] = (idx / 10) + 0x30;
    buf[1] = (idx % 10) + 0x30;
    buf[2] = ':';
    buf[20] = 0;
    if (*name)
        strncpy(&buf[3], name, 16);
    else
        strcpy(&buf[3], "---");
	lcd_clr_str(STR2_ADDR);
    lcd_print(STR2_ADDR, buf);
}

void utoa(uint16_t val, char *buf, uint8_t pdot)
{
    uint8_t i;

    buf[0] = (val / 10000) + 0x30;
    val %= 10000;
    buf[1] = (val / 1000) + 0x30;
    val %= 1000;
    buf[2] = (val / 100) + 0x30;
    val %= 100;
    buf[3] = (val / 10) + 0x30;
    val %= 10;
    buf[4] = val + 0x30;
    buf[5] = 0;
    if (pdot > 0)
    {
        pdot--;
        for (i = 6; i > pdot; i--) buf[i] = buf[i - 1];
        buf[pdot] = '.';
    }
}

char b2h(uint8_t val)
{
    if (val < 10) return val + 48;
    return val + 55;
}

void hex(uint8_t val, char *buf)
{
    buf[0] = b2h(val >> 4);
    buf[1] = b2h(val & 0x0F);
    buf[2] = 0;
}

void uart_uint(uint16_t val)
{
    char buf[10];
    utoa(val, buf, 0);
    uart_send_str(buf);
}

unsigned lcd_uint(uint8_t pos, uint16_t val, uint8_t c)
{
    char buf[10];
    utoa(val, buf, c);
    return lcd_print(pos, buf);
}
