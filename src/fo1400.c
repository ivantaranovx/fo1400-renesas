
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

#define ON  1
#define OFF 0

/* inputs */

#define KH0     (!dio_in(52))
#define KH1     (!dio_in(35))
#define KH2     (!dio_in(36))
#define KH3     (!dio_in(37))
#define KH4     (!dio_in(38))
#define KH5     (!dio_in(39))
#define KH6     (!dio_in(48))
#define KH7     (!dio_in(49))
#define KH10    (!dio_in(62))
#define KH11    (!dio_in(61))
#define KH12    (!dio_in(60))
#define KH13    (!dio_in(59))
#define KH14    (!dio_in(58))

#define OT      (!dio_in(57))
#define RD      (!dio_in(56))
#define CE      dio_in(5)
#define KM1     (!dio_in(53))

#define M_ADJUST    (!dio_in(32))
#define M_MANUAL    (!dio_in(33))
#define M_SEMI_AUTO (!dio_in(34))
#define M_AUTO      (!dio_in(63))

#define BK1     dio_in(0)
#define BK2     (!dio_in(1))
#define BK3     dio_in(2)
#define BK4     dio_in(3)

#define BK20    dio_in(18)
#define BK21    dio_in(19)
#define BK22    dio_in(20)
#define BK23    dio_in(22)
#define BK24    dio_in(21)
#define BK25    dio_in(23)

#define BK50    dio_in(16)
#define BK51    dio_in(17)
#define BK52    dio_in(7)
#define BK53    dio_in(6)

#define BK56    dio_in(54)
#define BK57    dio_in(55)
#define BK58    (!dio_in(51))
#define BK59    dio_in(50)
#define BK60    dio_in(25)
#define BK61    dio_in(24)

/* outputs */

#define EM18(v)     dio_out(23, v)
#define EM29(v)     dio_out(22, v)
#define EM16(v)     dio_out(21, v)
#define EM1(v)      dio_out(20, v)
#define EM31(v)     dio_out(19, v)
#define EM13(v)     dio_out(18, v)
#define EM30(v)     dio_out(17, v)
#define EM4(v)      dio_out(16, v)

#define EM3(v)      dio_out(0, v)
#define EM6(v)      dio_out(1, v)
#define EM5(v)      dio_out(2, v)
#define EM12(v)     dio_out(3, v)
#define EM7(v)      dio_out(4, v)
#define EM2(v)      dio_out(5, v)
#define EM40(v)     dio_out(6, v)
#define EM41(v)     dio_out(7, v)

#define RDY(v)      dio_out(15, v)
#define ENGINE(v)   dio_out(14, v)
#define FAIL(v)     dio_out(13, v)

#define KM5(v)      dio_out(47, v)
#define KM10(v)     dio_out(46, v)
#define KM7(v)      dio_out(45, v)

static WORKSET workset;
static MAIN_STATE state;

#define f_heat_ok       state.flags.f.heat_ok
#define f_cycle_stop    state.flags.f.cycle_stop
#define f_power_on      state.flags.f.power_on
#define f_guard_ok      state.flags.f.guard_ok
#define f_engine_on     state.flags.f.engine_on

static int guard_state = 0;
static int engine_state = 0;
static int lub_state = 0;
static int lub_push_count = 0;

//static uint16_t tmr[19];

int check_heat(void);


int main(void)
{
    brd_init();
    uart_send_str("Harware init complete\r\n");

    dio_init();
    uart_send_str("Peripheral bus init complete\r\n");

    set_timer(0, 100);
    while (get_timer(0) > 0);

    uart_send_str("LCD init...");
    while (!lcd_init());
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

    state.mode = unknown;
    state.oper = idle;
    state.error = e_success;

    set_timer(TMR_LUB, workset.lub_reload);
    RDY(ON);

    lcd_clear();
    uart_send_str("Running\r\n");

    for(;;)
    {
        main_task();
        thermo_task();
        f_heat_ok = thermo_heat_ok();
        dio_task();
        ui_task();
    }
}

