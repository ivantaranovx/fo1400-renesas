
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
#include "eth/tcpip.h"
#include "ui/ui_main.h"

#include "fo1400_io.h"

/* +lcdconv */

const char msg_machine[] = "Formoplast 1400";
const char msg_version[] = "v0.02";
const char err_eeprom[] = "Ошибка EEPROM";
const char err_wdt[] = "Системный сбой";

/* -lcdconv */

void wait_keypress(void);
void main_task(void);
void check_mode_selector(void);
void engine_task(void);
void lub_task(void);
void guard_task(void);
void engine_enable(bool e);

void uart_rx_cb(uint8_t b);

void tcp_conn_cb(bool connected);
void tcp_rx_cb(char *data, uint16_t len);

#endif /* FO1400_H */

