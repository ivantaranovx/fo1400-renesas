
#include <stdlib.h>
#include <string.h>

#include "../lcd.h"
#include "../hal.h"
#include "../24lc512.h"
#include "../helper.h"

#include "ui_settings.h"

typedef struct
{
    char *name;
    char *unit1;
    char *unit2;
}
UNIT_PARAM;

// submenu`s in menu
const uint8_t main_menu[] =
{
    7, 11, 19, 16, 5, 5, 5, 3
};

static const UNIT_PARAM main_menu_items[] =
{
    {"Настройка общие",			0, 0},
    {"Настройка таймеров",		0, 0},
    {"Настройка гидроагр.",		0, 0},
    {"Настройка температ.",		0, 0},
    {"Настройка ШИМ",			0, 0},
    {"Настройка давления",		0, 0},
    {"Логика завершения",		0, 0},

    {"Счетчик изделий",			"шт.", 0},
    {"Отвод сопла",			"с отводом", "без отвода"},
    {"Пауза контр.",			"мс.", 0},
    {"Подскок",				"с подскоком", "без подскока"},
    {"Опорожнение формы",		"с опорож.", "без опорож."},
    {"Период смазки",			"мин.", 0},
    {"Период перез.см.",		"мин.", 0},
    {"Число толчков смазки",            "раз", 0},
    {"Время толчка",			"сек.", 0},
    {"Превышение темп.",		"град.", 0},
    {"Доп.недогрев",			"град.", 0},

    {"Т01 Время цикла",			"сек.", 0},
    {"Т02 Предохранение",		"сек.", 0},
    {"Т03 Впрыск",			"сек.", 0},
    {"Т04 Формование",			"сек.", 0},
    {"Т05 Охлаждение",			"сек.", 0},
    {"Т06 Загрузка",			"сек.", 0},
    {"Т07 Задержка впрыска",            "сек.", 0},
    {"Т08 Медл.размыкание",		"сек.", 0},
    {"Т09 Пауза",			"сек.", 0},
    {"Т10 Форм.высок.",			"сек.", 0},
    {"Т11 Декомпрессия",		"сек.", 0},
    {"Т12 Время опорожнен.",            "сек.", 0},
    {"Т13 Зад.пнев.1",			"сек.", 0},
    {"Т14 Врем.пнев.1",			"сек.", 0},
    {"Т15 Зад.пнев.2",			"сек.", 0},
    {"Т16 Врем.пнев.2",			"сек.", 0},
    {"Т17 Зад.пнев.3",			"сек.", 0},
    {"Т18 Врем.пнев.3",			"сек.", 0},
    {"Т19 Гидрот.",			"сек.", 0},

    {"U01 Страгивание",			"об.мин.", 0},
    {"U02 Запирание уск.",		"об.мин.", 0},
    {"U03 Предохранение",		"об.мин.", 0},
    {"U04 Запирание",			"об.мин.", 0},
    {"U05 Размыкание",			"об.мин.", 0},
    {"U06 Подвод МВ",			"об.мин.", 0},
    {"U07 1 ст. впрыска",		"об.мин.", 0},
    {"U08 2 ст. впрыска",		"об.мин.", 0},
    {"U09 Формование",			"об.мин.", 0},
    {"U10 Загрузка",			"об.мин.", 0},
    {"U11 Декомпрессия",		"об.мин.", 0},
    {"U12 Размык. ускор.",		"об.мин.", 0},
    {"U13 Отвод МВ",			"об.мин.", 0},
    {"U14 Размык. отрыва",		"об.мин.", 0},
    {"U15 Выс.давление",		"об.мин.", 0},
    {"U16 Подскок",			"об.мин.", 0},

    {"Зона 1",				"град.", 0},
    {"Зона 2",				"град.", 0},
    {"Зона 3",				"град.", 0},
    {"Зона 4",				"град.", 0},
    {"Зона 5",				"град.", 0},

    {"Зона 1",				"проц.", 0},
    {"Зона 2",				"проц.", 0},
    {"Зона 3",				"проц.", 0},
    {"Зона 4",				"проц.", 0},
    {"Зона 5",				"проц.", 0},

    {"1 ст. впрыска",			"кгс", 0},
    {"2 ст. впрыска",			"кгс", 0},
    {"Формование выс.",			"кгс", 0},
    {"Формование низ.",			"кгс", 0},
    {"Подпор",				"кгс", 0},

    {"конец 1 ст. впрыска",		"", 0},
    {"конец 2 ст. впрыска",		"", 0},
    {"Формование выс.",			"", 0}
};

WORKSET *workset;
static uint8_t menu = 1;
static uint8_t submenu = 1;
static uint8_t edit = 0;

static uint16_t cval;
static uint16_t cmin;
static uint16_t cmax;

