
#include <stdbool.h>

#include "hal.h"
#include "workset.h"
#include "fo1400_io.h"
#include "fo1400_common.h"
#include "fo1400_mode_manual.h"
#include "pressure.h"

extern WORKSET workset; // GLOBAL!

static bool once = false;
#define RUN_ONCE for(; !once; once = true)

void op_mode_manual(MAIN_STATE *state)
{
    if (state == 0)
    {

        return;
    }

    if (state->oper != (MAIN_OPER) s_idle)
    {
        if (!check_guard(state))
        {
            stop();
            state->oper = o_idle;
        }
        if (!engine_ready(state))
        {
            state->err[0] = e_engine_off;
            state->oper = o_idle;
        }
    }

    switch (state->oper)
    {
    case o_idle:

        state->stat[0] = s_idle;

        if (check_kn(KH1, S_KH1) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK2) break;
            if (!BK1) BREAK_ERR(e_err_bk1);
            state->oper = o_junction;
            break;
        }

        if (check_kn(KH3, S_KH3) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK22) break;
            if (!BK21) BREAK_ERR(e_err_bk21);
            state->oper = o_inj_push;
            break;
        }

        if (check_kn(KH5, S_KH5) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            if (BK23) break;
            if (!BK20) BREAK_ERR(e_err_bk20);
            state->oper = o_inject;
            break;
        }

        if (check_kn(KH6, S_KH6) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            if (BK25) break;
            if (!BK23) BREAK_ERR(e_err_bk23);
            state->oper = o_load;
            break;
        }

        if (check_kn(KH7, S_KH7) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            if (BK20) break;
            if (!BK25) BREAK_ERR(e_err_bk25);
            state->oper = o_decompression;
            break;
        }

        if (check_kn(KH4, S_KH4) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK21) break;
            if (!BK22) BREAK_ERR(e_err_bk22);
            state->oper = o_inj_pop;
            break;
        }

        if (check_kn(KH2, S_KH2) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK3) break;
            if (!BK2) BREAK_ERR(e_err_bk2);
            state->oper = o_disjunction;
            break;
        }
        
        if (state->oper == o_idle) break;
        state->err[0] = e_success;
        state->stat[0] = s_none;
        clr_scale_timer(TMR_SCALE_D1);
        clr_scale_timer(TMR_SCALE_D2);
        run_scale_timer(TMR_SCALE_D1, true);
        once = false;
        break;

    case o_junction:
        state->stat[1] = s_junction;
        state->stat[2] = s_junction_break;
        RUN_ONCE
        {
            run_scale_timer(TMR_SCALE_D2, true);
        }
        set_hydro_u(workset.hyd_U01);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
        if (!BK52) break;
        stop();
        state->oper = o_junction_fast;
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_junction_fast:
        state->stat[2] = s_junction_fast;
        set_hydro_u(workset.hyd_U02);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
        if (!BK50) break;
        stop();
        state->oper = o_junction_prev;
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_junction_prev:
        state->stat[2] = s_junction_prev;
        set_hydro_u(workset.hyd_U03);
        EM3(ON);
        EM18(ON);
        EM1(ON);
        if (!BK4) break;
        stop();
        state->oper = o_junction_lock;
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_junction_lock:
        state->stat[2] = s_junction_lock;
        set_hydro_u(workset.hyd_U04);
        EM3(ON);
        EM16(ON);
        EM1(ON);
        if (!BK2) break;
        stop();
        state->oper = o_idle;
        run_scale_timer(TMR_SCALE_D1, false);
        run_scale_timer(TMR_SCALE_D2, false);
        break;

    case o_inj_push:
        state->stat[1] = s_inj_push;
        state->stat[2] = s_none;
        set_hydro_u(workset.hyd_U06);
        EM5(ON);
        EM16(ON);
        EM1(ON);
        if (!BK22) break;
        stop();
        state->oper = o_idle;
        run_scale_timer(TMR_SCALE_D1, false);
        break;

    case o_inject:
        state->stat[1] = s_inject;
        run_scale_timer(TMR_SCALE_D1, true);
        uart_print("inj start\r\n");
        state->oper = o_inject_1;
        run_scale_timer(TMR_SCALE_D2, true);
        break;

    case o_inject_1:
        state->stat[2] = s_inject_1;
        set_hydro_u(workset.hyd_U07);
        EM7(ON);
        EM16(ON);
        EM1(ON);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_D1), get_pressure_mpa(0));
        if (!BK24) break;
        stop();
        state->oper = o_inject_2;
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_inject_2:
        state->stat[2] = s_inject_2;
        set_hydro_u(workset.hyd_U08);
        EM7(ON);
        EM16(ON);
        EM1(ON);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_D1), get_pressure_mpa(0));
        if (!BK23) break;
        uart_print("inj stop\r\n");
        stop();
        state->oper = o_idle;
        run_scale_timer(TMR_SCALE_D1, false);
        run_scale_timer(TMR_SCALE_D2, false);
        break;

    case o_load:
        state->stat[1] = s_load;
        state->stat[2] = s_none;
        set_hydro_u(workset.hyd_U10);
        EM2(ON);
        EM16(ON);
        EM13(ON);
        EM1(ON);
        if (!BK25) break;
        stop();
        state->oper = o_idle;
        run_scale_timer(TMR_SCALE_D1, false);
        break;

    case o_decompression:
        state->stat[1] = s_decompression;
        state->stat[2] = s_none;
        set_hydro_u(workset.hyd_U11);
        EM29(ON);
        EM16(ON);
        EM12(ON);
        EM1(ON);
        if (!BK20) break;
        stop();
        state->oper = o_idle;
        run_scale_timer(TMR_SCALE_D1, false);
        break;

    case o_inj_pop:
        state->stat[1] = s_inj_pop;
        state->stat[2] = s_none;
        set_hydro_u(workset.hyd_U13);
        EM6(ON);
        EM16(ON);
        EM1(ON);
        if (!BK21) break;
        stop();
        state->oper = o_idle;
        run_scale_timer(TMR_SCALE_D1, false);
        break;

    case o_disjunction:
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (workset.p_s)
        {
            set_timer(TMR_13, (uint32_t)(workset.tmr_T13) * 10);
            set_timer(TMR_15, (uint32_t)(workset.tmr_T15) * 10);
            set_timer(TMR_17, (uint32_t)(workset.tmr_T17) * 10);
        }
        state->oper = o_disjunction_break;
        state->stat[1] = s_disjunction;
        run_scale_timer(TMR_SCALE_D2, true);
        break;

    case o_disjunction_break:
        state->stat[2] = s_disjunction_break;
        set_hydro_u(workset.hyd_U14);
        if (!BK3) break;
        state->oper = o_disjunction_fast;
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_disjunction_fast:
        state->stat[2] = s_disjunction_fast;
        set_hydro_u(workset.hyd_U12);
        if (!BK51) break;
        state->oper = o_disjunction_slow;
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_disjunction_slow:
        state->stat[2] = s_disjunction_slow;
        set_hydro_u(workset.hyd_U05);
        if (!BK1) break;
        set_hydro_u(0);
        EM4(OFF);
        EM16(OFF);
        EM1(OFF);
        state->oper = o_idle;
        run_scale_timer(TMR_SCALE_D1, false);
        run_scale_timer(TMR_SCALE_D2, false);
        break;

    default:
        break;
    }

    if (get_timer(TMR_13) == 0)
    {
        EM40(ON);
        set_timer(TMR_14, (uint32_t)(workset.tmr_T14) * 10);
    }
    if (get_timer(TMR_14) == 0) EM40(OFF);

    if (get_timer(TMR_15) == 0)
    {
        EM41(ON);
        set_timer(TMR_16, (uint32_t)(workset.tmr_T16) * 10);
    }
    if (get_timer(TMR_16) == 0) EM41(OFF);

    if (get_timer(TMR_17) == 0)
    {
        EM40(ON);
        set_timer(TMR_18, (uint32_t)(workset.tmr_T18) * 10);
    }
    if (get_timer(TMR_18) == 0) EM40(OFF);
}

