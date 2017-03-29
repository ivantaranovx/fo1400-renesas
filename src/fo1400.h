
#ifndef FO1400_H
#define FO1400_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hal.h"
#include "thermo.h"
#include "lcd.h"
#include "workset.h"
#include "misc.h"
#include "eeprom.h"
#include "ui/ui_main.h"
#include "eth/tcpip.h"


#include "fo1400_io.h"

/* +lcdconv */

const char msg_machine[] = "Formoplast 1400";
const char msg_version[] = "v0.01";
const char err_eeprom[] = "Ошибка EEPROM";

/* -lcdconv */

void main_task(void);

void check_mode_selector(void);

void engine_task(void);
void lub_task(void);
void guard_task(void);

void engine_enable(bool e);

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

#endif /* FO1400_H */

