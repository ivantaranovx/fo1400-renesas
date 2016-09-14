
#ifndef FO1400_H
#define	FO1400_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define msg_machine "Formoplast 1400"
#define msg_version "v0.01"

typedef enum
{
    unknown = 0,
    adjustment = 1,
    manual = 2,
    semi_auto = 4,
    full_auto = 8
}
MAIN_MODE;

typedef enum
{
    idle,

    junction_start,
    junction_full,
    junction_end,

    inj_push_start,
    inj_push_end,

    inject_start,
    inject_process,
  
    load_start,
    load_end,

    decompression_start,
    decompression_end,

    inj_pop_start,
    inj_pop_end,

    disjunction_start,
    disjunction_process,
    disjunction_end,

    emergency_stop,
    operation_end,
    wait_buttons_release
}
MAIN_OPER;

typedef enum
{
    e_success = 0,
    e_not_warmed,
    e_guard_56,
    e_guard_57,
    e_emergency,
    e_engine_off,
    e_engine_not_ready,
    e_engine_overheat
}
MAIN_ERROR;

typedef struct
{
    unsigned cycle_stop   :1;
    unsigned power_on     :1;
    unsigned guard_ok     :1;
    unsigned engine_on    :1;
    unsigned heat_ok      :1;
}
MAIN_FLAGS;

typedef union
{
    MAIN_FLAGS  f;
    int         v;
}
MAIN_FLAGS_V;

typedef struct
{
    MAIN_MODE mode;
    MAIN_OPER oper;
    MAIN_ERROR error;
    MAIN_FLAGS_V flags;
}
MAIN_STATE;

void main_task(void);
void engine_task(void);

char temp_sym(uint16_t current, uint16_t etal);
void all_em_off(void);
void set_hydro(uint16_t speed);

MAIN_STATE *main_get_state(void);

#endif	/* FO1400_H */

