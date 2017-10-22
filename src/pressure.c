
#include "hal.h"

const float press_u[] = {
    3.41, 0,
    2.43, 40,
    1.44, 80,
    0.46, 120,
    0.003, 160,
    0, 165,
};

float get_pressure_mpa(uint8_t ch)
{
    int i;
    if (ch > 1) return 0;
    float r = get_adc_u(6 + ch);
    for (i = 0; press_u[i] > r; i += 2);
    if (i < 2) return press_u[1];
    r = (r - press_u[i]) / (press_u[i - 2] - press_u[i]);
    r *= press_u[i + 1] - press_u[i - 1];
    return press_u[i + 1] - r;
}

float get_pressure_kg(uint8_t ch)
{
    return get_pressure_mpa(ch) * 10.19716213;
}
