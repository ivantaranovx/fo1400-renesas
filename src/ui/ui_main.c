
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "../hal.h"
#include "../dio.h"
#include "../lcd.h"
#include "../thermo.h"
#include "../fo1400.h"

#include "ui_settings.h"
#include "ui_main.h"

void ui_print(unsigned char pos, const char *fmt, ...);
char tz_state_char(uint8_t ch);
unsigned is_changed(int val, int *store);
void clear_store();

/* +lcdconv */
static const char  msg_mode_adj[]          = "Налад.";
static const char  msg_mode_hand[]         = "Ручн. ";
static const char  msg_mode_half[]         = "П/авт.";
static const char  msg_mode_auto[]         = "Авт.  ";

static const char  msg_stop[]              = "Cтоп";
static const char  msg_stop_error[]        = "Аварийный стоп";

static const char  msg_guard_56[]          = "Нет ограждения BK56";
static const char  msg_guard_57[]          = "Нет ограждения BK57";

static const char  msg_engine_on[]         = "Привод включен";
static const char  msg_engine_off[]        = "Привод выключен";
static const char  msg_engine_not_ready[]  = "Привод не готов";
static const char  msg_engine_overheat[]   = "Перегрев двигателя";

static const char  msg_slow_lock[]         = "Замедл. запирание";
static const char  msg_full_lock[]         = "Полное запирание";
static const char  msg_push_injector[]     = "Подвод  МВ";
static const char  msg_pop_injector[]      = "Отвод МВ";
static const char  msg_inject[]            = "Впрыск";
static const char  msg_load[]              = "Загрузка";
static const char  msg_decompression[]     = "Декомпрессия";
static const char  msg_disjunction[]       = "Размыкание";
static const char  msg_not_warmed[]        = "Не разогрет МЦ";
/* -lcdconv */

static const char  str_temp[]              = "% 4i % 4i % 4i % 4i % 4i";

static int tz_state[TZ_MAX];
static int tz_int;
static int main_mode;
static int main_oper;
static MAIN_FLAGS_V main_flags;

typedef enum
{
    scr_main,
    scr_settings,
    scr_return,
    scr_thermo,
    scr_inputs
}
SCREEN;

static int screen = scr_main;

void ui_task(void)
{
    
    int i, j;
    uint8_t key = get_key();
    MAIN_STATE *state = main_get_state();

    switch (screen)
    {
    case scr_main:

        if (is_changed(state->mode, &main_mode))
        {
            switch(main_mode)
            {
            case 0x01:
                lcd_print_rom(STR1_ADDR, msg_mode_adj);
                break;
            case 0x02:
                lcd_print_rom(STR1_ADDR, msg_mode_hand);
                break;
            case 0x04:
                lcd_print_rom(STR1_ADDR, msg_mode_half);
                break;
            case 0x08:
                lcd_print_rom(STR1_ADDR, msg_mode_auto);
                break;
            }
        }

        if (is_changed(thermo_get_int_temp(), &tz_int))
        {
            ui_print(STR1_ADDR + 7, "%2iC", tz_int);
        }

        j = 0;
        for(i = 0; i < TZ_MAX; i++)
        {
            if (is_changed(thermo_get_tz_state(i), &tz_state[i])) j++;
        }

        if (j)
        {
            ui_print(STR1_ADDR + 15, "%c%c%c%c%c",
                    tz_state_char(0),
                    tz_state_char(1),
                    tz_state_char(2),
                    tz_state_char(3),
                    tz_state_char(4));
        }

        if (is_changed(state->flags.v, &main_flags.v))
        {
            ui_print(STR1_ADDR + 11, "%c%c%c%c",
                main_flags.f.power_on?"P":"_",
                main_flags.f.engine_on?"M":"_",
                main_flags.f.guard_ok?"S":"_",
                main_flags.f.heat_ok?'T':'_');
        }

        if (is_changed(state->oper, &main_oper))
        {
            switch(main_oper)
            {
            case idle:
                lcd_clear();
                break;


            }
        }
        
        switch (key)
        {
        case '*':

            lcd_clear();
            ui_settings(0xFF);
            uart_send_str("Enter settings menu\r\n");
            screen = scr_settings;
            break;

        case '7':

            lcd_clear();
            screen = scr_inputs;
            break;

        case '9':

            lcd_clear();
            lcd_print_rom(STR1_ADDR, "  T1  T2  T3  T4  T5");
            screen = scr_thermo;
            break;

/*
    if (key == '1')
    {
        uart_send((uint8_t*)"Enter users menu\r\n");
        gui_users();
        uart_send((uint8_t*)"Leave users menu\r\n");
    }

    if (key == '2')
    {
        uart_send((uint8_t*)"Enter library\r\n");
        gui_library(&workset);
        uart_send((uint8_t*)"Leave library\r\n");
    }
*/

        }
        break;

    case scr_inputs:

        lcd_print_rom(STR1_ADDR, "01 ");
        for(i = 0; i < 16; i++) lcd_put_char(0x30 + dio_in(i));
        lcd_print_rom(STR2_ADDR, "17 ");
        for(i = 0; i < 16; i++) lcd_put_char(0x30 + dio_in(i + 16));
        lcd_print_rom(STR3_ADDR, "33 ");
        for(i = 0; i < 16; i++) lcd_put_char(0x30 + dio_in(i + 32));
        lcd_print_rom(STR4_ADDR, "49 ");
        for(i = 0; i < 16; i++) lcd_put_char(0x30 + dio_in(i + 48));

        if (key == '*') screen = scr_return;
        break;

    case scr_settings:

        if (!ui_settings(key)) break;
        uart_send_str("Leave settings menu\r\n");
        screen = scr_return;
        break;

    case scr_thermo:

        ui_print(STR2_ADDR, str_temp,
                    thermo_get_tz_temp(0),
                    thermo_get_tz_temp(1),
                    thermo_get_tz_temp(2),
                    thermo_get_tz_temp(3),
                    thermo_get_tz_temp(4));

        ui_print(STR3_ADDR, str_temp,
                    get_param(46),
                    get_param(47),
                    get_param(48),
                    get_param(49),
                    get_param(50));

        ui_print(STR4_ADDR, str_temp,
                    thermo_get_tz_temp(0) - get_param(46),
                    thermo_get_tz_temp(1) - get_param(47),
                    thermo_get_tz_temp(2) - get_param(48),
                    thermo_get_tz_temp(3) - get_param(49),
                    thermo_get_tz_temp(4) - get_param(50));

        if (key == '*') screen = scr_return;
        break;

    case scr_return:

        lcd_clear();
        clear_store();
        screen = scr_main;
        break;

    }

}

void ui_print(unsigned char pos, const char *fmt, ...)
{
    va_list args;
    char buf[STR_MAX_LENGTH];

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    lcd_print_rom(pos, buf);
}

char tz_state_char(uint8_t ch)
{
    if (tz_state[ch] > 0) return 0x99;
    if (tz_state[ch] < 0) return 0x9a;
    return '=';
}

unsigned is_changed(int val, int *store)
{
    if (val == *store) return 0;
    *store = val;
    return 1;
}

void clear_store()
{
    memset(tz_state, 0, TZ_MAX);
    tz_int = 0;
    main_mode = -1;
    main_oper = -1;
    main_flags.v = 0;
}

