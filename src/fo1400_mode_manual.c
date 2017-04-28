
#include <stdbool.h>

#include "hal.h"
#include "workset.h"
#include "fo1400_io.h"
#include "fo1400_common.h"
#include "fo1400_mode_manual.h"
#include "pressure.h"

extern WORKSET workset; // GLOBAL!

void op_mode_manual(MAIN_STATE *state)
{
    if (state->oper != (MAIN_OPER) s_idle)
    {
        if (!check_guard(state))
        {
            stop();
            state->oper = o_idle;
        }
        if (!engine_ready(state))
        {
            state->error = e_engine_off;
            state->oper = o_idle;
        }
    }

    switch (state->oper)
    {
    case o_idle:

        state->status = s_idle;

        if (check_kn(KH1, S_KH1) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK2) break;
            if (!BK1) BREAK_ERR(e_err_bk1);
            state->error = e_success;
            state->oper = o_junction;
            break;
        }

        if (check_kn(KH3, S_KH3) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK22) break;
            if (!BK21) BREAK_ERR(e_err_bk21);
            state->error = e_success;
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
            state->error = e_success;
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
            state->error = e_success;
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
            state->error = e_success;
            state->oper = o_decompression;
            break;
        }

        if (check_kn(KH4, S_KH4) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK21) break;
            if (!BK22) BREAK_ERR(e_err_bk22);
            state->error = e_success;
            state->oper = o_inj_pop;
            break;
        }

        if (check_kn(KH2, S_KH2) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            if (BK3) break;
            if (!BK2) BREAK_ERR(e_err_bk2);
            state->error = e_success;
            state->oper = o_disjunction;
            break;
        }
        break;

    case o_junction:
        state->status = s_junction_break;
        set_hydro(workset.hyd_U01);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
        if (!BK52) break;
        stop();
        state->oper = o_junction_fast;
        break;

    case o_junction_fast:
        state->status = s_junction_fast;
        set_hydro(workset.hyd_U02);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
        if (!BK50) break;
        stop();
        state->oper = o_junction_prev;
        break;

    case o_junction_prev:
        state->status = s_junction_prev;
        set_hydro(workset.hyd_U03);
        EM3(ON);
        EM18(ON);
        EM1(ON);
        if (!BK4) break;
        stop();
        state->oper = o_junction_lock;
        break;

    case o_junction_lock:
        state->status = s_junction_lock;
        set_hydro(workset.hyd_U04);
        EM3(ON);
        EM16(ON);
        EM1(ON);
        if (!BK2) break;
        stop();
        state->oper = o_idle;
        break;

    case o_inj_push:
        state->status = s_inj_push;
        set_hydro(workset.hyd_U06);
        EM5(ON);
        EM16(ON);
        EM1(ON);
        if (!BK22) break;
        stop();
        state->oper = o_idle;
        break;

    case o_inject:
        clr_scale_timer(TMR_SCALE_INJECT);
        uart_print("inj start\r\n");
        state->oper = o_inject_1;
        break;

    case o_inject_1:
        state->status = s_inject_1;
        set_hydro(workset.hyd_U07);
        EM7(ON);
        EM16(ON);
        EM1(ON);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_INJECT), get_pressure_mpa());
        if (!BK24) break;
        stop();
        state->oper = o_inject_2;
        break;

    case o_inject_2:
        state->status = s_inject_2;
        set_hydro(workset.hyd_U08);
        EM7(ON);
        EM16(ON);
        EM1(ON);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_INJECT), get_pressure_mpa());
        if (!BK23) break;
        uart_print("inj stop\r\n");
        stop();
        state->oper = o_idle;
        break;

    case o_load:
        state->status = s_load;
        set_hydro(workset.hyd_U10);
        EM2(ON);
        EM16(ON);
        EM13(ON);
        EM1(ON);
        if (!BK25) break;
        stop();
        state->oper = o_idle;
        break;

    case o_decompression:
        state->status = s_decompression;
        set_hydro(workset.hyd_U11);
        EM29(ON);
        EM16(ON);
        EM12(ON);
        EM1(ON);
        if (!BK20) break;
        stop();
        state->oper = o_idle;
        break;

    case o_inj_pop:
        state->status = s_inj_pop;
        set_hydro(workset.hyd_U13);
        EM6(ON);
        EM16(ON);
        EM1(ON);
        if (!BK21) break;
        stop();
        state->oper = o_idle;
        break;

    case o_disjunction:
        EM4(ON);
        EM16(ON);
        EM1(ON);
        EM40(ON);
        set_timer(TMR_14, workset.tmr_T14 * 10);
        state->oper = o_disjunction_break;
        break;

    case o_disjunction_break:
        state->status = s_disjunction_break;
        set_hydro(workset.hyd_U14);
        if (!BK3) break;
        state->oper = o_disjunction_fast;
        break;

    case o_disjunction_fast:
        state->status = s_disjunction_fast;
        set_hydro(workset.hyd_U12);
        if (!BK51) break;
        state->oper = o_disjunction_slow;
        break;

    case o_disjunction_slow:
        state->status = s_disjunction_slow;
        set_hydro(workset.hyd_U05);
        if (!BK1) break;
        set_hydro(0);
        EM4(OFF);
        EM16(OFF);
        EM1(OFF);
        EM41(ON);
        set_timer(TMR_16, workset.tmr_T16 * 10);
        state->oper = o_idle;
        break;

    default:
        break;
    }

    if (get_timer(TMR_14) == 0) EM40(OFF);
    if (get_timer(TMR_16) == 0) EM41(OFF);
}

