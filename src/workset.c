
#include <string.h>

#include "workset.h"
#include "24lc512.h"

typedef struct
{
    uint16_t    min;
    uint16_t    max;
}
WORKSET_LIM;

static const WORKSET_LIM workset_lim[] =
{
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

void get_param_limits(uint8_t num, uint16_t *min, uint16_t *max)
{
    if (num > WORKSET_PARAM_COUNT) return;
    *min = workset_lim[num].min;
    *max = workset_lim[num].max;
}

void check_param(uint8_t num, uint16_t *param)
{
    if (num > WORKSET_PARAM_COUNT) return;
    if ((*param < workset_lim[num].min) || (*param > workset_lim[num].max))
        *param = workset_lim[num].min;
}

unsigned workset_load(uint8_t num, WORKSET *set)
{
    uint16_t *ptr = &set->prod_count;
    int i = num;
    if (num > WORKSET_PARAM_COUNT) return 1;
    i *= sizeof(WORKSET);
    i += ADDR_WORKSET;

    if (readArray(i, (uint8_t*)set, sizeof(WORKSET)) != sizeof(WORKSET)) memset(set, 0xFF, sizeof(WORKSET));

    for(i = 0; i < WORKSET_PARAM_COUNT; i++) check_param(i, ptr++);
    return 0;
}

unsigned workset_save(uint8_t num, WORKSET *set)
{
    uint16_t dst = num;
    uint8_t *src = (uint8_t*)set;
    uint16_t len = sizeof(WORKSET);
    uint8_t l;
    uint8_t i;
    if (num > WORKSET_PARAM_COUNT) return 0;
    dst *= sizeof(WORKSET);
    dst += ADDR_WORKSET;
    while (len)
    {
        if (len > 128) l = 128; else l = len;
        for(i = 0; i < 20; i++) if (writeArray(dst, src, l) == l) break;
        if (i == 20) return 0;
        src += l;
        dst += l;
        len -= l;
    }
    return 1;
}

