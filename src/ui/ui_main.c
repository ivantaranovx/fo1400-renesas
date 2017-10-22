
#include "../hal.h"
#include "../dio.h"
#include "../lcd.h"
#include "../thermo.h"
#include "../misc.h"
#include "../pressure.h"

#include "ui_main.h"
#include "ui_settings.h"
#include "ui_library.h"
#include "ui_users.h"
#include "ui_lan.h"

#include <stdint.h>
#include <limits.h>

typedef struct
{
    int code;
    const char *msg;
}
MSG_TEXT;

int tz_delta(uint8_t ch);
char tz_state_sym(uint8_t ch);

const char *get_msg_text(int code, const MSG_TEXT *text);

static const MSG_TEXT msg_modes[] = {
    {m_adjust, "Налад."},
    {m_manual, "Ручн. "},
    {m_semi, "П/авт."},
    {m_auto, "Авт.  "},
    {m_unknown, "Режим?"},
};

static const MSG_TEXT msg_status[] = {
    {s_idle, "Cтоп  "},
    {s_cycle, " Цикл "},
    {s_cycle_t, "-Цикл-"},
    {s_done, "Готово"},
    {s_pause, "Пауза "},
    {s_none, "      "},
    {s_junction, "Смык. "},
    {s_junction_slow, "Смык.М"},
    {s_junction_full, "Смык.П"},
    {s_inj_push, "Подвод"},
    {s_inject, "Впрыск"},
    {s_load, "Загруз"},
    {s_decompression, "Декомп"},
    {s_inj_pop, "Отвод"},
    {s_disjunction, "Размык"},
    {s_junction_break, "Страг."},
    {s_junction_fast, "Ускор."},
    {s_junction_prev, "Пред. "},
    {s_junction_lock, "Запир."},
    {s_inject_1, "1 ступ"},
    {s_inject_2, "2 ступ"},
    {s_form_h, "Форм.B"},
    {s_form_l, "Форм.H"},
    {s_cooling, "Охлаж."},
    {s_disjunction_break, "Отрыв "},
    {s_disjunction_fast, "Ускор."},
    {s_disjunction_slow, "Замедл"},
    {s_jump, "Подс-к"},
};

static const MSG_TEXT msg_error[] = {
    {e_success, ""},
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
    {e_err_bk1, "Ошибка BK1"},
    {e_err_bk2, "Ошибка BK2"},
    {e_err_bk20, "Ошибка BK20"},
    {e_err_bk21, "Ошибка BK21"},
    {e_err_bk22, "Ошибка BK22"},
    {e_err_bk23, "Ошибка BK23"},
    {e_err_bk25, "Ошибка BK25"},
    {e_err_bk53, "Ошибка BK53"},
    {e_err_tmr1, "Ошибка TMR1"},
    {e_err_tmr2, "Ошибка TMR2"},
    {e_err_tmr3, "Ошибка TMR3"},
    {e_err_tmr6, "Ошибка TMR6"},
    {e_err_tmr8, "Ошибка TMR8"},
};

static const char str_temp[] = "%4i%4i%4i%4i%4i";
static const uint8_t str_addr[] = {STR1_ADDR, STR2_ADDR, STR3_ADDR, STR4_ADDR};
static const char str_noerr[] = "Нет ошибок";
static const char str_nosel[] = "Не выбрано";

static const TMR_SCALE_NUM tmrs[3] = {TMR_SCALE_D0, TMR_SCALE_D1, TMR_SCALE_D2};

static int tz_temp[TZ_MAX];
static int tz_int;
static int main_mode;
static int main_status;
static int main_flags;
static int main_err[4];
static int stat[3];
static uint32_t tmr[3];
static uint16_t prod_id;
static uint16_t prod_cnt;
static int edit = 0;

typedef enum
{
    scr_main,
    scr_settings,
    scr_users,
    scr_library,
    scr_return,
    scr_thermo,
    scr_analog,
    scr_ipaddr,
    scr_inputs,
    scr_outputs,
    scr_chars,
    scr_errors,
}
SCREEN;

static int screen = scr_return;

extern WORKSET workset; // GLOBAL!

void clr_stat_tmr(void);

