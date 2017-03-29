
#include <stdint.h>
#include <limits.h>

#include "../hal.h"
#include "../dio.h"
#include "../lcd.h"
#include "../thermo.h"
#include "../misc.h"

#include "ui_main.h"
#include "ui_settings.h"
#include "ui_library.h"
#include "ui_users.h"

typedef struct
{
    int code;
    const char *msg;
}
MSG_TEXT;

int tz_delta(uint8_t ch);
char tz_state_sym(uint8_t ch);

void clear_store();

const char *get_msg_text(int code, const MSG_TEXT *text);

/* +lcdconv */

static const MSG_TEXT msg_modes[] = {
    {m_adjust, "Налад."},
    {m_manual, "Ручн. "},
    {m_semi, "П/авт."},
    {m_auto, "Авт.  "},
    {m_unknown, "Режим?"},
};

static const MSG_TEXT msg_status[] = {
    {s_idle, "Cтоп"},
    {s_done, "Готово"},
    {s_junction_slow, "Замедл. запирание"},
    {s_junction_full, "Полное запирание"},
    {s_inj_push, "Подвод  МВ"},
    {s_inject, "Впрыск"},
    {s_load, "Загрузка"},
    {s_decompression, "Декомпрессия"},
    {s_inj_pop, "Отвод МВ"},
    {s_disjunction, "Размыкание"},
};

static const MSG_TEXT msg_error[] = {
    {e_not_powered, "Силовая не включена"},
    {e_emergency_stop, "Аварийный стоп"},
    {e_not_warmed, "Не разогрет МЦ"},
    {e_guard_chk, "Проверить ограждение"},
    {e_guard_56, "Неисправность BK56"},
    {e_guard_57, "Неисправность BK57"},
    {e_engine_off, "Привод выключен"},
    {e_engine_not_ready, "Привод не готов"},
    {e_engine_overheat, "Перегрев двигателя"},
    {e_lub_low, "Проверить уров.смаз"},
};

/* -lcdconv */

static const char str_temp[] = "%4i%4i%4i%4i%4i";
static const uint8_t str_addr[] = {STR1_ADDR, STR2_ADDR, STR3_ADDR, STR4_ADDR};

static int tz_temp[TZ_MAX];
static int tz_int;
static int main_mode;
static int main_status;
static int main_error;
static int main_flags;

typedef enum
{
    scr_main,
    scr_settings,
    scr_users,
    scr_library,
    scr_return,
    scr_thermo,
    scr_inputs,
    scr_outputs,
    scr_chars
}
SCREEN;

static int screen = scr_return;

extern WORKSET workset; // GLOBAL!