unsigned workset_load_name(uint8_t num, char *name)
{
    uint16_t addr = num;
    if (num > WORKSET_COUNT) return 0;
    addr *= sizeof(WORKSET);
    addr += ADDR_WORKSET;
    return readArray(addr, (uint8_t*)name, prod_name_sz) == prod_name_sz;
}

unsigned workset_save_name(uint8_t num, char *name)
{
    uint16_t addr = num;
    if (num > WORKSET_COUNT) return 0;
    addr *= sizeof(WORKSET);
    addr += ADDR_WORKSET;
    return writeArray(addr, (uint8_t*)name, prod_name_sz) == prod_name_sz;
}

unsigned workset_save_param(uint8_t num, uint8_t idx, uint16_t val)
{
    uint16_t addr = num;
    if (num > WORKSET_COUNT) return 0;
    addr *= sizeof(WORKSET);
    addr += prod_name_sz;
    idx *= sizeof(uint16_t);
    addr += idx;
    addr += ADDR_WORKSET;
    return writeArray(addr, (uint8_t*)&val, sizeof(uint16_t)) == sizeof(uint16_t);
}

uint8_t get_idx(uint8_t menu, uint8_t submenu);

void ui_settings_set_workset(WORKSET *set)
{
    workset = set;
}

int ui_settings(char key)
{
    char buf[10];
    uint8_t idx, i, j;

    if (key == 0) return 0;

    if (edit == 0)
    {
        if (key == '*')
        {
            edit = 0;
            return 1;
        }

        if (key == 'A')
        {
            menu++;
            submenu = 1;
        }
        if (key == 'B')
        {
            menu--;
            submenu = 1;
        }
        if (menu > main_menu[0]) menu = 1;
        if (menu == 0) menu = main_menu[0];

        if (key == 'C') submenu++;
        if (key == 'D') submenu--;
        if (submenu > main_menu[menu]) submenu = 1;
        if (submenu == 0) submenu = main_menu[menu];
    }

    if (key == '#')
    {
        edit = !(edit > 0);
        if ((cmin == 0) && (cmax == 1))
        {
            cval = !(cval > 0);
            edit = 0;
        }
        if (edit == 0)
        {
            idx = get_idx(menu, submenu);
            check_param(idx, &cval);
            set_param(idx, cval);
            workset_save_param(0, idx, cval);
        }
    }

    if (edit)
    {
        if ((key >= '0') && (key <= '9'))
        {
            utoa(cval, buf, 0);
            buf[edit - 1] = key;
            cval = (uint16_t)atol(buf);
            edit++;
            if (edit > 5) edit = 1;
        }
        lcd_uint(STR3_ADDR + 2, cval, (menu == 2) ? 4 : 0);
        i = edit + 1;
        if ((menu == 2) && (edit > 3)) i = edit + 2;
        lcd_set_cursor(STR3_ADDR + i, 1);
        return 0;
    }

    lcd_clear();
    lcd_print(STR1_ADDR, main_menu_items[menu - 1].name);

    i = get_idx(menu, submenu);	// index of parameter
    j = main_menu[0] + i;	// index of parameter name
    lcd_print(STR2_ADDR + 1, main_menu_items[j].name);

    cval = get_param(i);
    get_param_limits(i, &cmin, &cmax);

    if ((cmin == 0) && (cmax == 1))
    {
        lcd_print(STR3_ADDR + 2, (cval == 0) ? main_menu_items[j].unit1 : main_menu_items[j].unit2);
    }
    else
    {
        if (menu == 2) // dot in param
        {
            lcd_uint(STR3_ADDR + 2, cval, 4);
            lcd_print(STR3_ADDR + 9, main_menu_items[j].unit1);
        }
        else
        {
            lcd_uint(STR3_ADDR + 2, cval, 0);
            lcd_print(STR3_ADDR + 8, main_menu_items[j].unit1);
        }
    }

    lcd_print(STR4_ADDR + 0, "от");
    lcd_uint(STR4_ADDR + 3, cmin, (menu == 2) ? 4 : 0);
    lcd_print(STR4_ADDR + 10, "до");
    lcd_uint(STR4_ADDR + 13, cmax, (menu == 2) ? 4 : 0);

    return 0;
}

uint8_t get_idx(uint8_t menu, uint8_t submenu)
{
    uint8_t i = 0;
    uint8_t j;

    for (j = 1; j < menu; j++) i += main_menu[j];
    return i + submenu - 1;
}

uint16_t *pptr(uint8_t idx)
{
    uint16_t *ptr;
    if (idx >= WORKSET_PARAM_COUNT) return 0;
    ptr = &workset->prod_count;
    ptr += idx;
    return ptr;
}

uint16_t get_param(uint8_t idx)
{
    uint16_t *ptr;
    ptr = pptr(idx);
    if (ptr == 0) return 0;
    return *ptr;
}

void set_param(uint8_t idx, uint16_t val)
{
    uint16_t *ptr;
    ptr = pptr(idx);
    if (ptr == 0) return;
    *ptr = val;
}
