
#include "json.h"
#include "hal.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

int json_get_param(char *key, char *in, char *value, int val_sz)
{
    char buf[20];
    char *p;
    int len;
    int t = 0;
    int i;

    len = snprintf(buf, sizeof (buf) - 1, "\"%s\"", key);
    p = strstr(in, buf);
    if (p == 0)return 0;
    for (p += len;; p++)
    {
        if (*p == 0) return 0;
        if (*p == ',') return 0;
        if (*p == ':') break;
    }
    val_sz--;
    for (i = 0;; p++)
    {
        if (*p == 0) return 0;
        if ((t == 2) && (*p == '"')) break;
        if ((t == 1) && !isdigit((unsigned char) *p)) break;
        if ((t == 0) && isdigit((unsigned char) *p)) t = 1;
        if (t > 0) value[i++] = *p;
        if (i == val_sz) break;
        if ((t == 0) && (*p == '"')) t = 2;
    }
    value[i] = 0;
    return i;
}
