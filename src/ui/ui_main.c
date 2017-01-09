
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "../hal.h"
#include "../dio.h"
#include "../lcd.h"
#include "../thermo.h"
#include "../fo1400_states.h"

#include "ui_settings.h"
#include "ui_main.h"

typedef struct
{
    int code;
    const char *msg;
}
MSG_TEXT;

void ui_print(unsigned char pos, const char *fmt, ...);
char tz_state_char(uint8_t ch);
unsigned is_changed(int val, int *store);
void clear_store();

const char *get_msg_text(int code, const MSG_TEXT *text);

/* +lcdconv */

static const char str_m_adjust[]   = "Налад.";
static const char str_m_manual[]   = "Ручн. ";
static const char str_m_semi[]     = "П/авт.";
static const char str_m_auto[]     = "Авт.  ";
static const char str_m_unknown[]  = "Режим?";

static const char str_o_idle[]                  = "Cтоп";
static const char str_o_junction_start[]        = "Замедл. запирание";
static const char str_o_junction_full[]         = "Полное запирание";
static const char str_o_inj_push_start[]        = "Подвод  МВ";
static const char str_o_inject_start[]          = "Впрыск";
static const char str_o_load_start[]            = "Загрузка";
static const char str_o_decompression_start[]   = "Декомпрессия";
static const char str_o_inj_pop_start[]         = "Отвод МВ";
static const char str_o_disjunction_start[]     = "Размыкание";

static const char str_e_emergency_stop[]    = "Аварийный стоп";
static const char str_e_not_warmed[]        = "Не разогрет МЦ";
static const char str_e_guard_56[]          = "Нет ограждения BK56";
static const char str_e_guard_57[]          = "Нет ограждения BK57";
static const char str_e_engine_off[]        = "Привод выключен";
static const char str_e_engine_not_ready[]  = "Привод не готов";
static const char str_e_engine_overheat[]   = "Перегрев двигателя";

/* -lcdconv */

static const MSG_TEXT msg_modes[] = 
{
    {m_adjust, str_m_adjust},
    {m_manual, str_m_manual},
    {m_semi, str_m_semi},
    {m_auto, str_m_auto},
    {m_unknown, str_m_unknown},
};

static const MSG_TEXT msg_oper[] =
{
    {o_idle, str_o_idle},

    {o_junction_start, str_o_junction_start},
    {o_junction_full, str_o_junction_full},
    {o_inj_push_start, str_o_inj_push_start},
    {o_inject_start, str_o_inject_start},
    {o_load_start, str_o_load_start},
    {o_decompression_start, str_o_decompression_start},
    {o_inj_pop_start, str_o_inj_pop_start},
    {o_disjunction_start, str_o_disjunction_start},
};

static const MSG_TEXT msg_error[] =
{
    {e_emergency_stop, str_e_emergency_stop},
    {e_not_warmed, str_e_not_warmed},
    {e_guard_56, str_e_guard_56},
    {e_guard_57, str_e_guard_57},
    {e_engine_off, str_e_engine_off},
    {e_engine_not_ready, str_e_engine_not_ready},
    {e_engine_overheat, str_e_engine_overheat},
};

static const char str_temp[] = "%4i%4i%4i%4i%4i";
static const uint8_t str_addr[] = {STR1_ADDR, STR2_ADDR, STR3_ADDR, STR4_ADDR};

static int tz_state[TZ_MAX];
static int tz_int;
static int main_mode;
static int main_oper;
static int main_error;
static int main_flags;

