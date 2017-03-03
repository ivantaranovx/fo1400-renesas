
#include <stdlib.h>
#include <string.h>

#include "../lcd.h"
#include "../hal.h"
#include "../helper.h"
#include "../workset.h"
#include "../eeprom.h"

/* +lcdconv */

static const char str1[] = "Библиотека изделий";
static const char str4[] = "3-Зап. 4-Чтен. #-Имя";
static const char str_rd[] = "Загружено";
static const char str_wr[] = "Сохранено";
static const char str_err[] = "Ошибка";

/* -lcdconv */

extern WORKSET workset; // GLOBAL!

static uint8_t idx = 1;
static uint8_t edit = 0;
static char cname[20];

int ui_library(char key) {

    int addr;

    if (key == 0) return 0;

    if (edit == 0) {
        if (key == '*') return 1;
        if (key == 'A') idx++;
        if (key == 'B') idx--;
        if (idx >= WORKSET_COUNT) idx = 1;
        if (idx == 0) idx = WORKSET_COUNT - 1;
        memset(cname, 0, sizeof (cname));
        addr = get_workset_name_addr(idx);
        eeprom_cs(0, addr);
        eeprom_read((uint8_t*) cname, WORKSET_NAME_LENGTH);
        eeprom_status_wait();
    }

    if (key == '#') {
        edit = !(edit > 0);
        if (edit == 0) {
            addr = get_workset_name_addr(idx);
            eeprom_cs(0, addr);
            eeprom_write((uint8_t*) cname, WORKSET_NAME_LENGTH);
            eeprom_status_wait();
        } else {
            lcd_clr_str(STR3_ADDR);
            lcd_clr_str(STR4_ADDR);
        }
    }

    if (edit) {
        set_char(key, &cname[edit - 1]);
        if (key == '*') edit++;
        if (key == 'C') edit++;
        if (key == 'D') edit--;
        if (edit > WORKSET_NAME_LENGTH) edit = 1;
        if (edit == 0) edit = WORKSET_NAME_LENGTH;
        print_name(idx, cname);
        lcd_set_cursor(STR2_ADDR + 2 + edit, 1);
        return 0;
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, str1);
    print_name(idx, cname);
    lcd_print_rom(STR4_ADDR, str4);

    if (key == '3') {

        addr = get_workset_name_addr(idx);
        eeprom_cs(0, addr);
        eeprom_write((uint8_t*) & workset, sizeof (workset));
        if (eeprom_status_wait())
            lcd_print_rom(STR3_ADDR, str_wr);
        else
            lcd_print_rom(STR3_ADDR, str_err);
    }

    if (key == '4') {

        addr = get_workset_name_addr(idx);
        eeprom_cs(0, addr);
        eeprom_read((uint8_t*) & workset, sizeof (workset));
        if (eeprom_status_wait())
            lcd_print_rom(STR3_ADDR, str_rd);
        else
            lcd_print_rom(STR3_ADDR, str_err);

        addr = get_workset_name_addr(0);
        eeprom_cs(0, addr);
        eeprom_write((uint8_t*) & workset, sizeof (workset));
        if (eeprom_status_wait())
            lcd_print_rom(STR3_ADDR + 10, str_wr);
        else
            lcd_print_rom(STR3_ADDR + 10, str_err);
    }

    return 0;
}
