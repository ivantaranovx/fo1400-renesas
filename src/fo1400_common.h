
#ifndef FO1400_COMMON_H
#define FO1400_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#include "fo1400_states.h"

#define OP_BREAK { stop(); state->oper = o_idle; break; }

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
    S_OT = 1 << 4,
    S_ENG = 1 << 5,
    S_CE = 1 << 6,
    S_KM1 = 1 << 7,
}
STN;

int check_kn(bool kn, STN stn);

#endif /* FO1400_COMMON_H */

