
#include "../hal.h"
#include "../eeprom.h"
#include "../eeprom_addr.h"

#include "enc28j60.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/appconf.h"

#include "tcpip.h"

#include <string.h>
#include <stdbool.h>

#define SRV_PORT 34567

/*
 * port6:0 ENC /CS
 * port6:1 ENC CLK
 * port6:2 ENC MISO
 * port6:3 ENC MOSI
 * port6:4 ENC /INT
 * port6:5 ENC /RESET
 */

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

const uint8_t IPSET_DEF[] = {
    0xff, 0xfe, 0xfd, 0x00, 0x00, 0x01,
    192, 168, 9, 123,
    255, 255, 255, 0,
    192, 168, 9, 1,
    192, 168, 9, 10,
    1, 0,
};

static int state = 0;
static int err;
static struct uip_conn *conn = 0;
static int tcpflags;
static int conndly = 0;
static bool lnk = false;
static tcp_conn_cb_func tcp_conn_cb = 0;
static tcp_rx_cb_func tcp_rx_cb = 0;

#define tcp_ring_buf_sz 1024
static uint8_t tcp_ring_buf[1024];
static uint8_t *tcp_in_ptr = tcp_ring_buf;
static uint8_t *tcp_out_ptr = tcp_ring_buf;
static int tlen = 0;

IPSET ipset;

uint16_t tcpip_get_id(void)
{
    return ipset.id;
}

void tcpip_reconf(void)
{
    state = 2;
}

bool tcpip_ready(void)
{
    return (state == 4);
}

bool tcpip_link(void)
{
    return lnk;
}

bool tcpip_connected(void)
{
    if (conn == 0) return false;
    return (conn->tcpstateflags == UIP_ESTABLISHED);
}

void tcpip_task(void)
{
    int i;
    uip_ipaddr_t ipaddr;

    switch (state)
    {
    case 0:

        i = enc28j60getrev();
        if (i == 0) break;
        if (i == 0xFF) break;
        state = 1;
        break;

    case 1:

        eeprom_cs(0, IPSET_ADDR);
        eeprom_read((uint8_t*) & ipset, sizeof (ipset));
        state = 3;
        break;

    case 2:

        eeprom_cs(0, IPSET_ADDR);
        eeprom_write((uint8_t*) & ipset, sizeof (ipset));
        state = 3;
        break;

    case 3:

        i = eeprom_status();
        if (i < 0) break;
        if (memcmp((uint8_t*) & ipset, IPSET_DEF, 3))
        {
            memcpy((uint8_t*) & ipset, IPSET_DEF, sizeof (ipset));
        }

        conn = 0;
        tcpflags = -1;
        err = 0;

        enc28j60Init(ipset.mac);

        uip_init();
        uip_arp_init();

        memcpy(uip_ethaddr.addr, ipset.mac, 6);

        uip_ipaddr(ipaddr, ipset.ip[0], ipset.ip[1], ipset.ip[2], ipset.ip[3]);
        uip_sethostaddr(ipaddr);
        uip_ipaddr(ipaddr, ipset.mask[0], ipset.mask[1], ipset.mask[2], ipset.mask[3]);
        uip_setnetmask(ipaddr);
        uip_ipaddr(ipaddr, ipset.gw[0], ipset.gw[1], ipset.gw[2], ipset.gw[3]);
        uip_setdraddr(ipaddr);

        set_timer(TMR_UIP, 100);
        set_timer(TMR_ARP, 100);

        state = 4;
        break;

    case 4:

        if (eth_int())
        {
            uip_len = enc28j60PacketReceive(UIP_BUFSIZE, (uint8_t *) uip_buf);
        }

        if (uip_len > 0)
        {
            if (BUF->type == htons(UIP_ETHTYPE_IP))
            {
                uip_arp_ipin();
                uip_input();
                if (uip_len > 0)
                {
                    uip_arp_out();
                    enc28j60PacketSend(uip_len, (uint8_t *) uip_buf);
                }
            }
            else if (BUF->type == htons(UIP_ETHTYPE_ARP))
            {
                uip_arp_arpin();
                if (uip_len > 0)
                {
                    enc28j60PacketSend(uip_len, (uint8_t *) uip_buf);
                }
            }
        }

        if (get_timer(TMR_ARP) < 0)
        {
            set_timer(TMR_ARP, 10000);
            uip_arp_timer();
        }

        if (get_timer(TMR_UIP) < 0)
        {
            set_timer(TMR_UIP, 100);

            i = enc28j60getrev();
            if ((i == 0) || (i == 0xFF))
            {
                err++;
                if (err < 3) break;

                eth_rst();
                state = 0;
                break;
            }

            lnk = (enc28j60linkup() > 0);

            err = 0;

            for (i = 0; i < UIP_CONNS; i++)
            {
                uip_periodic(i);
                if (uip_len > 0)
                {
                    uip_arp_out();
                    enc28j60PacketSend(uip_len, (uint8_t *) uip_buf);
                }
            }

            if (conndly > 0) conndly--;

            if (lnk && (conn == 0) && (conndly == 0))
            {
                uip_ipaddr(ipaddr, ipset.srv[0], ipset.srv[1], ipset.srv[2], ipset.srv[3]);
                conn = uip_connect(&ipaddr, HTONS(SRV_PORT));
            }

            if (tcpflags != conn->tcpstateflags)
            {
                tcpflags = conn->tcpstateflags;
                if (tcpflags == 0)
                {
                    conn = 0;
                    conndly = 10;
                }
            }

        }
        break;
    }
}

bool tcpip_send(char *str)
{
    if (!tcpip_connected()) return false;
    while (*str)
    {
        *(tcp_in_ptr++) = *(str++);
        if (tcp_in_ptr >= &tcp_ring_buf[tcp_ring_buf_sz]) tcp_in_ptr = tcp_ring_buf;
    }
    return true;
}

void tcp_conn_cb_register(tcp_conn_cb_func f)
{
    tcp_conn_cb = f;
}

void tcp_rx_cb_register(tcp_rx_cb_func f)
{
    tcp_rx_cb = f;
}

void uip_appcall(void)
{
    if (uip_connected())
    {
        if (tcp_conn_cb) tcp_conn_cb(true);
    }

    if (uip_closed())
    {
        if (tcp_conn_cb) tcp_conn_cb(false);
    }

    if (uip_newdata())
    {
        if (uip_len > 0)
        {
            if (tcp_rx_cb) tcp_rx_cb(uip_appdata, uip_len);
        }
    }

    if (!lnk) uip_abort();

    if (uip_poll())
    {
        char *p = uip_appdata;
        uip_len = 0;
        while (tcp_out_ptr != tcp_in_ptr)
        {
            *(p++) = *(tcp_out_ptr++);
            uip_len++;
            if (tcp_out_ptr >= &tcp_ring_buf[tcp_ring_buf_sz]) tcp_out_ptr = tcp_ring_buf;
        }
        uip_send(uip_appdata, uip_len);
    }
}

