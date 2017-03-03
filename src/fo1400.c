
#include "fo1400.h"

extern WORKSET workset; // GLOBAL!

static MAIN_STATE state;

#define f_ready         state.flags.f.ready
#define f_cycle_stop    state.flags.f.cycle_stop
#define f_heat_ok       state.flags.f.heat_ok
#define f_power_on      state.flags.f.power_on
#define f_guard_ok      state.flags.f.guard_ok
#define f_engine_on     state.flags.f.engine_on

static int pwr_count = 0;

//static uint16_t tmr[19];

void main_task(void)
{
    switch (state.oper)
    {

    case o_idle:

        f_cycle_stop = false;
        state.status = s_idle;

        if (state.mode == m_adjust)
        {
            if (KH1)
            {
                state.oper = o_junction;
                break;
            }
        }
        break;

    case o_junction:

        if (!KH1 || !op_junction())
        {
            error(e_success);
            state.oper = o_idle;
            break;
        }
        break;

        /*
                if (state.mode == m_adjust)
                {
                    if (KH1 && !BK4)
                    {
                        SET_ERROR(e_success);
                        NEXT_OPER(o_junction_start);
                        break;
                    }
                    if (KH3 && !BK22)
                    {
                        SET_ERROR(e_success);
                        NEXT_OPER(o_inj_push_start);
                        break;
                    }
                    if (KH5 && !BK23)
                    {
                        SET_ERROR(e_success);
                        NEXT_OPER(o_inject_start);
                        break;
                    }
                    if (KH6 && !BK25)
                    {
                        SET_ERROR(e_success);
                        NEXT_OPER(o_load_start);
                        break;
                    }
                    if (KH7 && !BK20)
                    {
                        SET_ERROR(e_success);
                        NEXT_OPER(o_decompression_start);
                        break;
                    }
                    if (KH4 && !BK21)
                    {
                        SET_ERROR(e_success);
                        NEXT_OPER(o_inj_pop_start);
                        break;
                    }
                    if (KH2 && !BK1)
                    {
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
                uart_print("Injection start\r\n");
                NEXT_OPER(o_inject_process);
                break;

            case o_inject_process:
                uart_print("Injection x:");
                uart_uint(get_scale_timer(TMR_SCALE_INJECT));
                uart_print(", y:");
                uart_uint(rand());
                uart_print("\r\n");
                if (!KH5) NEXT_OPER(o_operation_end);
                if (BK23) NEXT_OPER(o_operation_end);
                if (state.oper != o_inject_process) uart_print("Injection stop\r\n");
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

            case o_operation_end:
                set_hydro(0);
                all_em_off();
                NEXT_OPER(o_wait_buttons_release);
                break;
         */

    }

}

bool op_junction(void)
{
    if (BK2) return false;
    set_hydro(workset.hyd_U01);
    if (!BK4)
    {
        state.status = s_junction_slow;
        EM29(ON);
    }
    else
    {
        state.status = s_junction_full;
        EM29(OFF);
    }
    EM16(ON);
    EM3(ON);
    EM1(ON);
    EM31(ON);
    return true;
}

void error(MAIN_ERROR e)
{
    engine_enable(false);

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

    state.error = e;
}

void set_hydro(uint16_t speed)
{
    double dac = speed;
    dac /= 9.8039;
    if (dac > 255) dac = 255;
    dac_set(0, (uint8_t) round(dac));
}

int check_kn(bool kn, STN stn)
{
    static uint16_t store = 0;
    if (kn ^ ((store & stn) ? true : false))
    {
        store ^= stn;
        if (kn) return 1;
        return -1;
    }
    return 0;
}

void check_mode_selector(void)
{
    int j = 0;
    int i = m_unknown;

    if (M_ADJUST)
    {
        i = m_adjust;
        j++;
    }
    if (M_MANUAL)
    {
        i = m_manual;
        j++;
    }
    if (M_SEMI_AUTO)
    {
        i = m_semi;
        j++;
    }
    if (M_AUTO)
    {
        i = m_auto;
        j++;
    }
    if (j > 1) return;
    if (i == state.mode) return;

    FAIL(OFF);
    engine_enable(true);

    f_guard_ok = false;
    f_cycle_stop = true;

    state.mode = i;
    state.error = e_success;
}

void check_guard(void)
{
    const uint8_t gs[] = {3, 2, 0, 2, 3, 3};
    static int gc = 0;
    uint8_t s = 0;

    if (BK56) s |= 1;
    if (BK57) s |= 2;

    if (gs[gc] == s)
    {
        if (gc == 5)
        {
            f_guard_ok = true;
            rst_timer(TMR_GUARD);
            state.error = e_success;
        }
        if (gc < 5)
        {
            gc++;
            set_timer(TMR_GUARD, 1000);
        }
    }
    else
    {
        if (gc == 5)
        {
            gc = 0;
            f_guard_ok = false;
            FAIL(ON);
            error(e_guard_chk);
        }
    }

    if (get_timer(TMR_GUARD) == 0)
    {
        if (gc == 3) error(e_guard_57);
        if (gc == 4) error(e_guard_56);
        gc = 0;
    }
}

