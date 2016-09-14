
#include <stdlib.h>
#include <string.h>

#include "../lcd.h"
#include "../hal.h"
#include "../helper.h"

#include "ui_library.h"
#include "ui_settings.h"

static const char str1[] = "Библиотека изделий";
static const char str4[] = "3-Зап. 6-Чтен. #-Имя";
static const char str_wr[] = "Записано";
static const char str_rd[] = "Прочитано";

static WORKSET *workset;

static uint8_t idx;
static uint8_t edit;
static char cname[20];

uint8_t ui_library_main(char key);

void ui_library(WORKSET *set)
{
    idx = 1;
    edit = 0;
    workset = set;
    if ((set->prod_name[0] > 0) && (set->prod_name[0] <= WORKSET_COUNT)) idx = set->prod_name[0];
    lcd_clear();
    ui_library_main(0xFF);
    for (;;) if (ui_library_main(get_key())) break;
    lcd_clear();
}

uint8_t ui_library_main(char key)
{
    if (key == 0) return 0;

    if (edit == 0)
    {
        if (key == '*') return 1;
        if (key == 'A') idx++;
        if (key == 'B') idx--;
        if (idx >= WORKSET_COUNT) idx = 1;
        if (idx == 0) idx = WORKSET_COUNT - 1;
		workset_load_name(idx, cname);
	    if (cname[0] == 0xFF) memset(cname, 0, prod_name_sz);
    }

    if (key == '#')
    {
        edit = !(edit > 0);
        if (edit == 0)
        {
            workset_save_name(idx, cname);
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
        if (edit > prod_name_sz) edit = 1;
        if (edit == 0) edit = prod_name_sz;
        print_name(idx, cname);
        lcd_set_cursor(STR2_ADDR + 2 + edit, 1);
        return 0;
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, str1);

    print_name(idx, cname);

    if (key == '6')
    {
        workset_load(idx, workset);
        lcd_print_rom(STR3_ADDR, str_rd);
        memset(workset->prod_name, 0, prod_name_sz);
        workset->prod_name[0] = idx;
    }

    if (key == '3')
    {
		memcpy(workset->prod_name, cname, prod_name_sz);
        workset_save(idx, workset);
        lcd_print_rom(STR3_ADDR, str_wr);
        memset(workset->prod_name, 0, prod_name_sz);
        workset->prod_name[0] = idx;
    }

    lcd_print_rom(STR4_ADDR, str4);
    return 0;
}
