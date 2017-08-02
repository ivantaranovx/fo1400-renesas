
#ifndef FO1400_COMMON_H
#define FO1400_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#include "fo1400_states.h"

#define f_ready         state.flags.f.ready
#define f_cycle_stop    state.flags.f.cycle_stop
#define f_cycle_report  state.flags.f.cycle_report
#define f_heat_on       state.flags.f.heat_on
#define f_power_on      state.flags.f.power_on
#define f_guard_chk     state.flags.f.guard_chk
#define f_guard_ok      state.flags.f.guard_ok
#define f_engine_on     state.flags.f.engine_on

#define OP_BREAK { stop(); state->oper = o_idle; break; }
#define STOP_ERR(e) { stop(); state->oper = o_idle; state->error = e; }
#define BREAK_ERR(e) { stop(); state->oper = o_idle; state->error = e; break; }

void stop(void);
void set_hydro(uint16_t speed);

bool engine_ready(MAIN_STATE *state);

bool check_guard(MAIN_STATE *state);
bool check_engine(MAIN_STATE *state);
bool check_heat(MAIN_STATE *state);

typedef enum {
    S_KH10 = 1 << 0,
    S_KH12 = 1 << 1,
    S_KH13 = 1 << 2,
    S_KH14 = 1 << 3,
    S_ENG = 1 << 4,
    S_CE = 1 << 5,
    S_KM1 = 1 << 6,
    S_KH1 = 1 << 7,
    S_KH3 = 1 << 8,
    S_KH5 = 1 << 9,
    S_KH6 = 1 << 10,
    S_KH7 = 1 << 11,
    S_KH4 = 1 << 12,
    S_KH2 = 1 << 13,
    S_KH0 = 1 << 14,
}
STN;

int check_kn(bool kn, STN stn);

#endif /* FO1400_COMMON_H */

