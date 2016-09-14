
#include <stdlib.h>
#include <string.h>

#include "../lcd.h"
#include "../hal.h"
#include "../24lc512.h"
#include "../helper.h"

#include "ui_users.h"

static const char str1[] = "Выбор пользователя";
static const char str4[] = "*-Выбор #-Имя";

static uint8_t idx;
static uint8_t edit;
static char cname[20];

uint8_t gui_users_main(char key);

void gui_users()
{
    idx = 1;
    edit = 0;
    lcd_clear();
    gui_users_main(0xFF);
    for (;;) if (gui_users_main(get_key())) break;
    lcd_clear();
}

uint8_t gui_users_main(char key)
{
    uint16_t wkaddr;
    char buf[10];

    if (key == 0) return 0;

    if (edit == 0)
    {
        if (key == '*') return 1;
        if (key == 'A') idx++;
        if (key == 'B') idx--;
        if (idx >= NUM_USERS) idx = 1;
        if (idx == 0) idx = NUM_USERS - 1;
    }

    wkaddr = ADDR_USERS + ((idx - 1) * sizeof (USERSET));

    if (key == '#')
    {
        edit = !(edit > 0);
        if (edit == 0)
        {
            writeArray(wkaddr, (uint8_t*)cname, user_name_sz);
        }
        else
        {
            lcd_clr_str(STR4_ADDR);
        }
    }

    if (edit)
    {
        set_char(key, &cname[edit - 1]);
        if (key == '*') edit++;
        if (key == 'C') edit++;
        if (key == 'D') edit--;
        if (edit > user_name_sz) edit = 1;
        if (edit == 0) edit = user_name_sz;
        print_name(idx, cname);
        lcd_set_cursor(STR2_ADDR + 2 + edit, 1);
        return 0;
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, str1);

    buf[0] = (idx / 10) + 0x30;
    buf[1] = (idx % 10) + 0x30;
    buf[2] = ':';
    buf[3] = ' ';
    buf[4] = 0;
    lcd_print(STR2_ADDR, buf);

    readArray(wkaddr, (uint8_t*)cname, user_name_sz);
    if (cname[0] == 0xFF) memset(cname, 0, user_name_sz);
    print_name(idx, cname);

    lcd_print_rom(STR4_ADDR, str4);
    return 0;
}
