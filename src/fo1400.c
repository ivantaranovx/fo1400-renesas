
#include <stdio.h>
#include <math.h>

#include "fo1400.h"

#include "dio.h"
#include "thermo.h"
#include "hal.h"
#include "lcd.h"
#include "workset.h"
#include "helper.h"

#include "ui/ui_main.h"
#include "ui/ui_settings.h"

static WORKSET workset;
static MAIN_STATE state;
static int guard_state = 0;

#define f_cycle_stop    state.flags.f.cycle_stop
#define f_heat_ok       state.flags.f.heat_ok
#define f_power_on      state.flags.f.power_on
#define f_guard_ok      state.flags.f.guard_ok
#define f_engine_on     state.flags.f.engine_on

#define NEXT_OPER(x)    state.oper = x
#define SET_ERROR(x)    state.error = x

//static uint16_t tmr[19];

int main(void) {

    brd_init();
    uart_send_str("Harware init complete\r\n");

    dio_init();
    uart_send_str("Peripheral bus init complete\r\n");

    set_timer(0, 100);
    while (get_timer(0) > 0);

    while (1) {
        uart_send_str("LCD init...");
        if (lcd_init()) break;
        uart_send_str("fail\r\n");
    }
    uart_send_str("done\r\n");

    uart_send_str("Board: ");
    uart_send_str(brd_id());
    uart_send_str("\r\n");

    uart_send_str("Machine: ");
    uart_send_str(msg_machine);
    uart_send_str("\r\n");

    uart_send_str("Version: ");
    uart_send_str(msg_version);
    uart_send_str("\r\n");

    workset_load(0, &workset);
    thermo_set_workset(&workset);
    ui_settings_set_workset(&workset);

    f_cycle_stop = 1;
    f_power_on = 0;
    f_guard_ok = 0;
    f_engine_on = 0;
    f_heat_ok = 0;

    state.mode = m_unknown;
    SET_ERROR(e_success);
    NEXT_OPER(o_idle);

    RDY(ON);

    lcd_clear();
    uart_send_str("Running\r\n");

    for (;;) {

        check_mode_selector();
        check_guard();

        if (KH14 ^ thermo_heat_enabled()) {
            thermo_heat_enable(KH14);
        }

        main_task();
        thermo_task();
        engine_task();
        lub_task();

        f_heat_ok = thermo_heat_ok();
        f_power_on = KM1;
        f_guard_ok = (guard_state == 3);
        f_engine_on = ENGINE_STATE;

        dio_task();
        ui_task();
    }
}

int check_heat(void) {

    if (f_heat_ok) return 0;
    SET_ERROR(e_not_warmed);
    NEXT_OPER(o_idle);
    return 1;
}

MAIN_STATE *main_get_state(void) {

    return &state;
}

void all_em_off(void) {

    EM18(OFF);
    EM29(OFF);
    EM16(OFF);
    EM1(OFF);
    EM31(OFF);
    EM13(OFF);
    EM30(OFF);
    EM4(OFF);

    EM3(OFF);
    EM6(OFF);
    EM5(OFF);
    EM12(OFF);
    EM7(OFF);
    EM2(OFF);
    EM40(OFF);
    EM41(OFF);
}

void set_hydro(uint16_t speed) {

    double dac = speed;
    dac /= 9.8039;
    if (dac > 255) dac = 255;
    _set_dac(0, (uint8_t)round(dac));
}

void check_mode_selector(void)
{
    int j = 0;
    int i = m_unknown;

    if (M_ADJUST) {
        i = m_adjust;
        j++;
    }
    if (M_MANUAL) {
        i = m_manual;
        j++;
    }
    if (M_SEMI_AUTO) {
        i = m_semi;
        j++;
    }
    if (M_AUTO) {
        i = m_auto;
        j++;
    }
    if (j > 1) return;
    if (i == state.mode) return;
    state.mode = i;
    guard_state = 0;
    f_cycle_stop = 1;
}

void check_guard(void)
{
    if (!BK56 && !BK57 && (guard_state == 0)) guard_state = 1;
    if (!BK56 && BK57 && (guard_state == 1)) guard_state = 2;
    if (guard_state < 3) return;
    if (BK56 && BK57)
    {
        if (guard_state == 2) guard_state = 3;
        return;
    }
    guard_state = 0;
    NEXT_OPER(o_guard_stop);
}

