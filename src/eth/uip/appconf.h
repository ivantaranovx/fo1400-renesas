
#ifndef APPCONF_H
#define APPCONF_H

#include "psock.h"

typedef struct app_state {
}
uip_tcp_appstate_t;

void uip_appcall(void);
#ifndef UIP_APPCALL
#define UIP_APPCALL uip_appcall
#endif

#endif /* APPCONF_H */

