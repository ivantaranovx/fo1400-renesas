
#include <math.h>

#include "hal.h"
#include "fo1400_io.h"
#include "fo1400_common.h"
#include "fo1400_states.h"
#include "thermo.h"

void stop(void)
{
    set_hydro_u(0);

    EM18(OFF);
    EM29(OFF);
    EM16(OFF);
    EM1(OFF);
    EM13(OFF);
    EM30(OFF);
    EM4(OFF);

    EM3(OFF);
    EM6(OFF);
    EM5(OFF);
    EM12(OFF);
    EM7(OFF);
    EM2(OFF);

    EM40(OFF);
    EM41(OFF);
}

void set_hydro_u(uint16_t speed)
{
    double dac = speed;
    dac /= 9.8039;
    if (dac > 255) dac = 255;
    set_dac(0, (uint8_t) round(dac));
}

void set_hydro_p(uint16_t pressure)
{
    double dac = pressure;
    dac *= 0.627451;
    if (dac > 255) dac = 255;
    set_dac(1, (uint8_t) round(dac));
}

bool check_guard(MAIN_STATE *state)
{
    if (state->flags.f.guard_ok) return true;
    state->err[2] = e_guard_chk;
    return false;
}

bool engine_ready(MAIN_STATE *state)
{
    return state->flags.f.ready && state->flags.f.power_on && state->flags.f.engine_on;
}

bool check_engine(MAIN_STATE *state)
{
    if (engine_ready(state)) return true;
    state->err[1] = e_engine_off;
    return false;
}

bool check_heat(MAIN_STATE *state)
{
    for (int i = 0; i < TZ_MAX; i++)
    {
        if (thermo_get_tz_state(i) < 0)
        {
            state->err[0] = e_not_warmed;
            return false;
        }
    }
    return true;
}

int check_kn(bool kn, STN stn)
{
    static uint16_t store = false;
    if (kn ^ ((store & stn) ? true : false))
    {
        store ^= stn;
        if (kn) return 1;
        return -1;
    }
    return 0;
}