void ui_task(MAIN_STATE *state)
{
    static int keydly = 0;
    static int keytmp;

    int i, j;
    uint8_t key;

    if (get_timer(TMR_UI) > 0) return;
    set_timer(TMR_UI, 100);

    key = get_key();
    while (1)
    {
        if (key != keytmp)
        {
            keytmp = key;
            keydly = 10;
            break;
        }
        if (keydly == 0) break;
        keydly--;
        if (keydly > 0)
        {
            key = 0;
            break;
        }
        keydly = 2;
        break;
    }

    switch (screen)
    {

    case scr_main:

        if (check_int(state->mode, &main_mode))
        {
            lcd_print_rom(STR1_ADDR, get_msg_text(main_mode, msg_modes));
        }

        if (check_int(state->status, &main_status))
        {
            lcd_clr_str(STR2_ADDR);
            lcd_print_rom(STR2_ADDR, get_msg_text(main_status, msg_status));
        }

        if (check_int(state->error, &main_error))
        {
            lcd_clr_str(STR4_ADDR);
            lcd_print_rom(STR4_ADDR, get_msg_text(main_error, msg_error));
        }

        if (check_int(thermo_get_int_temp(), &tz_int))
        {
            lcd_printf(STR1_ADDR + 7, "%2iC", tz_int);
        }

        if (check_int(state->flags.v, &main_flags))
        {
            lcd_set_cursor(STR1_ADDR + 11, 0);
            lcd_put_char(state->flags.f.power_on ? 'P' : '_');
            lcd_put_char(state->flags.f.engine_on ? 'M' : '_');
            lcd_put_char(state->flags.f.guard_ok ? 'S' : '_');
            lcd_put_char(state->flags.f.heat_on ? 'T' : '_');
        }

        for (i = 0; i < TZ_MAX; i++)
        {
            if (check_int(thermo_get_tz_temp(i), &tz_temp[i]))
            {
                lcd_set_cursor(STR1_ADDR + 15 + i, 0);
                lcd_put_char(tz_state_sym(i));
            }
        }

        switch (key)
        {

        case '1':

            lcd_clear();
            ui_library(0xFF);
            screen = scr_library;
            break;

        case '2':

            lcd_clear();
            ui_users(0xFF);
            screen = scr_users;
            break;


        case '7':

            lcd_clear();
            screen = scr_inputs;
            break;

        case '8':

            lcd_clear();
            screen = scr_outputs;
            lcd_set_cursor(STR1_ADDR + 2, 1);
            main_mode = 0;
            break;

        case '9':

            lcd_clear();
            lcd_print_rom(STR1_ADDR, "  T1  T2  T3  T4  T5");
            lcd_printf(STR3_ADDR, str_temp,
                       workset.temp_Z1,
                       workset.temp_Z2,
                       workset.temp_Z3,
                       workset.temp_Z4,
                       workset.temp_Z5);
            screen = scr_thermo;
            break;

        case '*':

            lcd_clear();
            ui_settings(0xFF);
            screen = scr_settings;
            break;

        case '0':

            lcd_clear();
            main_mode = 0;
            main_status = 1;
            screen = scr_chars;
            break;

        }
        break;

    case scr_inputs:

        for (j = 0; j < 4; j++)
        {
            lcd_set_cursor(str_addr[j], 0);
            lcd_put_char(0x30 + j);
            lcd_put_char(0x20);
            for (i = 0; i < 16; i++)
            {
                lcd_put_char(0x30 + dio_in((j * 16) + 15 - i));
                if (i == 7) lcd_put_char(0x20);
            }
        }
        if (key == '*') screen = scr_return;
        break;

    case scr_outputs:

        for (j = 0; j < 4; j++)
        {
            lcd_set_cursor(str_addr[j], 0);
            lcd_put_char(0x30 + j);
            lcd_put_char(0x20);
            for (i = 0; i < 16; i++)
            {
                lcd_put_char(0x30 + dio_out_state((j * 16) + 15 - i));
                if (i == 7) lcd_put_char(0x20);
            }
        }
        lcd_set_cursor(str_addr[main_mode / 16] + 2 + (15 - (main_mode % 16)), 1);

        if (key == 'A')
        {
            main_mode += 1;
            if (main_mode > 63) main_mode = 0;
        }
        if (key == 'B')
        {
            main_mode -= 1;
            if (main_mode < 0) main_mode = 63;
        }
        if (key == 'C')
        {
            main_mode -= 16;
            if (main_mode < 0) main_mode = 64 + main_mode;
        }
        if (key == 'D')
        {
            main_mode += 16;
            if (main_mode > 63) main_mode %= 16;
        }
        if (key == '#')
        {
            dio_out(main_mode, 2);
        }
        if (key == '*')
        {
            screen = scr_return;
        }
        break;


    case scr_settings:

        if (!ui_settings(key)) break;
        screen = scr_return;
        break;

    case scr_library:

        if (!ui_library(key)) break;
        screen = scr_return;
        break;

    case scr_users:

        if (!ui_users(key)) break;
        screen = scr_return;
        break;

    case scr_thermo:

        lcd_printf(STR2_ADDR, str_temp,
                   thermo_get_tz_temp(0),
                   thermo_get_tz_temp(1),
                   thermo_get_tz_temp(2),
                   thermo_get_tz_temp(3),
                   thermo_get_tz_temp(4));

        lcd_printf(STR4_ADDR, str_temp,
                   tz_delta(0),
                   tz_delta(1),
                   tz_delta(2),
                   tz_delta(3),
                   tz_delta(4));

        if (key == '*') screen = scr_return;
        break;

    case scr_chars:

        if (key == '*') screen = scr_return;
        if (key == 'A')
        {
            main_mode -= 0x10;
            if (main_mode < 0) main_mode = 0;
            main_status = 1;
        }
        if (key == 'B')
        {
            main_mode += 0x10;
            if (main_mode > 0xC0) main_mode = 0xC0;
            main_status = 1;
        }

        if (main_status == 0) break;
        main_status = 0;

        for (j = 0; j < 4; j++)
        {
            lcd_printf(str_addr[j], "%02X ", main_mode + j * 0x10);
            for (i = 0; i < 16; i++) lcd_put_char(main_mode + j * 0x10 + i);
        }
        break;

    case scr_return:

        lcd_clear();
        clear_store();
        screen = scr_main;
        break;

    }

}

int tz_delta(uint8_t ch)
{
    int d = thermo_get_tz_temp(ch);
    uint16_t *tz = &workset.temp_Z1;
    d -= *(tz + ch);
    return d;
}

char tz_state_sym(uint8_t ch)
{
    int d = thermo_get_tz_state(ch);
    if (d < 0) return 0xDA;
    if (d > 0) return 0xD9;
    return '=';
}

void clear_store()
{
    for (int i = 0; i < TZ_MAX; i++) tz_temp[i] = INT_MIN;
    tz_int = INT_MIN;
    main_mode = INT_MIN;
    main_status = INT_MIN;
    main_error = INT_MIN;
    main_flags = INT_MIN;
}

const char *get_msg_text(int code, const MSG_TEXT *text)
{
    for (int i = 0;; i++)
    {
        if (text[i].code == code) return text[i].msg;
    }
    return "Undefined";
}
