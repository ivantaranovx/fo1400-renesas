
#include <math.h>

#include "hal.h"
#include "dio.h"
#include "workset.h"

#include "thermo.h"

#define PWM_PULSE 50
#define PWM_PERIOD (PWM_PULSE * 100)

const uint8_t thermo_zones[TZ_MAX] =
{
    44, 43, 42, 41, 40
};

typedef struct
{
    unsigned f_heat_on      :1;

}
FLAGS;

extern WORKSET workset; // GLOBAL!

static FLAGS flags;

static uint16_t tz_current[TZ_MAX] = {0};
static uint16_t tz_pwm[TZ_MAX] = {0};

const double thermo_res[] =
{
    0.680, 1.000,
    0.930, 5.000,
    1.390, 10.000,
    1.570, 12.000,
    1.780, 15.000,
    1.950, 17.000,
    1.990, 18.000,
    2.110, 20.000,
    2.230, 22.000,
    2.400, 25.000,
    2.650, 30.000,
    2.840, 35.000,
    3.020, 40.000,
    3.180, 45.000,
    3.330, 50.000,
    3.440, 55.000,
    3.480, 57.000,
    0, 0
};

uint16_t thermo_get_int_temp(void)
{
    int i;
    double u = 0;
    double t = 0;
    double adc = get_adc_u(0);
    static uint16_t result = 0;

    if (adc == 0) return 0;

    // среднее по таблице
    for(i = 0;;i += 2)
    {
        if (thermo_res[i] == 0) return 59;
        if (adc < thermo_res[i]) break;
        u = thermo_res[i];
        t = thermo_res[i + 1];
    }
    adc -= u;
    u = adc / (thermo_res[i] - u);
    t = t + ((thermo_res[i + 1] - t) * u);
    //

    t = modf(t, &u) * 10;
    if ((t > 0) & (t < 9)) result = u;
    return result;
}

void thermo_heat_enable(unsigned e)
{
    int i;
    if (e)
    {
        clr_scale_timer(TMR_SCALE_TPWM);
    }
    else
    {
        for (i = 0; i < TZ_MAX; i++) dio_out(thermo_zones[i], 0);
        dio_flush();
    }
    flags.f_heat_on = e;
}

int thermo_get_tz_state(uint8_t ch)
{
    if (ch >= TZ_MAX) return -1;
    uint16_t *temp_z = &workset.temp_Z1;
    if (tz_current[ch] < (temp_z[ch] - workset.temp_under)) return -1;
    if (tz_current[ch] > (temp_z[ch] + workset.temp_over)) return 1;
    return 0;
}

uint16_t thermo_get_tz_temp(uint8_t ch)
{
    if (ch >= TZ_MAX) return 0;
    return tz_current[ch];
}

void thermo_task(void)
{
    uint16_t tmr;
    uint16_t *temp_z = &workset.temp_Z1;
    uint16_t *pwm_z = &workset.pwm_Z1;
    double c_temp;
    int i;

    tmr = get_scale_timer(TMR_SCALE_TPWM);

    if (tmr < PWM_PERIOD)
    {
        for (i = 0; i < TZ_MAX; i++)
        {
            if (tmr < tz_pwm[i]) continue;
            dio_out(thermo_zones[i], 0);
        }
        dio_flush();
        return;
    }

    clr_scale_timer(TMR_SCALE_TPWM);

    for (i = 0; i < TZ_MAX; i++)
    {
        c_temp = get_adc_u(i + 1);
        if (c_temp > 4.85) c_temp = 4.85;
        if (c_temp < 0.095) c_temp = 0.095;
        c_temp = ((4.85 - c_temp) * 102);
        tz_current[i] = (uint16_t)round(c_temp);

        if ((tz_current[i] < *temp_z) && flags.f_heat_on)
            tz_pwm[i] = (*pwm_z) * PWM_PULSE;
        else
            tz_pwm[i] = 0;

        temp_z++;
        pwm_z++;

        if (tz_pwm[i] > 0) dio_out(thermo_zones[i], 1);
        dio_flush();
    }
}
