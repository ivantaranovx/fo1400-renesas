
#ifndef EEPROM_ADDR_H
#define EEPROM_ADDR_H

#define WORKSET_ADDR    0x0000  // length: (16 + 136) * 400 = 60800/0xED80
// ED80-F2FF
#define USERS_ADDR      0xF300  // length: 16 * 200 = 3200/0x0C80
// FF80-FFDF
#define IPSET_ADDR      0xFFE0  // length: 22

#endif /* EEPROM_ADDR_H */

