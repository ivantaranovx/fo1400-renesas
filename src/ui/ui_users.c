
#include <stdlib.h>
#include <string.h>

#include "../lcd.h"
#include "../hal.h"
#include "../eeprom.h"
#include "../helper.h"
#include "../workset.h"
#include "../eeprom.h"

/* +lcdconv */

static const char str1[] = "Выбор пользователя";
static const char str4[] = "*-Выбор #-Имя";

/* -lcdconv */

static uint8_t idx = 1;
static uint8_t edit = 0;

static char cname[20];

int ui_users(char key) {

    int addr;

    if (key == 0) return 0;

    if (edit == 0) {
        if (key == '*') return 1;
        if (key == 'A') idx++;
        if (key == 'B') idx--;
        if (idx >= USER_COUNT) idx = 1;
        if (idx == 0) idx = USER_COUNT - 1;
        memset(cname, 0, sizeof (cname));
        addr = get_user_name_addr(idx);
        eeprom_cs(0, addr);
        eeprom_read((uint8_t*) cname, USER_NAME_LENGTH);
        eeprom_status_wait();
    }

    if (key == '#') {
        edit = !(edit > 0);
        if (edit == 0) {
            addr = get_user_name_addr(idx);
            eeprom_cs(0, addr);
            eeprom_write((uint8_t*) cname, USER_NAME_LENGTH);
            eeprom_status_wait();
        } else {
            lcd_clr_str(STR4_ADDR);
        }
    }

    if (edit) {
        set_char(key, &cname[edit - 1]);
        if (key == '*') edit++;
        if (key == 'C') edit++;
        if (key == 'D') edit--;
        if (edit > USER_NAME_LENGTH) edit = 1;
        if (edit == 0) edit = USER_NAME_LENGTH;
        print_name(idx, cname);
        lcd_set_cursor(STR2_ADDR + 2 + edit, 1);
        return 0;
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, str1);
    print_name(idx, cname);
    lcd_print_rom(STR4_ADDR, str4);

    return 0;
}
