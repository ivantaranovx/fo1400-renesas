
#ifndef ENC28J60_H
#define ENC28J60_H

typedef enum
{
    es_init = 0,

}
ETH_STATE;

ETH_STATE eth_state(void);

void eth_init(void);

#endif /* ENC28J60_H */

