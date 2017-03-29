
#include <stdlib.h>

#include "hal.h"
#include "fo1400_states.h"
#include "fo1400_io.h"
#include "fo1400_common.h"
#include "workset.h"

int op_junction(void);
int op_inj_push(void);
int op_inject(void);
int op_load(void);
int op_decompression(void);
int op_inj_pop(void);
int op_disjunction(void);

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
        switch (op_junction())
        {
        case 1:
            state->status = s_junction_slow;
            break;
        case 2:
            state->status = s_junction_full;
            break;
        default:
            state->status = s_done;
            break;
        }
        break;

    case o_inj_push:

        if (!KH3) OP_BREAK;
        switch (op_inj_push())
        {
        case 1:
            state->status = s_inj_push;
            break;
        default:
            state->status = s_done;
            break;
        }
        break;

    case o_inject:

        if (!KH5)
        {
            uart_print("inj stop\r\n");
            OP_BREAK;
        }
        switch (op_inject())
        {
        case 1:
            state->status = s_inject;
            uart_printf("inj %u %i\r\n", get_scale_timer(TMR_SCALE_INJECT), rand());
            break;
        default:
            state->status = s_done;
            break;
        }
        break;

    case o_load:

        if (!KH6) OP_BREAK;
        switch (op_load())
        {
        case 1:
            state->status = s_load;
            break;
        default:
            state->status = s_done;
            break;
        }
        break;

    case o_decompression:

        if (!KH7) OP_BREAK;
        switch (op_decompression())
        {
        case 1:
            state->status = s_decompression;
            break;
        default:
            state->status = s_done;
            break;
        }
        break;

    case o_inj_pop:

        if (!KH4) OP_BREAK;
        switch (op_inj_pop())
        {
        case 1:
            state->status = s_inj_pop;
            break;
        default:
            state->status = s_done;
            break;
        }
        break;

    case o_disjunction:

        if (!KH2) OP_BREAK;
        switch (op_disjunction())
        {
        case 1:
            state->status = s_disjunction;
            break;
        default:
            state->status = s_done;
            break;
        }
        break;
    }

}

int op_junction(void)
{
    if (BK2) return 0;
    set_hydro(workset.hyd_U01);
    EM16(ON);
    EM3(ON);
    EM1(ON);
    if (!BK4)
    {
        EM29(ON);
        return 1;
    }
    EM29(OFF);
    return 2;
}

int op_inj_push(void)
{
    if (BK22) return 0;
    set_hydro(workset.hyd_U06);
    EM16(ON);
    EM5(ON);
    EM1(ON);
    return 1;
}

int op_inject(void)
{
    if (BK23) return 0;
    set_hydro(workset.hyd_U07);
    EM16(ON);
    EM7(ON);
    EM1(ON);
    return 1;
}

int op_load(void)
{
    if (BK25) return 0;
    set_hydro(workset.hyd_U10);
    EM16(ON);
    EM2(ON);
    EM13(ON);
    EM1(ON);
    return 1;
}

int op_decompression(void)
{
    if (BK20) return 0;
    set_hydro(workset.hyd_U11);
    EM16(ON);
    EM29(ON);
    EM12(ON);
    EM1(ON);
    return 1;
}

int op_inj_pop(void)
{
    if (BK21) return 0;
    set_hydro(workset.hyd_U13);
    EM16(ON);
    EM6(ON);
    EM1(ON);
    return 1;
}

int op_disjunction(void)
{
    if (BK1) return 0;
    set_hydro(workset.hyd_U05);
    EM16(ON);
    EM4(ON);
    EM1(ON);
    return 1;
}
