
#include "hal.h"

const double press_u[] = {
    4.85, -40,
    4.146, -10,
    3.908, 0,
    3.67, 10,
    2.48, 60,
    1.28, 110,
    0.338, 150,
    0.095, 160,
    0, 161,
};

double get_pressure_mpa(void)
{
    int i;
    double r = adc_get_u(6);
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