void main_task(void)
{
    int i, j;

    j = 0;
    if (M_ADJUST) { i = adjustment; j++; }
    if (M_MANUAL) { i = manual; j++; }
    if (M_SEMI_AUTO) { i = semi_auto; j++; }
    if (M_AUTO) { i = full_auto; j++; }
    if ((j == 1) && (i != state.mode))
    {
        state.mode = i;
        guard_state = 0;
        f_cycle_stop = 1;
    }

    if (KH14 ^ thermo_heat_enabled()) 
    {
        thermo_heat_enable(KH14);
    }

    if (!BK56 && !BK57 && (guard_state == 0)) guard_state = 1;
    if (!BK56 && BK57 && (guard_state == 1)) guard_state = 2;
    if (BK56 && BK57 && (guard_state == 2)) guard_state = 3;
    if ((!BK56 || !BK57) && (guard_state == 3))
    {
        guard_state = 0;
        all_em_off();
        set_hydro(0);
        ENGINE(OFF);
        if (!BK56) state.error = e_guard_56;
        if (!BK57) state.error = e_guard_57;
        state.oper = idle;
    }

    f_guard_ok = (guard_state == 3);
    f_power_on = KM1;

    switch(state.oper)
    {
    case idle:

        if (f_cycle_stop) f_cycle_stop = 0;

        if (state.mode == adjustment)
        {
            if (KH1 && !BK4)
            {
                state.oper = junction_start;
                state.error = e_success;
                break;
            }
            if (KH3 && !BK22)
            {
                state.oper = inj_push_start;
                state.error = e_success;
                break;
            }
            if (KH5 && !BK23)
            {
                state.oper = inject_start;
                state.error = e_success;
                break;
            }
            if (KH6 && !BK25)
            {
                state.oper = load_start;
                state.error = e_success;
                break;
            }
            if (KH7 && !BK20)
            {
                state.oper = decompression_start;
                state.error = e_success;
                break;
            }
            if (KH4 && !BK21)
            {
                state.oper = inj_pop_start;
                state.error = e_success;
                break;
            }
            if (KH2 && !BK1)
            {
                state.oper = disjunction_start;
                state.error = e_success;
                break;
            }
        }
        break;

    case junction_start:
        set_hydro(workset.hyd_U01);
        EM16(ON);
        EM3(ON);
        EM1(ON);
        EM31(ON);
        EM29(ON);
        state.oper = junction_full;
        break;

    case junction_full:
        if (KH10) state.oper = emergency_stop;
        if (!KH1) state.oper = operation_end;
        if (!BK4) break;
        set_hydro(workset.hyd_U01);
        EM29(OFF);
        state.oper = junction_end;
        break;

    case junction_end:
        if (KH10) state.oper = emergency_stop;
        if (!KH1) state.oper = operation_end;
        if (!BK2) break;
        state.oper = operation_end;
        break;

    case inj_push_start:
        set_hydro(workset.hyd_U06);
        EM16(ON);
        EM5(ON);
        EM1(ON);
        EM31(ON);
        state.oper = inj_push_end;
        break;

    case inj_push_end:
        if (KH10) state.oper = emergency_stop;
        if (!KH3) state.oper = operation_end;
        if (!BK22) break;
        state.oper = operation_end;
        break;

    case inject_start:
        if (check_heat()) break;
        set_hydro(workset.hyd_U07);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        EM31(ON);
        clr_scale_timer(TMR_SCALE_INJECT);
        uart_send_str("Injection start\r\n");
        state.oper = inject_process;
        break;

    case inject_process:
        uart_send_str("Injection x:");
        uart_uint(get_scale_timer(TMR_SCALE_INJECT));
        uart_send_str(", y:");
        uart_uint(rand());
        uart_send_str("\r\n");
        if (KH10) state.oper = emergency_stop;
        if (!KH5) state.oper = operation_end;
        if (BK23) state.oper = operation_end;
        if (state.oper != inject_process) uart_send_str("Injection stop\r\n");
        break;

    case load_start:
        if (check_heat()) break;
        set_hydro(workset.hyd_U10);
        EM16(ON);
        EM2(ON);
        EM13(ON);
        EM1(ON);
        EM31(ON);
        state.oper = load_end;
        break;

    case load_end:
        if (KH10) state.oper = emergency_stop;
        if (!KH6) state.oper = operation_end;
        if (!BK25) break;
        state.oper = operation_end;
        break;

    case decompression_start:
        if (check_heat()) break;
        set_hydro(workset.hyd_U11);
        EM16(ON);
        EM29(ON);
        EM12(ON);
        EM1(ON);
        EM31(ON);
        state.oper = decompression_end;
        break;

    case decompression_end:
        if (KH10) state.oper = emergency_stop;
        if (!KH7) state.oper = operation_end;
        if (!BK20) break;
        state.oper = operation_end;
        break;

    case inj_pop_start:
        set_hydro(workset.hyd_U13);
        EM16(ON);
        EM6(ON);
        EM1(ON);
        EM31(ON);
        state.oper = inj_pop_end;
        break;

    case inj_pop_end:
        if (KH10) state.oper = emergency_stop;
        if (!KH4) state.oper = operation_end;
        if (!BK21) break;
        state.oper = operation_end;
        break;

    case disjunction_start:
        set_hydro(workset.hyd_U05);
        EM16(ON);
        EM4(ON);
        EM1(ON);
        EM31(ON);
        set_timer(TMR_14, workset.tmr_T14);
        EM40(ON);
        state.oper = disjunction_process;
        break;

    case disjunction_process:
        if (KH10) state.oper = emergency_stop;
        if (!KH2) state.oper = operation_end;
        i = get_timer(TMR_14);
        if (i == 0) EM40(OFF);
        if (!BK1) break;
        EM16(OFF);
        EM4(OFF);
        EM1(OFF);
        EM31(OFF);
        if (i < 0)
        {
            set_timer(TMR_16, workset.tmr_T16);
            EM41(ON);
            state.oper = disjunction_end;
        }
        break;

    case disjunction_end:
        if (KH10) state.oper = emergency_stop;
        if (!KH2) state.oper = operation_end;
        if (get_timer(TMR_16) > 0) break;
        state.oper = operation_end;
        break;

    case emergency_stop:
        set_hydro(0);
        all_em_off();
        RDY(OFF);
        state.oper = wait_buttons_release;
        state.error = e_emergency;
        break;

    case operation_end:
        set_hydro(0);
        all_em_off();
        state.oper = wait_buttons_release;
        break;

    case wait_buttons_release:
        if (KH10) break;
        if (KH1) break;
        if (KH3) break;
        if (KH5) break;
        if (KH6) break;
        if (KH7) break;
        if (KH4) break;
        if (KH2) break;
        state.oper = idle;
        break;
    }


    switch (engine_state)
    {
    case 0:
        if (KH10) break;
        if (!KM1) break;
        if (KH12) engine_state = 10;
        if (KH13)
        {
            //error = e_engine_off;
            engine_state = 20;
            break;
        }
        if (OT)
        {
            f_cycle_stop = 1;
            ENGINE(OFF);
            set_hydro(0);
            //error = e_engine_overheat;
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
        if (RD)
        {
            ENGINE(ON);
            f_engine_on = 1;
            guard_state = 0;
            engine_state = 0;
            break;
        }
        if (get_timer(TMR_ENGINE)) break;
        //error = e_engine_not_ready;
        engine_state = 20;
        break;

    case 20:
        KM5(OFF);
        KM10(OFF);
        engine_state = 21;
        break;
    case 21:
        if (RD) break;
        f_engine_on = 0;
        engine_state = 0;
        break;
    }
    

    switch(lub_state)
    {
    case 0:
        if (KH10) break;
        if (!KM1) break;
        if (!f_engine_on) break;
        if (KH11)
        {
            lub_push_count = 1;
            lub_state = 2;
        }
        if (get_timer(TMR_LUB) < 0)
        {
            lub_push_count = workset.lub_count;
            lub_state = 2;
        }
        break;
    case 2:
        if (BK58)
        {
            lub_state = 8;
            break;
        }
        set_timer(TMR_LUB, workset.lub_time);
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
        set_timer(TMR_LUB, workset.lub_period);
        if (KH11) break;
        lub_state = 0;
        break;
    }

}

int check_heat(void)
{
    if (f_heat_ok) return 0;
    state.oper = idle;
    state.error = e_not_warmed;
    return 1;
}

MAIN_STATE *main_get_state(void)
{
    return &state;
}

void all_em_off(void)
{
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

void set_hydro(uint16_t speed)
{
    double dac = speed;
    dac /= 9.8039;
    if (dac > 255) dac = 255;
    dac = round(dac);
    _set_dac(0, (uint8_t)dac);
}

