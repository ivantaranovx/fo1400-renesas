
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "misc.h"
#include "hal.h"
#include "lcd.h"
#include "workset.h"

typedef struct
{
    char key;
    char aKey;
    char bKey;
}
ABKEYS;

static uint8_t epos;
static uint8_t edot;
static uint8_t idx;
static char buf[STR_MAX_LENGTH + 1];

static ABKEYS abkeys[] = {
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
    lcd_clr_str(STR2_ADDR);
    if (*name)
        lcd_printf(STR2_ADDR, "%02d:%s", idx, name);
    else
        lcd_printf(STR2_ADDR, "%02d:---", idx);
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

bool check_int(int val, int *store)
{
    if (val == *store) return false;
    *store = val;
    return true;
}

void print_uint(uint8_t pos, uint16_t val, uint8_t dot)
{
    snprintf(buf, sizeof(buf), "%05u", val);
    lcd_printd(pos, buf, dot);
}

void ui_input_int(uint8_t pos, uint16_t val, uint8_t dot)
{
    epos = pos;
    edot = dot;
    idx = 0;
    print_uint(pos, val, dot);
    lcd_set_cursor(epos, 1);
}

int ui_input_int_process(uint8_t key)
{
    if (key == '#') return 2;
    if (key == '*') return 1;
    if ((key >= '0') && (key <= '9'))
    {
        buf[idx++] = key;
        if (idx > 4) idx = 0;
    }
    lcd_printd(epos, buf, edot);
    lcd_set_cursor(epos + ((edot > 0) && (idx >= edot)?idx + 1:idx), 1);
    return 0;
}

uint16_t ui_input_int_get(void)
{
    long int r = atol(buf);
    if (r > UINT16_MAX) r = UINT16_MAX;
    return (uint16_t) r;
}