void engine_task(void) {

    static int engine_state = 0;

    switch (engine_state) {
        case 0:
            if (KH10) break;
            if (!KM1) break;
            if (KH12) engine_state = 10;
            if (KH13) {
                SET_ERROR(e_engine_off);
                engine_state = 20;
                break;
            }
            if (OT) {
                f_cycle_stop = 1;
                set_hydro(0);
                ENGINE(OFF);
                SET_ERROR(e_engine_overheat);
                engine_state = 20;
            }
            break;
        case 10:
            KM10(ON);
            set_timer(TMR_ENGINE, 3000);
            engine_state = 11;
            break;
        case 11:
            if (get_timer(TMR_ENGINE)) break;
            KM5(ON);
            set_timer(TMR_ENGINE, 2000);
            engine_state = 12;
            break;
        case 12:
            if (RD) {
                ENGINE(ON);
                guard_state = 0;
                engine_state = 0;
                break;
            }
            if (get_timer(TMR_ENGINE)) break;
            SET_ERROR(e_engine_not_ready);
            engine_state = 20;
            break;
        case 20:
            KM5(OFF);
            KM10(OFF);
            engine_state = 21;
            break;
        case 21:
            if (RD) break;
            engine_state = 0;
            break;
    }

}

void lub_task(void) {

    static int lub_period;
    static int lub_push_count;
    static int lub_state = 0;

    switch (lub_state) {
        case 0:
            if (KH10) break;
            if (!KM1) break;
            if (!f_engine_on)
            {
                lub_period = workset.lub_reload * 60;
                break;
            }
            if (KH11) {
                lub_push_count = 1;
                lub_state = 2;
                break;
            }
            if (get_timer(TMR_LUB) > 0) break;
            set_timer(TMR_LUB, 1000);
            lub_period--;
            if (lub_period > 0) break;
            lub_push_count = workset.lub_count;
            lub_state = 2;
            break;
        case 2:
            if (BK58) {
                lub_state = 8;
                break;
            }
            set_timer(TMR_LUB, workset.lub_time * 1000);
            KM7(ON);
            lub_state = 4;
            break;
        case 4:
            if (get_timer(TMR_LUB)) break;
            KM7(OFF);
            set_timer(TMR_LUB, 1000);
            lub_state = 6;
            break;
        case 6:
            if (get_timer(TMR_LUB)) break;
            lub_state = 2;
            lub_push_count--;
            if (lub_push_count == 0) lub_state = 8;
            break;
        case 8:
            if (KH11) break;
            lub_period = workset.lub_period * 60;
            lub_state = 0;
            break;
    }
}

