
#ifndef FO1400_STATES_H
#define FO1400_STATES_H

typedef enum {
    m_unknown = 0,
    m_adjust = 1,
    m_manual = 2,
    m_semi = 4,
    m_auto = 8
}
MAIN_MODE;

typedef enum {
    o_idle,
    o_pause,
    o_junction,
    o_junction_break,
    o_junction_prev,
    o_junction_lock,
    o_junction_fast,
    o_inj_push,
    o_inject,
    o_inject_1,
    o_inject_2,
    o_form_hi,
    o_form,
    o_load,
    o_decompression,
    o_inj_pop,
    o_cooling,
    o_disjunction,
    o_disjunction_break,
    o_disjunction_fast,
    o_disjunction_slow,
    o_jump,
}
MAIN_OPER;

typedef enum {
    s_idle,
    s_done,
    s_junction_slow,
    s_junction_full,
    s_junction_break,
    s_junction_fast,
    s_junction_prev,
    s_junction_lock,
    s_inj_push,
    s_inject,
    s_inject_1,
    s_inject_2,
    s_form,
    s_load,
    s_decompression,
    s_inj_pop,
    s_cooling,
    s_disjunction,
    s_disjunction_break,
    s_disjunction_fast,
    s_disjunction_slow,
    s_jump,
}
MAIN_STATUS;

typedef enum {
    e_success = 0,
    e_not_powered,
    e_not_warmed,
    e_guard_chk,
    e_guard_56,
    e_guard_57,
    e_emergency_stop,
    e_engine_off,
    e_engine_not_ready,
    e_engine_overheat,
    e_lub_low,
    e_err_bk1,
    e_err_bk2,
    e_err_bk20,
    e_err_bk21,
    e_err_bk22,
    e_err_bk23,
    e_err_bk25,
    e_err_bk53,
    e_err_tmr1,
    e_err_tmr2,
    e_err_tmr3,
    e_err_tmr6,
    e_err_tmr8,

}
MAIN_ERROR;

typedef struct {
    unsigned cycle_stop : 1;
    unsigned power_on : 1;
    unsigned guard_chk : 1;
    unsigned guard_ok : 1;
    unsigned engine_on : 1;
    unsigned heat_on : 1;
    unsigned ready : 1;
    unsigned cycle_report : 1;
}
MAIN_FLAGS;

typedef union {
    MAIN_FLAGS f;
    int v;
}
MAIN_FLAGS_V;

typedef struct {
    MAIN_MODE mode;
    MAIN_OPER oper;
    MAIN_STATUS status;
    MAIN_ERROR error;
    MAIN_FLAGS_V flags;
    uint16_t prod_id;
    uint16_t user_id;
    uint16_t user_cf;
}
MAIN_STATE;

#endif /* FO1400_STATES_H */

