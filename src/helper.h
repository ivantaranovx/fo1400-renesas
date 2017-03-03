
#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include <stdbool.h>

void utoa(uint16_t val, char *buf, uint8_t pdot);
void hex(uint8_t val, char *buf);

void uart_uint(uint16_t val);
unsigned lcd_uint(uint8_t pos, uint16_t val, uint8_t c);

void set_char(char key, char *str);
void print_name(uint8_t idx, char *name);

bool check_int(int val, int *store);

#endif /* HELPER_H */