void main_task(void) {

    int i;
    
    if (KH10 /*&& state.oper != o_idle*/) NEXT_OPER(o_emergency_stop);

    switch (state.oper) {

        case o_idle:

            f_cycle_stop = 0;

            if (state.mode == m_adjust) {

                if (KH1 && !BK4) {
                    SET_ERROR(e_success);
                    NEXT_OPER(o_junction_start);
                    break;
                }
                if (KH3 && !BK22) {
                    SET_ERROR(e_success);
                    NEXT_OPER(o_inj_push_start);
                    break;
                }
                if (KH5 && !BK23) {
                    SET_ERROR(e_success);
                    NEXT_OPER(o_inject_start);
                    break;
                }
                if (KH6 && !BK25) {
                    SET_ERROR(e_success);
                    NEXT_OPER(o_load_start);
                    break;
                }
                if (KH7 && !BK20) {
                    SET_ERROR(e_success);
                    NEXT_OPER(o_decompression_start);
                    break;
                }
                if (KH4 && !BK21) {
                    SET_ERROR(e_success);
                    NEXT_OPER(o_inj_pop_start);
                    break;
                }
                if (KH2 && !BK1) {
                    SET_ERROR(e_success);
                    NEXT_OPER(o_disjunction_start);
                    break;
                }
            }
            break;

        case o_junction_start:
            set_hydro(workset.hyd_U01);
            EM16(ON);
            EM3(ON);
            EM1(ON);
            EM31(ON);
            EM29(ON);
            NEXT_OPER(o_junction_full);
            break;

        case o_junction_full:
            if (!KH1) NEXT_OPER(o_operation_end);
            if (!BK4) break;
            set_hydro(workset.hyd_U01);
            EM29(OFF);
            NEXT_OPER(o_junction_end);
            break;

        case o_junction_end:
            if (!KH1) NEXT_OPER(o_operation_end);
            if (!BK2) break;
            NEXT_OPER(o_operation_end);
            break;

        case o_inj_push_start:
            set_hydro(workset.hyd_U06);
            EM16(ON);
            EM5(ON);
            EM1(ON);
            EM31(ON);
            NEXT_OPER(o_inj_push_end);
            break;

        case o_inj_push_end:
            if (!KH3) NEXT_OPER(o_operation_end);
            if (!BK22) break;
            NEXT_OPER(o_operation_end);
            break;

        case o_inject_start:
            if (check_heat()) break;
            set_hydro(workset.hyd_U07);
            EM16(ON);
            EM7(ON);
            EM1(ON);
            EM31(ON);
            clr_scale_timer(TMR_SCALE_INJECT);
            uart_send_str("Injection start\r\n");
            NEXT_OPER(o_inject_process);
            break;

        case o_inject_process:
            uart_send_str("Injection x:");
            uart_uint(get_scale_timer(TMR_SCALE_INJECT));
            uart_send_str(", y:");
            uart_uint(rand());
            uart_send_str("\r\n");
            if (!KH5) NEXT_OPER(o_operation_end);
            if (BK23) NEXT_OPER(o_operation_end);
            if (state.oper != o_inject_process) uart_send_str("Injection stop\r\n");
            break;

        case o_load_start:
            if (check_heat()) break;
            set_hydro(workset.hyd_U10);
            EM16(ON);
            EM2(ON);
            EM13(ON);
            EM1(ON);
            EM31(ON);
            NEXT_OPER(o_load_end);
            break;

        case o_load_end:
            if (!KH6) NEXT_OPER(o_operation_end);
            if (!BK25) break;
            NEXT_OPER(o_operation_end);
            break;

        case o_decompression_start:
            if (check_heat()) break;
            set_hydro(workset.hyd_U11);
            EM16(ON);
            EM29(ON);
            EM12(ON);
            EM1(ON);
            EM31(ON);
            NEXT_OPER(o_decompression_end);
            break;

        case o_decompression_end:
            if (!KH7) NEXT_OPER(o_operation_end);
            if (!BK20) break;
            NEXT_OPER(o_operation_end);
            break;

        case o_inj_pop_start:
            set_hydro(workset.hyd_U13);
            EM16(ON);
            EM6(ON);
            EM1(ON);
            EM31(ON);
            NEXT_OPER(o_inj_pop_end);
            break;

        case o_inj_pop_end:
            if (!KH4) NEXT_OPER(o_operation_end);
            if (!BK21) break;
            NEXT_OPER(o_operation_end);
            break;

        case o_disjunction_start:
            set_hydro(workset.hyd_U05);
            EM16(ON);
            EM4(ON);
            EM1(ON);
            EM31(ON);
            set_timer(TMR_14, workset.tmr_T14);
            EM40(ON);
            NEXT_OPER(o_disjunction_process);
            break;

        case o_disjunction_process:
            if (!KH2) NEXT_OPER(o_operation_end);
            i = get_timer(TMR_14);
            if (i == 0) EM40(OFF);
            if (!BK1) break;
            EM16(OFF);
            EM4(OFF);
            EM1(OFF);
            EM31(OFF);
            if (i >= 0) break;
            set_timer(TMR_16, workset.tmr_T16);
            EM41(ON);
            NEXT_OPER(o_disjunction_end);
            break;

        case o_disjunction_end:
            if (!KH2) NEXT_OPER(o_operation_end);
            if (get_timer(TMR_16) > 0) break;
            NEXT_OPER(o_operation_end);
            break;

        case o_emergency_stop:
            set_hydro(0);
            all_em_off();
            RDY(OFF);
            SET_ERROR(e_emergency_stop);
            NEXT_OPER(o_wait_buttons_release);
            break;
            
        case o_guard_stop:
            set_hydro(0);
            all_em_off();
            ENGINE(OFF);
            if (!BK56) SET_ERROR(e_guard_56);
            if (!BK57) SET_ERROR(e_guard_57);
            NEXT_OPER(o_wait_buttons_release);
            break;

        case o_operation_end:
            set_hydro(0);
            all_em_off();
            NEXT_OPER(o_wait_buttons_release);
            break;

        case o_wait_buttons_release:
            if (KH10) break;
            if (KH1) break;
            if (KH3) break;
            if (KH5) break;
            if (KH6) break;
            if (KH7) break;
            if (KH4) break;
            if (KH2) break;
            RDY(ON);
            NEXT_OPER(o_idle);
            break;
    }

}
