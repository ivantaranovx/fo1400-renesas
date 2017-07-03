
#ifndef TCPIP_H
#define TCPIP_H

#include <stdbool.h>

typedef struct {
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t mask[4];
    uint8_t gw[4];
    uint8_t srv[4];
    uint16_t id;
}
IPSET; // 24 bytes

uint16_t tcpip_get_id(void);

void tcpip_task(void);
void tcpip_reconf(void);
bool tcpip_ready(void);
bool tcpip_link(void);
bool tcpip_connected(void);
bool tcpip_send(char *str);

typedef void (*tcp_rx_cb_func)(char *data, uint16_t len);
void tcp_rx_cb_register(tcp_rx_cb_func f);

typedef void (*tcp_conn_cb_func)(bool connected);
void tcp_conn_cb_register(tcp_conn_cb_func f);

#endif /* TCPIP_H */

