
#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include <stdbool.h>

void utoa(uint16_t val, char *buf, uint8_t pdot);
void hex(uint8_t val, char *buf);

void set_char(char key, char *str);

void print_name(uint16_t idx, char *name);
void print_uint(uint8_t pos, uint16_t val, uint8_t dot);
void print_ex(uint8_t pos, uint8_t type, void *ptr);

bool check_int(int val, int *store);
bool check_uint16(uint16_t val, uint16_t *store);
bool check_uint32(uint32_t val, uint32_t *store);

void ui_input_int(uint8_t pos, uint16_t val, uint8_t dot);
int ui_input_int_process(uint8_t key);
void ui_input_int_get(uint16_t *res);

#endif /* HELPER_H */