typedef enum {
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

void ui_task(void) {

    int i, j;
    MAIN_STATE *state;
    uint8_t key = get_key();

    switch (screen) {

        case scr_main:

            state = main_get_state();

            if (is_changed(state->mode, &main_mode)) {

                lcd_print_rom(STR1_ADDR, get_msg_text(main_mode, msg_modes));
            }

            if (is_changed(state->oper, &main_oper)) {

                lcd_print_rom(STR2_ADDR, get_msg_text(main_oper, msg_oper));
            }

            if (is_changed(state->error, &main_error)) {

                lcd_print_rom(STR3_ADDR, get_msg_text(main_error, msg_error));
            }

            if (is_changed(thermo_get_int_temp(), &tz_int)) {

                ui_print(STR1_ADDR + 7, "%2iC", tz_int);
            }

            if (is_changed(state->flags.v, &main_flags)) {

                lcd_set_cursor(STR1_ADDR + 11, 0);
                lcd_put_char(state->flags.f.power_on ? 'P' : '_');
                lcd_put_char(state->flags.f.engine_on ? 'M' : '_');
                lcd_put_char(state->flags.f.guard_ok ? 'S' : '_');
                lcd_put_char(state->flags.f.heat_ok ? 'T' : '_');
            }

            for (i = 0; i < TZ_MAX; i++) {
                if (is_changed(thermo_get_tz_state(i), &tz_state[i])) {

                    lcd_set_cursor(STR1_ADDR + 15 + i, 0);
                    lcd_put_char(tz_state_char(i));
                }
            }

            switch (key) {

                case '1':

                    lcd_clear();
                    screen = scr_library;
                    break;

                case '2':

                    lcd_clear();
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
                    main_oper = 1;
                    screen = scr_chars;
                    break;

            }
            break;

        case scr_inputs:

            for (j = 0; j < 4; j++) {
                lcd_set_cursor(str_addr[j], 0);
                lcd_put_char(0x30 + j);
                lcd_put_char(0x20);
                for (i = 0; i < 16; i++) lcd_put_char(0x30 + dio_in((j * 16) + 15 - i));
            }
            if (key == '*') screen = scr_return;
            break;

        case scr_outputs:

            for (j = 0; j < 4; j++) {
                lcd_set_cursor(str_addr[j], 0);
                lcd_put_char(0x30 + j);
                lcd_put_char(0x20);
                for (i = 0; i < 16; i++) lcd_put_char(0x30 + dio_out_state((j * 16) + 15 - i));
            }
            lcd_set_cursor(str_addr[main_mode / 16] + 2 + (15 - (main_mode % 16)), 1);

            if (key == 'A') {
                main_mode += 1;
                if (main_mode > 63) main_mode = 0;
            }
            if (key == 'B') {
                main_mode -= 1;
                if (main_mode < 0) main_mode = 63;
            }
            if (key == 'C') {
                main_mode -= 16;
                if (main_mode < 0) main_mode = 64 + main_mode;
            }
            if (key == 'D') {
                main_mode += 16;
                if (main_mode > 63) main_mode %= 16;
            }
            if (key == '#') {
                dio_out(main_mode, 2);
            }
            if (key == '*') {
                screen = scr_return;
            }
            break;


        case scr_settings:

            if (!ui_settings(key)) break;
            screen = scr_return;
            break;

        case scr_library:

            // if (!ui_library(key)) break;
            screen = scr_return;
            break;

        case scr_users:

            //if (!ui_users(key)) break;
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

        case scr_chars:

            if (key == '*') screen = scr_return;
            if (key == 'A') {
                main_mode -= 0x10;
                if (main_mode < 0) main_mode = 0;
                main_oper = 1;
            }
            if (key == 'B') {
                main_mode += 0x10;
                if (main_mode > 0xC0) main_mode = 0xC0;
                main_oper = 1;
            }

            if (main_oper == 0) break;
            main_oper = 0;

            for (j = 0; j < 4; j++) {
                ui_print(str_addr[j], "%02X ", main_mode + j * 0x10);
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

void ui_print(unsigned char pos, const char *fmt, ...) {

    va_list args;
    char buf[STR_MAX_LENGTH + 1];

    va_start(args, fmt);
    vsnprintf(buf, sizeof (buf), fmt, args);
    va_end(args);

    lcd_print_rom(pos, buf);
}

char tz_state_char(uint8_t ch) {

    if (tz_state[ch] == 0) return '=';
    if (tz_state[ch] > 0) return 0xD9;
    return 0xDA;
}

unsigned is_changed(int val, int *store) {

    if (val == *store) return 0;
    *store = val;
    return 1;
}

void clear_store() {

    for (int i = 0; i < TZ_MAX; i++) tz_state[i] = -1;
    tz_int = -1;
    main_mode = -1;
    main_oper = -1;
    main_error = -1;
    main_flags = -1;
}

const char *get_msg_text(int code, const MSG_TEXT *text)
{
    int i;

    for(i = 0;;i++)
    {
        if (text[i].code == 0) break;
        if (text[i].code == code) break;
    }
    return text[i].msg;
}

