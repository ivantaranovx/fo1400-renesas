
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
        if (state->flags.f.cycle_run) state->stat[0] = s_cycle_t;
    }

    switch (state->oper)
    {
    case o_idle:

        state->stat[0] = s_idle;

        r = check_kn(KH1, S_KH1);

        if (state->mode == m_semi)
        {
            if (r <= 0) break;
        }

        if (state->mode == m_auto)
        {
            if (r > 0)
            {
                state->err[0] = e_success;
                state->flags.f.cycle_run = true;
            }
            if (state->err[0] != e_success) break;
            if (!state->flags.f.cycle_run) break;
        }

        if (!check_engine(state)) break;
        if (!check_heat(state)) break;
        if (!check_guard(state)) break;

        if (!BK53)
        {
            state->err[0] = e_err_bk53;
            break;
        }

        set_timer(TMR_1, (uint32_t)(workset.tmr_T01) * 10);
        state->err[0] = e_success;
        state->stat[0] = s_cycle;
        state->stat[1] = s_junction;
        state->oper = o_junction_break;
        clr_scale_timer(TMR_SCALE_D0);
        clr_scale_timer(TMR_SCALE_D1);
        clr_scale_timer(TMR_SCALE_D2);
        run_scale_timer(TMR_SCALE_D0, true);
        run_scale_timer(TMR_SCALE_D1, true);
        run_scale_timer(TMR_SCALE_D2, true);
        break;

    case o_junction_break:
        state->stat[2] = s_junction_break;
        set_hydro_u(workset.hyd_U01);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
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
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK50) break;
        stop();
        state->oper = o_junction_prev;
        set_timer(TMR_2, (uint32_t)(workset.tmr_T02) * 10);
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_junction_prev:
        state->stat[2] = s_junction_prev;
        set_hydro_u(workset.hyd_U03);
        EM3(ON);
        EM18(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (get_timer(TMR_2) < 0) BREAK_ERR(e_err_tmr2);
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
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (!BK2) break;
        stop();
        set_timer(TMR_HYD, 200);
        state->oper = o_inj_push;
        clr_scale_timer(TMR_SCALE_D1);
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_inj_push:
        if (f_inj_pop)
        {
            state->stat[1] = s_inj_push;
            state->stat[2] = s_none;
            set_hydro_u(workset.hyd_U06);
            EM5(ON);
            EM16(ON);
            EM1(ON);
            if (!BK2) BREAK_ERR(e_err_bk2);
            if (!BK20) BREAK_ERR(e_err_bk20);
            if (!BK22) break;
            stop();
            clr_scale_timer(TMR_SCALE_D1);
            clr_scale_timer(TMR_SCALE_D2);
        }
        state->oper = o_inject_1;
        state->stat[1] = s_inject;
        set_timer(TMR_3, (uint32_t)(workset.tmr_T03) * 10);
        uart_print("inj start\r\n");
        break;

    case o_inject_1:
        state->stat[2] = s_inject_1;
        set_hydro_u(workset.hyd_U07);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_D2), get_pressure_mpa(0));
        if (!BK24) break;
        stop();
        state->oper = o_inject_2;
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_inject_2:
        state->stat[2] = s_inject_2;
        set_hydro_u(workset.hyd_U08);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        if (get_timer(TMR_3) < 0) BREAK_ERR(e_err_tmr3);
        uart_printf("inj %u %f\r\n", get_scale_timer(TMR_SCALE_D2), get_pressure_mpa(0));
        if (!BK23) break;
        uart_print("inj stop\r\n");
        stop();
        state->oper = o_form_hi;
        set_timer(TMR_10, (uint32_t)(workset.tmr_T10) * 10);
        clr_scale_timer(TMR_SCALE_D1);
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_form_hi:
        state->stat[2] = s_form_h;
        set_hydro_u(workset.hyd_U15);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        if (!BK2) BREAK_ERR(e_err_bk2);
        if (!BK22) BREAK_ERR(e_err_bk22);
        if (get_timer(TMR_10) > 0) break;
        stop();
        state->oper = o_form;
        set_timer(TMR_4, (uint32_t)(workset.tmr_T04) * 10);
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_form:
        state->stat[2] = s_form_l;
        set_hydro_u(workset.hyd_U09);
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
        set_timer(TMR_5, (uint32_t)(workset.tmr_T05) * 10);
        set_timer(TMR_6, (uint32_t)(workset.tmr_T06) * 10);
        clr_scale_timer(TMR_SCALE_D1);
        clr_scale_timer(TMR_SCALE_D2);
        break;

    case o_load:
        state->stat[1] = s_load;
        state->stat[2] = s_cooling;
        set_hydro_u(workset.hyd_U10);
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
        clr_scale_timer(TMR_SCALE_D1);
        break;

    case o_decompression:
        state->stat[1] = s_decompression;
        set_hydro_u(workset.hyd_U11);
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
        clr_scale_timer(TMR_SCALE_D1);
        break;

    case o_inj_pop:
        f_inj_pop = workset.inj_pop;
        if (f_inj_pop)
        {
            state->stat[1] = s_inj_pop;
            set_hydro_u(workset.hyd_U13);
            EM6(ON);
            EM16(ON);
            EM1(ON);
            if (!BK2) BREAK_ERR(e_err_bk2);
            if (!BK20) BREAK_ERR(e_err_bk20);
            if (!BK21) break;
            stop();
        }
        state->oper = o_cooling;
        run_scale_timer(TMR_SCALE_D1, false);
        break;

    case o_cooling:
        state->stat[1] = s_none;
        if (get_timer(TMR_5) > 0) break;
        if (workset.p_s)
        {
            set_timer(TMR_13, (uint32_t)(workset.tmr_T13) * 10);
            set_timer(TMR_15, (uint32_t)(workset.tmr_T15) * 10);
            set_timer(TMR_17, (uint32_t)(workset.tmr_T17) * 10);
        }
        state->oper = o_disjunction_break;
        run_scale_timer(TMR_SCALE_D2, false);
        clr_scale_timer(TMR_SCALE_D1);
        run_scale_timer(TMR_SCALE_D1, true);
        break;

    case o_disjunction_break:
        state->stat[1] = s_disjunction_break;
        set_hydro_u(workset.hyd_U14);
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (f_inj_pop && !BK21) BREAK_ERR(e_err_bk21);
        if (!BK3) break;
        state->oper = o_disjunction_fast;
        clr_scale_timer(TMR_SCALE_D1);
        break;

    case o_disjunction_fast:
        state->stat[1] = s_disjunction_fast;
        set_hydro_u(workset.hyd_U12);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (f_inj_pop && !BK21) BREAK_ERR(e_err_bk21);
        if (!BK51) break;
        state->oper = o_disjunction_slow;
        set_timer(TMR_8, (uint32_t)(workset.tmr_T08) * 10);
        clr_scale_timer(TMR_SCALE_D1);
        break;

    case o_disjunction_slow:
        state->stat[1] = s_disjunction_slow;
        set_hydro_u(workset.hyd_U05);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (f_inj_pop && !BK21) BREAK_ERR(e_err_bk21);
        if (get_timer(TMR_8) < 0) BREAK_ERR(e_err_tmr8);
        if (!BK1) break;
        EM4(OFF);
        state->flags.f.cycle_report = true;
        state->oper = o_pause;
        if (workset.jump) state->oper = o_jump;
        clr_scale_timer(TMR_SCALE_D1);
        break;

    case o_jump:
        state->stat[1] = s_jump;
        set_hydro_u(workset.hyd_U16);
        EM3(ON);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (f_inj_pop && !BK21) BREAK_ERR(e_err_bk21);
        if (!BK53) break;
        set_hydro_u(0);
        state->oper = o_pause;
        break;

    case o_pause:
        if (state->stat[1] != s_pause)
        {
            state->stat[1] = s_pause;
            run_scale_timer(TMR_SCALE_D0, false);
            run_scale_timer(TMR_SCALE_D1, false);
            kill_timer(TMR_1);
            set_timer(TMR_9, (uint32_t)(workset.tmr_T09) * 10);
        }
        EM3(OFF);
        EM16(OFF);
        EM1(OFF);
        if (!BK20) BREAK_ERR(e_err_bk20);
        if (f_inj_pop && !BK21) BREAK_ERR(e_err_bk21);
        if (get_timer(TMR_9) > 0) break;
        if (get_timer(TMR_13) > 0) break;
        if (get_timer(TMR_14) > 0) break;
        if (get_timer(TMR_15) > 0) break;
        if (get_timer(TMR_16) > 0) break;
        if (get_timer(TMR_17) > 0) break;
        if (get_timer(TMR_18) > 0) break;
        state->oper = o_idle;
        break;

    default:
        break;

    }

    if (get_timer(TMR_1) == 0) STOP_ERR(e_err_tmr1);

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