void ui_task(MAIN_STATE *state)
{
    static int keydly = 0;
    static int keytmp;

    int i, j;
    uint8_t key;

    if (get_timer(TMR_UI) > 0) return;
    set_timer(TMR_UI, 100);

    key = get_key();
    for (;;) // autorepeat
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
            clr_stat_tmr();
        }

        if (check_int(thermo_get_int_temp(), &tz_int))
        {
            lcd_printf(STR1_ADDR + 7, "%2i", tz_int);
        }

        state->flags.f.error = false;
        for (i = 0; i < 4; i++)
        {
            if (state->err[i] != e_success)
            {
                state->flags.f.error = true;
                break;
            }
        }

        if (check_int(state->flags.v, &main_flags))
        {
            lcd_set_cursor(STR1_ADDR + 10, 0);
            lcd_put_char(state->flags.f.error ? 'E' : '_');
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

        for (i = 0; i < 3; i++)
        {
            if (check_int(state->stat[i], &stat[i]))
                lcd_print_rom(STR2_ADDR + (i * 7), get_msg_text(stat[i], msg_status));

            if (state->mode == m_unknown) continue;

            if (i == 0)
            {
                if (state->mode == m_adjust) continue;
                if (state->mode == m_manual) continue;
            }

            if (check_uint32(get_scale_timer(tmrs[i]), &tmr[i]))
                print_uint(STR3_ADDR + (i * 7), tmr[i] / 100, 4);
        }

        if (check_uint16(state->job.id, &prod_id))
        {
            char name[WORKSET_NAME_LENGTH + 1];
            load_name(prod_id, name);
            // WORKSET_NAME_LENGTH
            lcd_printf(STR4_ADDR, "%14.14s", name);
        }

        if (check_uint16(state->job.count, &prod_cnt))
            print_uint(STR4_ADDR + 15, prod_cnt, 0);

        if (edit)
        {
            i = ui_input_int_process(key);
            if (i > 0)
            {
                if (i == 2) ui_input_int_get(&state->job.count);
                edit = 0;
                prod_cnt = state->job.count ^ 1;
            }
            break;
        }

        switch (key)
        {
        case '1':

            lcd_clear();
            ui_users('R');
            screen = scr_users;
            break;

        case '2':

            lcd_clear();
            ui_library('R');
            screen = scr_library;
            break;

        case '4':

            lcd_clear();
            screen = scr_analog;
            break;

        case '5':

            lcd_clear();
            ui_ipaddr('R');
            screen = scr_ipaddr;
            break;

        case '7':

            lcd_clear();
            screen = scr_inputs;
            break;

        case '8':
            if (state->mode != m_adjust) break;
            lcd_clear();
            screen = scr_outputs;
            main_mode = 0;
            lcd_set_cursor(STR1_ADDR + 2, 1);
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
            ui_settings('R');
            screen = scr_settings;
            break;

        case '0':

            lcd_clear();
            main_mode = 0;
            main_status = 1;
            screen = scr_chars;
            break;

        case 'A':
            lcd_clear();
            screen = scr_errors;
            break;

        case '#':
            if (state->flags.f.cycle_run) break;
            ui_input_int(STR4_ADDR + 15, state->job.count, 0);
            edit = 1;
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

    case scr_analog:

        lcd_clr_str(STR1_ADDR);
        lcd_printf(STR1_ADDR, "P1 %05.1f кгс", get_pressure_kg(0));
        lcd_printf(STR2_ADDR, "P2 %05.1f кгс", get_pressure_kg(1));
        lcd_printf(STR3_ADDR, "Y1 %*ld", 11, get_enc(0));
        lcd_printf(STR4_ADDR, "Y2 %*ld", 11, get_enc(1));

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
        i = (15 - (main_mode % 16));
        if (i > 7) i++;
        lcd_set_cursor(str_addr[main_mode / 16] + 2 + i, 1);

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
            dio_out_reset();
            screen = scr_return;
        }
        break;


    case scr_ipaddr:

        if (!ui_ipaddr(key)) break;
        screen = scr_return;
        break;

    case scr_settings:

        if (!ui_settings(key)) break;
        screen = scr_return;
        break;

    case scr_library:

        i = ui_library(key);
        if (i == 0) break;
        if (i > 0)
        {
            state->job.id = i;
            state->job.count = 0;
        }
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

    case scr_errors:

        if (key == '*')
        {
            screen = scr_return;
            break;
        }
        for (i = 0; i < 4; i++)
        {
            if (!check_int(state->err[i], &main_err[i])) continue;
            lcd_clr_str(str_addr[i]);
            lcd_print_rom(str_addr[i], get_msg_text(main_err[i], msg_error));
        }
        break;

    case scr_return:

        for (i = 0; i < TZ_MAX; i++) tz_temp[i] = INT_MIN;
        for (i = 0; i < 4; i++) main_err[i] = INT_MIN;
        tz_int = INT_MIN;
        main_mode = INT_MIN;
        main_status = INT_MIN;
        main_flags = INT_MIN;
        prod_id = UINT16_MAX;
        prod_cnt = state->job.count ^ 1;
        clr_stat_tmr();

        lcd_clear();
        screen = scr_main;
        break;

    }

}

void clr_stat_tmr(void)
{
    for (int i = 0; i < 3; i++)
    {
        stat[i] = INT_MIN;
        tmr[i] = UINT32_MAX;
    }
    lcd_clr_str(STR2_ADDR);
    lcd_clr_str(STR3_ADDR);
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

const char *get_msg_text(int code, const MSG_TEXT *text)
{
    for (int i = 0; text[i].msg > 0; i++)
    {
        if (text[i].code == code) return text[i].msg;
    }
    return "Undefined";
}
