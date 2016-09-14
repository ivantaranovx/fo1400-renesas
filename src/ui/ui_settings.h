
#ifndef UI_SETTINGS_H
#define	UI_SETTINGS_H

#include "../workset.h"

#define WORKSET_COUNT	100

void ui_settings_set_workset(WORKSET *set);
int ui_settings(char key);

uint16_t get_param(uint8_t idx);
void set_param(uint8_t idx, uint16_t val);

unsigned workset_load_name(uint8_t num, char *name);
unsigned workset_save_name(uint8_t num, char *name);

unsigned workset_save_param(uint8_t num, uint8_t idx, uint16_t val);

#endif

