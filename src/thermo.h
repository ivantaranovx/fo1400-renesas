
#ifndef THERMO_H
#define	THERMO_H

#include <stdint.h>
#include "workset.h"

#define TZ_MAX 5

void thermo_heat_enable(unsigned e);
unsigned thermo_heat_ok(void);

uint16_t thermo_get_int_temp(void);
uint16_t thermo_get_tz_temp(uint8_t ch);

void thermo_task(void);

#endif	/* THERMO_H */

