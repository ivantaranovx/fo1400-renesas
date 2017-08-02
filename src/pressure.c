
#include "hal.h"

const double press_u[] = {
    3.41, 0,
    2.43, 40,
    1.44, 80,
    0.46, 120,
    0.003, 160,
    0, 165,
};

double get_pressure_mpa(void)
{
    int i;
    double r = get_adc_u(6);
    for (i = 0; press_u[i] > r; i += 2);
    if (i < 2) return press_u[1];
    r = (r - press_u[i]) / (press_u[i - 2] - press_u[i]);
    r *= press_u[i + 1] - press_u[i - 1];
    return press_u[i + 1] - r;
}

double get_pressure_kgf_cm2(void)
{
    return get_pressure_mpa() * 10.19716213;
}
