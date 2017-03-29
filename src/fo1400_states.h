
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
    o_junction,
    o_inj_push,
    o_inject,
    o_load,
    o_decompression,
    o_inj_pop,
    o_disjunction,
}
MAIN_OPER;

typedef enum {
    s_idle,
    s_done,
    s_junction_slow,
    s_junction_full,
    s_inj_push,
    s_inject,
    s_load,
    s_decompression,
    s_inj_pop,
    s_disjunction,
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
}
MAIN_STATE;

#endif /* FO1400_STATES_H */

