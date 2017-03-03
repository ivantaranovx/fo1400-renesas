
#include "../hal.h"
#include "enc28j60.h"


static ETH_STATE state = es_init;

ETH_STATE eth_state(void)
{
    return state;
}

void eth_init(void)
{

}

