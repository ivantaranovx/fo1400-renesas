
#include "../hal.h"
#include "enc28j60.h"

#include <stdlib.h>

void tcpip_task(void)
{
    static int state = 0;

    switch(state)
    {
        case 0:
            eth_init();
            state = 1;
            break;

    }
}