void engine_enable(bool e)
{
    if (e == false)
    {
        ENGINE(OFF);
        set_hydro(0);
        return;
    }
    if (!f_ready || !f_power_on || f_engine_on) return;
    ENGINE(ON);
}

void engine_task(void)
{
    static int engine_state = 0;

    switch (engine_state)
    {
    case 0:
        if (check_kn(KH12, S_KH12) > 0)
        {
            if (!f_ready || !f_power_on || f_engine_on) break;
            state.error = e_success;
            engine_state = 10;
            break;
        }
        if (!f_engine_on) break;
        if (check_kn(KH13, S_KH13) > 0)
        {
            if (state.oper != o_idle) break;
            engine_state = 20;
            break;
        }
        if (!f_power_on)
        {
            error(e_not_powered);
            state.oper = o_idle;
            engine_state = 20;
            break;
        }
        if (!RD)
        {
            error(e_engine_not_ready);
            state.oper = o_idle;
            engine_state = 20;
            break;
        }
        if (check_kn(OT, S_OT) > 0)
        {
            FAIL(ON);
            error(e_engine_overheat);
            f_cycle_stop = true;
            engine_state = 1;
            break;
        }
        break;
    case 1:
        if (state.oper != o_idle) break;
        engine_state = 20;
        break;
    case 10:
        set_hydro(0);
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
            engine_enable(true);
            f_engine_on = true;
            f_guard_ok = false;
            state.error = e_success;
            engine_state = 0;
            break;
        }
        if (get_timer(TMR_ENGINE)) break;
        error(e_engine_not_ready);
        engine_state = 20;
        break;
    case 20:
        KM5(OFF);
        KM10(OFF);
        engine_state = 21;
        break;
    case 21:
        if (RD) break;
        f_engine_on = false;
        engine_state = 0;
        break;
    }
}

void lub_task(void)
{
    static int lub_period;
    static int lub_push_count;
    static int lub_state = 0;

    if (check_kn(f_engine_on, S_ENG) > 0) lub_period = workset.lub_reload * 60;

    switch (lub_state)
    {
    case 0:
        if (!f_ready || !f_power_on || !f_engine_on) break;
        if (KH11)
        {
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
        if (BK58)
        {
            FAIL(ON);
            lub_state = 8;
            break;
        }
        set_timer(TMR_LUB, workset.lub_time * 1000);
        KM7(ON);
        lub_state = 4;
        break;
    case 4:
        if ((get_timer(TMR_LUB) > 0) && f_engine_on) break;
        KM7(OFF);
        set_timer(TMR_LUB, 1000);
        lub_state = 6;
        break;
    case 6:
        if (get_timer(TMR_LUB) > 0) break;
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

MAIN_STATE *main_get_state(void)
{
    return &state;
}

int main(void)
{
    int r;

    brd_init();

    uart_print("\fHarware init complete\r\n");
    uart_printf("Board: %s\r\n", brd_id());
    uart_printf("Machine: %s\r\n", msg_machine);
    uart_printf("Version: %s\r\n", msg_version);

    while (1)
    {
        uart_print("LCD init: ");
        if (lcd_init())
        {
            uart_print("ok\r\n");
            break;
        }
        uart_print("error\r\n");
        set_timer(TMR_SYS, 100);
        while (get_timer(TMR_SYS) > 0);
    }

    lcd_clear();

    for (int i = 0; i < 3; i++)
    {
        r = get_workset_addr(0);
        eeprom_cs(0, r);
        eeprom_read((uint8_t*) & workset, sizeof (workset));
        if (eeprom_status_wait()) break;
    }

    f_ready = false;
    f_power_on = false;
    f_guard_ok = false;
    f_engine_on = false;
    f_heat_ok = false;
    f_cycle_stop = false;

    check_kn(true, S_KH10);
    check_kn(false, S_KH14);
    check_kn(false, S_ENG);

    state.mode = m_unknown;
    state.oper = o_idle;
    state.error = e_success;

    dio_init();

    uart_print("Run\r\n");

    for (;;)
    {
        r = check_kn(KH14, S_KH14);
        if (r > 0) thermo_heat_enable(true);
        if (r < 0) thermo_heat_enable(false);

        r = check_kn(KH10, S_KH10);
        if (r > 0)
        {
            f_ready = false;
            RDY(OFF);
            if (state.oper != o_idle)
            {
                error(e_emergency_stop);
                state.oper = o_idle;
            }
        }
        if (r < 0)
        {
            f_ready = true;
            RDY(ON);
        }

        if (check_kn(CE, S_CE) > 0) pwr_count++;

        f_heat_ok = thermo_heat_ok();
        f_power_on = KM1;

        check_mode_selector();
        check_guard();
        engine_task();
        lub_task();
        thermo_task();
        main_task();
        dio_task();
        ui_task();
        tcpip_task();
    }
}
