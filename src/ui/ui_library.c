
#include <stdlib.h>
#include <string.h>

#include "../lcd.h"
#include "../hal.h"
#include "../misc.h"
#include "../workset.h"
#include "../eeprom.h"

void save_name(uint8_t idx);
void load_name(uint8_t idx);
void workset_default(uint8_t idx);

/* +lcdconv */

static const char str1[] = "Библиотека изделий";
static const char str4[] = "3-Зап  4-Чтен #-Имя ";

static const char str_rd[] = "Загружено";
static const char str_wr[] = "Сохранено";
static const char str_err[] = "Ошибка";

static const char str_rma[] = "Удалить? да-A";
static const char str_rm[] = "Удалено";

/* -lcdconv */

extern WORKSET workset; // GLOBAL!

static uint8_t idx = 1;
static uint8_t edit = 0;
static char cname[WORKSET_NAME_LENGTH + 1];

int ui_library(char key)
{
    if (key == 0) return 0;

    if (edit == 10)
    {
        lcd_clr_str(STR3_ADDR);
        if (key == 'A')
        {
            workset_default(idx);
            lcd_print_rom(STR3_ADDR, str_rm);
        }
        edit = 0;
        return 0;
    }

    if (edit == 0)
    {
        if (key == '*') return 1;
        if (key == 'A') idx++;
        if (key == 'B') idx--;
        if (idx >= WORKSET_COUNT) idx = 1;
        if (idx == 0) idx = WORKSET_COUNT - 1;
        load_name(idx);
    }

    if (key == '#')
    {
        edit = (edit > 0) ? 0 : 1;
        if (edit == 0)
        {
            save_name(idx);
        }
        else
        {
            lcd_clr_str(STR3_ADDR);
            lcd_clr_str(STR4_ADDR);
        }
    }

    if (edit)
    {
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

    if (key == '3')
    {
        if (workset_save(idx))
            lcd_print_rom(STR3_ADDR, str_wr);
        else
            lcd_print_rom(STR3_ADDR, str_err);
    }

    if (key == '4')
    {
        if (workset_load(idx))
            lcd_print_rom(STR3_ADDR, str_rd);
        else
            lcd_print_rom(STR3_ADDR, str_err);
    }

    if (key == '8')
    {
        lcd_print_rom(STR3_ADDR, str_rma);
        edit = 10;
    }

    return 0;
}

void save_name(uint8_t idx)
{
    int addr = get_workset_name_addr(idx);
    eeprom_cs(0, addr);
    eeprom_write((uint8_t*) cname, WORKSET_NAME_LENGTH);
    eeprom_status_wait();
}

void load_name(uint8_t idx)
{
    memset(cname, 0, sizeof (cname));
    int addr = get_workset_name_addr(idx);
    eeprom_cs(0, addr);
    eeprom_read((uint8_t*) cname, WORKSET_NAME_LENGTH);
    eeprom_status_wait();
    trim_name(cname, WORKSET_NAME_LENGTH);
}

void workset_default(uint8_t idx)
{
    WORKSET ws;
    uint16_t p;
    uint16_t *w = (uint16_t *) & ws;
    for (int i = 0; i < WORKSET_PARAM_COUNT; i++)
    {
        get_param_limits(i, &p, 0);
        w[i] = p;
    }
    workset_save(idx);
    memset(cname, 0x20, sizeof (cname));
    save_name(idx);
}
