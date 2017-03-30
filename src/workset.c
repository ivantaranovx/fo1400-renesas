
#include <string.h>
#include <stdbool.h>

#include "workset.h"
#include "eeprom.h"

#define USERS_MEM_OFFSET    ((WORKSET_NAME_LENGTH + sizeof (WORKSET)) * WORKSET_COUNT)

typedef struct
{
    uint16_t min;
    uint16_t max;
}
WORKSET_LIM;

static const WORKSET_LIM workset_lim[] = {
    {1, 0xFFFF},
    {0, 1},
    {1, 200},
    {0, 1},
    {0, 1},
    {1, 100},
    {1, 20},
    {1, 50},
    {1, 10},
    {5, 50},
    {5, 50},

    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},
    {1, 25000},

    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},
    {0, 2490},

    {50, 370},
    {50, 370},
    {50, 370},
    {50, 370},
    {50, 370},

    {1, 99},
    {1, 99},
    {1, 99},
    {1, 99},
    {1, 99},

    {0, 165},
    {0, 165},
    {0, 165},
    {0, 165},
    {0, 165},

    {0, 0},
    {0, 0},
    {0, 0}
};

extern WORKSET workset; // GLOBAL!

void get_param_limits(uint8_t idx, uint16_t *min, uint16_t *max)
{
    if (idx > WORKSET_PARAM_COUNT) return;
    if (min > 0) *min = workset_lim[idx].min;
    if (max > 0) *max = workset_lim[idx].max;
}

void check_limit(uint8_t idx, uint16_t *val)
{
    if (idx > WORKSET_PARAM_COUNT) return;
    if (val == 0) return;
    if (*val < workset_lim[idx].min) *val = workset_lim[idx].min;
    if (*val > workset_lim[idx].max) *val = workset_lim[idx].min;
}

void set_param(uint8_t idx, uint16_t val)
{
    if (idx > WORKSET_PARAM_COUNT) return;
    uint16_t *ws = (uint16_t *) & workset;
    ws[idx] = val;
}

void trim_name(char *name, int length)
{
    bool trim = false;
    for (; length > 0; length--, name++)
    {
        if ((*name < 0x20) || (*name > 0x7F)) trim = true;
        if (trim) *name = 0x20;
    }
}

int get_workset_name_addr(uint8_t num)
{
    int addr = WORKSET_NAME_LENGTH + sizeof (WORKSET);
    addr *= num;
    return addr;
}

int get_workset_addr(uint8_t num)
{
    int addr = WORKSET_NAME_LENGTH + sizeof (WORKSET);
    addr *= num;
    return addr + WORKSET_NAME_LENGTH;
}

int get_user_name_addr(uint8_t num)
{
    int addr = USER_NAME_LENGTH;
    addr *= num;
    addr += USERS_MEM_OFFSET;
    return addr;
}

int workset_save(uint8_t idx)
{
    int addr = get_workset_addr(idx);
    eeprom_cs(0, addr);
    eeprom_write((uint8_t*) & workset, sizeof (workset));
    return eeprom_status_wait();
}

int workset_load(uint8_t idx)
{
    int addr = get_workset_addr(idx);
    eeprom_cs(0, addr);
    eeprom_read((uint8_t*) & workset, sizeof (workset));
    if (eeprom_status_wait() == 0) return 0;
    uint16_t *ws = (uint16_t *) & workset;
    for (int i = 0; i < WORKSET_PARAM_COUNT; i++) check_limit(i, &ws[i]);
    if (idx == 0) return 1;
    addr = get_workset_addr(0);
    eeprom_cs(0, addr);
    eeprom_write((uint8_t*) & workset, sizeof (workset));
    return eeprom_status_wait();
}

