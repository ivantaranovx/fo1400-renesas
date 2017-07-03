
#include <stdlib.h>
#include <string.h>

#include "../lcd.h"
#include "../hal.h"
#include "../eeprom.h"
#include "../misc.h"
#include "../workset.h"
#include "../eeprom.h"

static const char str1[] = "Выбор пользователя";
static const char str3[] = "Тариф: ";
static const char str4[] = "D-Выбор #-Имя";

static uint16_t u_id = 0;
static uint16_t u_cf = 0;

static uint16_t idx = 1;
static uint8_t edit = 0;

static char cname[USER_NAME_LENGTH + 1];

uint16_t ui_users_get_id(void)
{
    return u_id;
}

uint16_t ui_users_get_cf(void)
{
    return u_cf;
}

int ui_users(char key)
{
    int addr;

    if (key == 0) return 0;

    if (edit == 0)
    {
        if (key == '*') return 1; // cancel
        if (key == 'D')
        {
            u_id = idx;
            return 2; // select
        }
        if ((key >= '0') && (key <= '9')) u_cf = key - 0x30;
        if (key == 'A') idx++;
        if (key == 'B') idx--;
        if (idx > USER_COUNT) idx = 1;
        if (idx == 0) idx = USER_COUNT;
        memset(cname, 0, sizeof (cname));
        addr = get_user_name_addr(idx - 1);
        eeprom_cs(0, addr);
        eeprom_read((uint8_t*) cname, USER_NAME_LENGTH);
        eeprom_status_wait();
        trim_name(cname, USER_NAME_LENGTH);
    }

    if (key == '#')
    {
        edit = !(edit > 0);
        if (edit == 0)
        {
            addr = get_user_name_addr(idx - 1);
            eeprom_cs(0, addr);
            eeprom_write((uint8_t*) cname, USER_NAME_LENGTH);
            eeprom_status_wait();
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
        if (edit > USER_NAME_LENGTH) edit = 1;
        if (edit == 0) edit = USER_NAME_LENGTH;
        print_name(idx, cname);
        lcd_set_cursor(STR2_ADDR + 3 + edit, 1);
        return 0;
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, str1);
    print_name(idx, cname);
    lcd_print_rom(STR3_ADDR, str3);
    lcd_put_char(u_cf + 0x30);
    lcd_print_rom(STR4_ADDR, str4);
    return 0;
}
