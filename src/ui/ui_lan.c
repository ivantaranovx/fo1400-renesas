
#include "../lcd.h"
#include "../misc.h"
#include "../eth/tcpip.h"

#include <string.h>

int ip_edit(char key);

extern IPSET ipset;

typedef struct
{
    char *title;
    void *ptr;
    uint8_t type;
}
IP_MENU_ITEM;

static const char str1[] = "Настройки сети";
static const char str2[] = "Нет модуля LAN";

static const char *strflg[] = {
    "   ", "LNK", "SRV",
};

#define MENU_SZ   6

static const IP_MENU_ITEM menu_items[MENU_SZ] = {
    {"MAC адрес", ipset.mac, 0},
    {"IP адрес", ipset.ip, 1},
    {"маска подсети", ipset.mask, 1},
    {"шлюз", ipset.gw, 1},
    {"IP адрес сервера", ipset.srv, 1},
    {"ID клиента", &ipset.id, 2},
};

static int idx = 0;
static int edit = 0;
static uint8_t buf[6];

static struct
{
    bool lnk;
    bool srv;
}
FLG;

int ui_ipaddr(char key)
{
    uint8_t r;

    if (!tcpip_ready())
    {
        lcd_clear();
        lcd_print_rom(STR2_ADDR + 1, str2);
        if (key == '*') return 1;
        return 0;
    }

    if (FLG.lnk ^ tcpip_link())
    {
        FLG.lnk ^= 1;
        key = 0xFF;
    }

    if (FLG.srv ^ tcpip_connected())
    {
        FLG.srv ^= 1;
        key = 0xFF;
    }

    if (key == 0) return 0;

    if (edit == 0)
    {
        if (key == '*') return 1;
        if (key == 'A') idx++;
        if (key == 'B') idx--;
        if (idx >= MENU_SZ) idx = 0;
        if (idx < 0) idx = MENU_SZ - 1;
        if (key == '#')
        {
            if (menu_items[idx].type == 0)
                memcpy(buf, menu_items[idx].ptr, 6);
            if (menu_items[idx].type == 1)
                memcpy(buf, menu_items[idx].ptr, 4);
            if (menu_items[idx].type == 2)
                ui_input_int(STR3_ADDR + 2, *(uint16_t*) menu_items[idx].ptr, 0);
            edit = 1;
            key = 0;
        }
    }

    if (edit > 0)
    {
        switch (ip_edit(key))
        {
        case 0:
            return 0;
        case 2:
            if (menu_items[idx].type == 0)
                memcpy(menu_items[idx].ptr, buf, 6);
            if (menu_items[idx].type == 1)
                memcpy(menu_items[idx].ptr, buf, 4);
            if (menu_items[idx].type == 2)
                ui_input_int_get((uint16_t*) menu_items[idx].ptr);
            tcpip_reconf();
        case 1:
            edit = 0;
            break;
        }
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, str1);
    lcd_print_rom(STR2_ADDR + 1, menu_items[idx].title);
    print_ex(STR3_ADDR + 2, menu_items[idx].type, menu_items[idx].ptr);
    lcd_print_rom(STR4_ADDR + 0, FLG.lnk ? strflg[1] : strflg[0]);
    lcd_print_rom(STR4_ADDR + 4, FLG.srv ? strflg[2] : strflg[0]);
    return 0;
}

int ip_edit(char key)
{
    uint8_t i, p;
    static uint8_t d = 0;
    int t;

    if (key == '*') return 1;
    if (key == '#') return 2;
    if (key == 'C') edit++;
    if (key == 'D') edit--;

    if (menu_items[idx].type == 0)
    {
        if (edit > 6) edit = 1;
        if (edit < 1) edit = 6;
        i = (edit - 1) / 2;
        d = (edit - 1) % 2;

        if ((key >= '0') && (key <= '9'))
        {
            if (d == 0) buf[i + 3] = (buf[i + 3] & 0x0F) + ((key - 0x30) << 4);
            if (d == 1) buf[i + 3] = (buf[i + 3] & 0xF0) + (key - 0x30);

            edit++;
            if (edit > 6) edit = 1;
            i = (edit - 1) / 2;
            d = (edit - 1) % 2;
        }

        if (key == 'A')
        {
            if (d == 0) buf[i + 3] += 0x10;
            if (d == 1) buf[i + 3] = (buf[i + 3] & 0xF0) + ((buf[i + 3] + 1) & 0x0F);
        }

        if (key == 'B')
        {
            if (d == 0) buf[i + 3] -= 0x10;
            if (d == 1) buf[i + 3] = (buf[i + 3] & 0xF0) + ((buf[i + 3] - 1) & 0x0F);
        }

        p = 11 + (i * 3) + d;
    }

    if (menu_items[idx].type == 1)
    {
        if (edit > 4)
        {
            edit = 1;
            d = 0;
        }
        if (edit < 1)
        {
            edit = 4;
            d = 0;
        }
        i = edit - 1;

        if ((key >= '0') && (key <= '9'))
        {
            if (d > 2) d = 0;
            t = buf[i];
            if (d == 0) t = 0;
            if (d == 1) t *= 10;
            if (d == 2) t *= 10;
            t += key - 0x30;
            if (t <= 255)
            {
                buf[i] = t;
                d++;
            }
        }

        if (key == 'A')
        {
            if (d > 0)
            {
                buf[i] /= 10;
                d--;
            }
        }

        p = 4 + (i * 4);
    }

    if (menu_items[idx].type == 2)
    {
        ui_input_int_process(key);
        return 0;
    }

    print_ex(STR3_ADDR + 2, menu_items[idx].type, buf);
    lcd_set_cursor(STR3_ADDR + p, 1);
    return 0;
}
