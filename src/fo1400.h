
#ifndef FO1400_H
#define	FO1400_H

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hal.h"
#include "dio.h"
#include "thermo.h"
#include "lcd.h"
#include "workset.h"
#include "helper.h"
#include "eeprom.h"
#include "ui/ui_main.h"
#include "eth/tcpip.h"

#include "fo1400_states.h"
#include "fo1400_io.h"

#define msg_machine "Formoplast 1400"
#define msg_version "v0.01"

void main_task(void);

void engine_enable(bool e);
void engine_task(void);

void lub_task(void);
void check_mode_selector(void);
void check_guard(void);
int check_heat(void);

typedef enum
{
    S_KH10 = 1 << 0,
    S_KH12 = 1 << 1,
    S_KH13 = 1 << 2,
    S_KH14 = 1 << 3,
    S_OT = 1 << 4,
    S_ENG = 1 << 5,
    S_CE = 1 << 6,
}
STN;

int check_kn(bool kn, STN stn);
void set_hydro(uint16_t speed);
void error(MAIN_ERROR e);

bool op_junction(void);

#endif	/* FO1400_H */

