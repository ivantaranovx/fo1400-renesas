
#include "ui_library.h"
#include "../lcd.h"
#include "../hal.h"
#include "../misc.h"
#include "../workset.h"
#include "../eeprom.h"

#include <stdlib.h>
#include <string.h>

void save_name(uint16_t idx);
void workset_default(uint16_t idx);

static const char str1[] = "Выбор изделия";
static const char str4[] = "3/4/8/#";

static const char str_ok[] = "Ok";
static const char str_err[] = "Err";

static const char str_rma[] = "Удалить? да-A";
static const char str_rm[] = "Удалено";

extern WORKSET workset; // GLOBAL!

char name[WORKSET_NAME_LENGTH + 1];

static uint8_t edit = 0;
static uint16_t id;
static bool select;

int ui_library(char key)
{
    if (key == 0) return 0;

    if (key == 'R')
    {
        name[WORKSET_NAME_LENGTH] = 0;
        id = 1;
        select = false;
    }

    if (edit == 100)
    {
        lcd_clr_str(STR4_ADDR);
        if (key == 'A')
        {
            workset_default(id);
            print_name(id, name);
            lcd_print_rom(STR4_ADDR, str_rm);
        }
        edit = 0;
        return 0;
    }

    if (edit == 0)
    {
        if (key == '*')
        {
            if (select) return id;
            return -1;
        }
        if (key == 'A') id++;
        if (key == 'B') id--;
        if (key == '#')
        {
            edit = 1;
            lcd_clr_str(STR3_ADDR);
            lcd_clr_str(STR4_ADDR);
            key = 0;
        }
        if (id > WORKSET_COUNT) id = 1;
        if (id == 0) id = WORKSET_COUNT;
        load_name(id, name);
    }

    while (edit)
    {
        if (key == '#')
        {
            save_name(id);
            edit = 0;
            break;
        }
        set_char(key, &name[edit - 1]);
        if (key == '*') edit++;
        if (key == 'A') edit++;
        if (key == 'B') edit--;
        if (edit > WORKSET_NAME_LENGTH) edit = 1;
        if (edit == 0) edit = WORKSET_NAME_LENGTH;
        print_name(id, name);
        lcd_set_cursor(STR2_ADDR + 3 + edit, 1);
        return 0;
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, str1);
    print_name(id, name);
    lcd_print_rom(STR4_ADDR, str4);

    if (key == '3')
    {
        if (workset_save(id))
        {
            lcd_print_rom(STR1_ADDR + 17, str_ok);
            select = true;
        }
        else
            lcd_print_rom(STR1_ADDR + 17, str_err);
    }

    if (key == '4')
    {
        if (workset_load(id))
        {
            lcd_print_rom(STR1_ADDR + 17, str_ok);
            save_name(0);
            select = true;
        }
        else
            lcd_print_rom(STR1_ADDR + 17, str_err);
    }

    if (key == '8')
    {
        lcd_clr_str(STR4_ADDR);
        lcd_print_rom(STR4_ADDR, str_rma);
        edit = 100;
    }

    return 0;
}

void save_name(uint16_t idx)
{
    uint16_t addr = get_workset_name_addr(idx);
    eeprom_cs(0, addr);
    eeprom_write((uint8_t*) name, WORKSET_NAME_LENGTH);
    eeprom_status_wait();
}

void workset_default(uint16_t idx)
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
    memset(name, 0x20, WORKSET_NAME_LENGTH);
    save_name(idx);
}
