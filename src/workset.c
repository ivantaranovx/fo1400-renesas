
#include <string.h>

#include "workset.h"
#include "eeprom.h"

#define USERS_MEM_OFFSET    ((WORKSET_NAME_LENGTH + sizeof (WORKSET)) * WORKSET_COUNT)

typedef struct {
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

WORKSET workset; // GLOBAL!

void get_param_limits(uint8_t idx, uint16_t *min, uint16_t *max) {

    if (idx > WORKSET_PARAM_COUNT) return;
    *min = workset_lim[idx].min;
    *max = workset_lim[idx].max;
}

void set_param(uint8_t idx, uint16_t val) {
    if (idx > WORKSET_PARAM_COUNT) return;
    uint16_t *ws = (uint16_t *) & workset;
    ws[idx] = val;
}

void trim_name(char *name, int length) {

    int i = 0;
    for (; i < length; i++) {
        if ((name[i] < 0x20) || (name[i] > 0x7F)) break;
    }
    for (; i < length; i++) {
        name[i] = 0x20;
    }
}

int get_workset_name_addr(uint8_t num) {

    int addr = WORKSET_NAME_LENGTH + sizeof (WORKSET);
    addr *= num;
    return addr;
}

int get_workset_addr(uint8_t num) {

    int addr = WORKSET_NAME_LENGTH + sizeof (WORKSET);
    addr *= num;
    return addr + WORKSET_NAME_LENGTH;
}

int get_user_name_addr(uint8_t num) {

    int addr = USER_NAME_LENGTH;
    addr *= num;
    addr += USERS_MEM_OFFSET;
    return addr;
}
