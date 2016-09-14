
#include <stdint.h>

#ifndef _USERSET_H_
#define _USERSET_H_

#define ADDR_USERS  0x3900
#define NUM_USERS	100

#define user_name_sz    16

typedef struct
{
    char        user_name[user_name_sz];
}
USERSET;

#endif
