
#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include <stdbool.h>

void utoa(uint16_t val, char *buf, uint8_t pdot);
void hex(uint8_t val, char *buf);

void set_char(char key, char *str);

void print_name(uint8_t idx, char *name);
void print_uint(uint8_t pos, uint16_t val, uint8_t dot);

bool check_int(int val, int *store);

void ui_input_int(uint8_t pos, uint16_t val, uint8_t dot);
int ui_input_int_process(uint8_t key);
uint16_t ui_input_int_get(void);

#endif /* HELPER_H */

