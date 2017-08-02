
#include <stdbool.h>

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

void op_mode_adj(MAIN_STATE *state)
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
            state->error = e_success;
            state->oper = o_junction;
            break;
        }
        if (check_kn(KH3, S_KH3) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_inj_push;
            break;
        }
        if (check_kn(KH5, S_KH5) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_inject;
            break;
        }
        if (check_kn(KH6, S_KH6) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_load;
            break;
        }
        if (check_kn(KH7, S_KH7) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_heat(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_decompression;
            break;
        }
        if (check_kn(KH4, S_KH4) > 0)
        {
            if (!check_engine(state)) break;
            if (!check_guard(state)) break;
            state->error = e_success;
            state->oper = o_inj_pop;
            break;
        }
        if (check_kn(KH2, S_KH2) > 0)
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

        if (!KH5) OP_BREAK;
        state->status = op_inject();
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
    if (BK2)
    {
        stop();
        return s_done;
    }
    if (!BK4)
    {
        set_hydro(workset.hyd_U01);
        EM16(ON);
        EM3(ON);
        EM1(ON);
        EM29(ON);
        return s_junction_slow;
    }
    else
    {
        set_hydro(workset.hyd_U01);
        EM16(ON);
        EM3(ON);
        EM1(ON);
        EM29(OFF);
        return s_junction_full;
    }
}

static MAIN_STATUS op_inj_push(void)
{
    if (BK22)
    {
        stop();
        return s_done;
    }
    set_hydro(workset.hyd_U06);
    EM16(ON);
    EM5(ON);
    EM1(ON);
    return s_inj_push;
    ;
}

static MAIN_STATUS op_inject(void)
{
    if (BK23)
    {
        stop();
        return s_done;
    }
    set_hydro(workset.hyd_U07);
    EM16(ON);
    EM7(ON);
    EM1(ON);
    return s_inject;
}

static MAIN_STATUS op_load(void)
{
    if (BK25)
    {
        stop();
        return s_done;
    }
    set_hydro(workset.hyd_U10);
    EM16(ON);
    EM2(ON);
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
    EM16(ON);
    EM29(ON);
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
    EM16(ON);
    EM6(ON);
    EM1(ON);
    return s_inj_pop;
}

static MAIN_STATUS op_disjunction(void)
{
    int r1, r2;

    if (BK1)
    {
        set_hydro(0);
        EM16(OFF);
        EM4(OFF);
        EM1(OFF);
    }
    else
    {
        set_hydro(workset.hyd_U05);
        EM16(ON);
        EM4(ON);
        EM1(ON);
        if (workset.p_s)
        {
            EM40(ON);
            set_timer(TMR_14, (uint32_t) (workset.tmr_T14) * 10);
            EM41(ON);
            set_timer(TMR_16, (uint32_t) (workset.tmr_T16) * 10);
        }
        return s_disjunction;
    }

    r1 = get_timer(TMR_14); 
    if (r1 == 0) EM40(OFF);

    r2 = get_timer(TMR_16); 
    if (r2 == 0) EM41(OFF);

    if ((r1 > 0) || (r2 > 0)) return s_disjunction;
    return s_done;
}
