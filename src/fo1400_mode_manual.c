
#include <stdlib.h>

#include "hal.h"
#include "workset.h"
#include "fo1400_io.h"
#include "fo1400_common.h"
#include "fo1400_mode_adj.h"

static MAIN_STATUS op_junction(void);
static MAIN_STATUS op_inj_push(void);
static MAIN_STATUS op_inject(void);
static MAIN_STATUS op_load(void);
static MAIN_STATUS op_decompression(void);
static MAIN_STATUS op_inj_pop(void);
static MAIN_STATUS op_disjunction(void);

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

        if (KH1)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_junction;
            break;
        }
        if (KH3)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_inj_push;
            break;
        }
        if (KH5)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_inject;
            clr_scale_timer(TMR_SCALE_INJECT);
            uart_print("inj start\r\n");
            break;
        }
        if (KH6)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_load;
            break;
        }
        if (KH7)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_decompression;
            break;
        }
        if (KH4)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_inj_pop;
            break;
        }
        if (KH2)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_disjunction;
            break;
        }
        break;

    case o_junction:

        if (!KH1) OP_BREAK;
        state->status = op_junction();
        break;

    case o_inj_push:

        if (!KH3) OP_BREAK;
        state->status = op_inj_push();
        break;

    case o_inject:

        if (!KH5)
        {
            uart_print("inj stop\r\n");
            OP_BREAK;
        }
        state->status = op_inject();
        if (state->status == s_inject)
            uart_printf("inj %u %i\r\n", get_scale_timer(TMR_SCALE_INJECT), rand());
        break;

    case o_load:

        if (!KH6) OP_BREAK;
        state->status = op_load();
        break;

    case o_decompression:

        if (!KH7) OP_BREAK;
        state->status = op_decompression();
        break;

    case o_inj_pop:

        if (!KH4) OP_BREAK;
        state->status = op_inj_pop();
        break;

    case o_disjunction:

        if (!KH2) OP_BREAK;
        state->status = op_disjunction();
        break;

    default:
        break;
    }

}

static MAIN_STATUS op_junction(void)
{
    static MAIN_STATUS phase;

    if (BK1 && !BK52) phase = s_junction_break;
    if (BK52 && !BK50) phase = s_junction_fast;
    if (BK50 && !BK4) phase = s_junction_prev;
    if (BK4 && !BK2) phase = s_junction_lock;
    if (BK2) phase = s_done;

    switch (phase)
    {
    case s_junction_break:
        set_hydro(workset.hyd_U01);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
    case s_junction_fast:
        set_hydro(workset.hyd_U02);
        EM3(ON);
        EM16(ON);
        EM29(ON);
        EM1(ON);
        break;
    case s_junction_prev:
        set_hydro(workset.hyd_U03);
        EM3(ON);
        EM18(ON);
        EM1(ON);
    case s_junction_lock:
        set_hydro(workset.hyd_U04);
        EM3(ON);
        EM16(ON);
        EM1(ON);
        break;
    default:
        stop();
        break;
    }
    return phase;
}

static MAIN_STATUS op_inj_push(void)
{
    if (BK22)
    {
        stop();
        return s_done;
    }
    set_hydro(workset.hyd_U06);
    EM5(ON);
    EM16(ON);
    EM1(ON);
    return s_inj_push;
}

static MAIN_STATUS op_inject(void)
{
    static MAIN_STATUS phase;

    if (BK20 && !BK24) phase = s_inject_1;
    if (BK24 && !BK23) phase = s_inject_2;
    if (BK23) phase = s_done;

    switch (phase)
    {
    case s_inject_1:
        set_hydro(workset.hyd_U07);
        EM7(ON);
        EM16(ON);
        EM1(ON);
        break;
    case s_inject_2:
        set_hydro(workset.hyd_U08);
        EM7(ON);
        EM16(ON);
        EM1(ON);
        break;
    default:
        stop();
        break;
    }

    return phase;
}

static MAIN_STATUS op_load(void)
{
    if (BK25)
    {
        stop();
        return s_done;
    }
    set_hydro(workset.hyd_U10);
    EM2(ON);
    EM16(ON);
    EM13(ON);
    EM1(ON);
    return s_load;

}

static MAIN_STATUS op_decompression(void)
{
    if (BK20)
    {
        stop();
        return s_done;
    }
    set_hydro(workset.hyd_U11);
    EM29(ON);
    EM16(ON);
    EM12(ON);
    EM1(ON);
    return s_decompression;
}

static MAIN_STATUS op_inj_pop(void)
{
    if (BK21)
    {
        stop();
        return s_done;
    }
    set_hydro(workset.hyd_U13);
    EM6(ON);
    EM16(ON);
    EM1(ON);
    return s_inj_pop;
}

static MAIN_STATUS op_disjunction(void)
{
    static MAIN_STATUS phase;

    if (BK2 && !BK3) phase = s_disjunction_break;
    if (BK3 && !BK51) phase = s_disjunction_fast;
    if (BK51 && !BK1) phase = s_disjunction_slow;
    if (BK1) phase = s_done;

    switch (phase)
    {
    case s_disjunction_break:
        set_hydro(workset.hyd_U14);
        EM16(ON);
        EM4(ON);
        EM1(ON);
        break;
    case s_disjunction_fast:
        set_hydro(workset.hyd_U12);
        EM16(ON);
        EM4(ON);
        EM1(ON);
        break;
    case s_disjunction_slow:
        set_hydro(workset.hyd_U05);
        EM16(ON);
        EM4(ON);
        EM1(ON);
        break;
    default:
        stop();
        break;
    }

    return phase;
}
