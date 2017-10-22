
#include "../lcd.h"
#include "../hal.h"
#include "../eeprom.h"
#include "../misc.h"
#include "../workset.h"
#include "../eeprom.h"
#include "../eth/tcpip.h"
#include "../json.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

static const char str1[] = "Выбор пользователя";
static const char str2[] = "Нет связи!";
static const char str3[] = "Тариф: ";
static const char str4[] = "C-сброс D-выбор";

static bool ret = false;
static bool srv = false;

int ui_users(char key)
{
    char buf[10];

    if (ret)
    {
        ret = false;
        return 2;
    }

    if (tcpip_connected() ^ srv)
    {
        srv = !srv;
        key = 'R';
    }

    if (key == 0) return 0;

    if (key == 'R')
    {
        lcd_clear();
        lcd_print_rom(STR1_ADDR, str1);
        if (srv)
        {
            lcd_print_rom(STR3_ADDR, str3);
            lcd_print_rom(STR4_ADDR, str4);
            tcpip_send("{\"type\":\"users\",\"cmd\":\"current\"}");
        }
        else
        {
            lcd_print_rom(STR2_ADDR, str2);
        }
    }

    if (key == '*') return 1; // cancel

    if (!srv) return 0;

    if (key == 'A') tcpip_send("{\"type\":\"users\",\"cmd\":\"next\"}");
    if (key == 'B') tcpip_send("{\"type\":\"users\",\"cmd\":\"prev\"}");
    if (key == 'C') tcpip_send("{\"type\":\"users\",\"cmd\":\"clear\"}");
    if (key == 'D') tcpip_send("{\"type\":\"users\",\"cmd\":\"select\"}");

    if (isdigit((unsigned char) key))
    {
        tcpip_send("{\"type\":\"users\",\"tarif\":");
        snprintf(buf, sizeof (buf) - 1, "%hu", key - 0x30);
        tcpip_send(buf);
        tcpip_send("}");
    }

    return 0;
}

void ui_users_cb(char *data)
{
    char buf[50];
    if (json_get_param("user", data, buf, sizeof(buf)) > 0)
    {
        lcd_clr_str(STR2_ADDR);
        lcd_print(STR2_ADDR, buf);
    }
    if (json_get_param("tarif", data, buf, sizeof(buf)) > 0)
    {
        lcd_print(STR3_ADDR + 7, buf);
    }
    if (json_get_param("select", data, buf, sizeof(buf)) > 0)
    {
        ret = true;
    }
}
