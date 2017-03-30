
#include <stdbool.h>

#include "fo1400_io.h"
#include "fo1400_common.h"
#include "fo1400_mode_auto.h"
#include "hal.h"
#include "workset.h"

extern WORKSET workset; // GLOBAL!
static bool nozzle_out = true;

void op_mode_auto(MAIN_STATE *state)
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
        if (!check_engine(state)) break;
        if (!check_heat(state)) break;
        if (!check_guard(state)) break;
        state->error = e_success;
        if ((state->mode == m_semi) && !KH1) break;
        if (!BK53) break;
        state->oper = o_junction_break;
        set_timer(TMR_1, workset.tmr_T01);
        break;

    case o_junction_break:
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
        set_timer(TMR_2, workset.tmr_T02);
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
        state->oper = o_inj_push;
        break;

    case o_inj_push:
        if (nozzle_out)
        {
            state->status = s_inj_push;
            set_hydro(workset.hyd_U06);
            EM5(ON);
            EM16(ON);
            EM1(ON);
            if (!BK22) break;
            if (!BK20) break;
            stop();
        }
        state->oper = o_inject_1;
        set_timer(TMR_3, workset.tmr_T03);
        break;

    case o_inject_1:
        state->status = s_inject_1;
        set_hydro(workset.hyd_U07);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
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
        if (!BK23) break;
        stop();
        state->oper = o_form_hi;
        set_timer(TMR_10, workset.tmr_T10);
        break;

    case o_form_hi:
        state->status = s_form;
        set_hydro(workset.hyd_U15);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        if (get_timer(TMR_10) > 0) break;
        stop();
        state->oper = o_form;
        set_timer(TMR_4, workset.tmr_T04);
        break;

    case o_form:
        state->status = s_form;
        set_hydro(workset.hyd_U09);
        EM5(ON);
        EM16(ON);
        EM7(ON);
        EM1(ON);
        EM30(ON);
        if (get_timer(TMR_4) > 0) break;
        stop();
        state->oper = o_load;
        set_timer(TMR_5, workset.tmr_T05);
        set_timer(TMR_6, workset.tmr_T06);
        break;

    case o_load:
        state->status = s_load;
        set_hydro(workset.hyd_U10);
        EM2(ON);
        EM16(ON);
        EM13(ON);
        EM1(ON);
        EM5(ON);
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
        if (!BK20) break;
        stop();
        state->oper = o_inj_pop;
        break;

    case o_inj_pop:
        nozzle_out = false;
        if (workset.nozzle_out)
        {
            nozzle_out = true;
            state->status = s_inj_pop;
            set_hydro(workset.hyd_U11);
            EM6(ON);
            EM16(ON);
            EM1(ON);
            if (!BK21) break;
            stop();
        }
        state->oper = o_cooling;
        break;

    case o_cooling:
        state->status = s_cooling;
        if (get_timer(TMR_5) > 0) break;
        state->oper = o_disjunction_break;
        break;

    case o_disjunction_break:
        state->status = s_disjunction_break;
        set_hydro(workset.hyd_U14);
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (!BK3) break;
        stop();
        state->oper = o_disjunction_fast;
        break;

    case o_disjunction_fast:
        state->status = s_disjunction_fast;
        set_hydro(workset.hyd_U12);
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (!BK51) break;
        stop();
        state->oper = o_disjunction_slow;
        set_timer(TMR_8, workset.tmr_T08);
        break;

    case o_disjunction_slow:
        state->status = s_disjunction_slow;
        set_hydro(workset.hyd_U05);
        EM4(ON);
        EM16(ON);
        EM1(ON);
        if (!BK1) break;
        if (get_timer(TMR_8) > 0) break;
        stop();
        state->oper = o_jump;
        break;

    case o_jump:
        state->status = s_jump;
        set_hydro(workset.hyd_U16);
        EM3(ON);
        EM16(ON);
        EM1(ON);
        if (!BK53) break;
        stop();
        state->oper = o_done;
        set_timer(TMR_9, workset.tmr_T09);
        break;

    case o_done:
        state->status = s_done;
        if (get_timer(TMR_9) > 0) break;
        if (get_timer(TMR_1) > 0) break;
        state->oper = o_idle;
        break;

    default:
        break;

    }

}

