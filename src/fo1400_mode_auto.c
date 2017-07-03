
#include <stdbool.h>

#include "hal.h"
#include "workset.h"
#include "fo1400_io.h"
#include "fo1400_common.h"
#include "fo1400_mode_auto.h"
#include "pressure.h"

extern WORKSET workset; // GLOBAL!

static bool f_inj_pop = true;

void op_mode_auto(MAIN_STATE *state)
{
    int r;

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
        
        r = check_kn(KH1, S_KH1);

        if (state->mode == m_semi)
        {
            state->status = s_idle;
            if (r <= 0) break;
        }

        if (state->mode == m_auto)
        {
            if (r > 0) 
            {
                state->error = e_success;
                state->flags.f.cycle_stop = false;
            }
            if (state->error != e_success) break;
            if (state->flags.f.cycle_stop) break;
        }

        if (!check_engine(state)) break;
        if (!check_heat(state)) break;
        if (!check_guard(state)) break;

        if (!BK53)
        {
            state->error = e_err_bk53;
            break;
        }

        state->oper = o_junction_break;
        state->error = e_success;
        set_timer(TMR_1, workset.tmr_T01 * 10);
        break;

    case o_junction_break:
        state->status = s_junction_break;
        set_hydro(workset.hyd_U01);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
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
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK50) break;
        stop();
        state->oper = o_junction_prev;
        set_timer(TMR_2, workset.tmr_T02 * 10);
        break;

    case o_junction_prev:
        state->status = s_junction_prev;
        set_hydro(workset.hyd_U03);
        EM3(ON);
        EM18(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (get_timer(TMR_2) < 0) BREAK_ERR(e_err_tmr2);
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
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK2) break;
        stop();
        state->oper = o_inj_push;
        break;

    case o_inj_push:
        if (f_inj_pop)
        {
            state->status = s_inj_push;
            set_hydro(workset.hyd_U06);
            EM5(ON);
            EM16(ON);
            EM1(ON);
            if (!BK2) BREAK_ERR(e_err_bk2);
            if (!BK20) BREAK_ERR(e_err_bk20);
            if (!BK22) break;
            stop();
        }
        state->oper = o_inject_1;
        set_timer(TMR_3, workset.tmr_T03 * 10);
        clr_scale_timer(TMR_SCALE_INJECT);
        uart_print("inj start\r\n");
        break;

    case o_inject_1:
        state->status = s_inject_1;
        set_hydro(workset.hyd_U07);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_INJECT), get_pressure_mpa());
        if (!BK24) break;
        stop();
        state->oper = o_inject_2;
        break;

    case o_inject_2:
        state->status = s_inject_2;
        set_hydro(workset.hyd_U08);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        if (get_timer(TMR_3) < 0) BREAK_ERR(e_err_tmr3);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_INJECT), get_pressure_mpa());
        if (!BK23) break;
        uart_print("inj stop\r\n");
        stop();
        state->oper = o_form_hi;
        set_timer(TMR_10, workset.tmr_T10 * 10);
        break;

    case o_form_hi:
        state->status = s_form;
        set_hydro(workset.hyd_U15);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        if (get_timer(TMR_10) > 0) break;
        stop();
        state->oper = o_form;
        set_timer(TMR_4, workset.tmr_T04 * 10);
        break;

    case o_form:
        state->status = s_form;
        set_hydro(workset.hyd_U09);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        EM30(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        if (get_timer(TMR_4) > 0) break;
        stop();
        state->oper = o_load;
        set_timer(TMR_5, workset.tmr_T05 * 10);
        set_timer(TMR_6, workset.tmr_T06 * 10);
        break;

    case o_load:
        state->status = s_load;
        set_hydro(workset.hyd_U10);
        EM2(ON);
        EM16(ON);
        EM13(ON);
        EM1(ON);
        EM5(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        if (get_timer(TMR_6) < 0) BREAK_ERR(e_err_tmr6);
        if (!BK25) break;
        stop();
        state->oper = o_decompression;
        break;

    case o_decompression:
        state->status = s_decompression;
        set_hydro(workset.hyd_U11);
        EM29(ON);
        EM16(ON);
        EM12(ON);
        EM1(ON);
        EM5(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        if (!BK20) break;
        stop();
        state->oper = o_inj_pop;
        break;

    case o_inj_pop:
        f_inj_pop = workset.inj_pop;
        if (f_inj_pop)
        {
            state->status = s_inj_pop;
            set_hydro(workset.hyd_U13);
            EM6(ON);
            EM16(ON);
            EM1(ON);
            if (!BK2) BREAK_ERR(e_err_bk2);
            if (!BK20) BREAK_ERR(e_err_bk20);
            if (!BK21) break;
            stop();
        }
        state->oper = o_cooling;
        break;

    case o_cooling:
        state->status = s_cooling;
        if (get_timer(TMR_5) > 0) break;
        set_timer(TMR_13, workset.tmr_T13 * 10);
        state->oper = o_disjunction_break;
        break;

    case o_disjunction_break:
        state->status = s_disjunction_break;
        set_hydro(workset.hyd_U14);
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK21) BREAK_ERR(e_err_bk21);
        if (!BK3) break;
        set_timer(TMR_15, workset.tmr_T15 * 10);
        stop();
        state->oper = o_disjunction_fast;
        break;

    case o_disjunction_fast:
        state->status = s_disjunction_fast;
        set_hydro(workset.hyd_U12);
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK21) BREAK_ERR(e_err_bk21);
        if (!BK51) break;
        stop();
        state->oper = o_disjunction_slow;
        set_timer(TMR_8, workset.tmr_T08 * 10);
        break;

    case o_disjunction_slow:
        state->status = s_disjunction_slow;
        set_hydro(workset.hyd_U05);
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK21) BREAK_ERR(e_err_bk21);
        if (get_timer(TMR_8) < 0) BREAK_ERR(e_err_tmr8);
        if (!BK1) break;
        stop();
        state->oper = o_jump;
        break;

    case o_jump:
        state->status = s_jump;
        set_hydro(workset.hyd_U16);
        EM3(ON);
        EM16(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK21) BREAK_ERR(e_err_bk21);
        if (!BK53) break;
        stop();
        state->flags.f.cycle_report = true;
        state->oper = o_pause;
        set_timer(TMR_9, workset.tmr_T09 * 10);
        break;

    case o_pause:
        state->status = s_done;
        kill_timer(TMR_1);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK21) BREAK_ERR(e_err_bk21);
        if (get_timer(TMR_9) > 0) break;
        state->oper = o_idle;
        break;

    default:
        break;

    }

    if (get_timer(TMR_1) == 0) STOP_ERR(e_err_tmr1);

    if (get_timer(TMR_13) == 0)
    {
        EM40(ON);
        set_timer(TMR_14, workset.tmr_T14 * 10);
    }
    if (get_timer(TMR_15) == 0)
    {
        EM41(ON);
        set_timer(TMR_16, workset.tmr_T16 * 10);
    }
    if (get_timer(TMR_14) == 0) EM40(OFF);
    if (get_timer(TMR_16) == 0) EM41(OFF);
}

