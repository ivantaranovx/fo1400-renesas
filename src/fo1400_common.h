
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

#endif /* FO1400_COMMON_H */

