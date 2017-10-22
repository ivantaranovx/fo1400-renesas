
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
static int conndly = 0;
static bool lnk = false;
static tcp_conn_cb_func tcp_conn_cb = 0;
static tcp_rx_cb_func tcp_rx_cb = 0;

#define tcp_buf_sz 1024
static uint8_t tcp_buf[tcp_buf_sz];
static uint16_t tcp_idx = 0;

IPSET ipset;

char *uip_flags_str(uint8_t flags, char *buf, size_t buf_sz)
{
    memset(buf, 0, buf_sz);
    if (flags & UIP_ACKDATA) strlcat(buf, " UIP_ACKDATA", buf_sz);
    if (flags & UIP_NEWDATA) strlcat(buf, " UIP_NEWDATA", buf_sz);
    if (flags & UIP_REXMIT) strlcat(buf, " UIP_REXMIT", buf_sz);
    if (flags & UIP_POLL) strlcat(buf, " UIP_POLL", buf_sz);
    if (flags & UIP_CLOSE) strlcat(buf, " UIP_CLOSE", buf_sz);
    if (flags & UIP_ABORT) strlcat(buf, " UIP_ABORT", buf_sz);
    if (flags & UIP_CONNECTED) strlcat(buf, " UIP_CONNECTED", buf_sz);
    if (flags & UIP_TIMEDOUT) strlcat(buf, " UIP_TIMEDOUT", buf_sz);
    return buf;
}

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

        uip_len = 0;
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
            set_timer(TMR_UIP, 200);

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

        }
        break;
    }
}

bool tcpip_send(char *str)
{
    if (!tcpip_connected()) return false;
    size_t len = strlen(str);
    memcpy(&tcp_buf[tcp_idx], str, len);
    tcp_idx += len;
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
    char buf[100];

    if (uip_connected())
    {
        if (tcp_conn_cb) tcp_conn_cb(true);
    }

    if (uip_closed())
    {
        if (tcp_conn_cb) tcp_conn_cb(false);
    }

    if (uip_aborted() | uip_timedout() | uip_closed())
    {
        conn = 0;
        conndly = 50;
    }

    if (uip_newdata())
    {
        if ((uip_len > 0) && tcp_rx_cb) tcp_rx_cb(uip_appdata, uip_len);
    }

    if (uip_poll())
    {
        if (tcp_idx > 0) uip_send(tcp_buf, tcp_idx);
    }
    else
    {
        uart_printf("uip_appcall: %s\r\n", uip_flags_str(uip_flags, buf, sizeof (buf)));
    }

    if (uip_rexmit())
    {
        if (tcp_idx > 0) uip_send(tcp_buf, tcp_idx);
    }

    if (uip_acked())
    {
        tcp_idx = 0;
    }

    if ((!lnk) && conn) uip_abort();
}

