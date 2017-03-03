
#ifndef EEPROM_H
#define	EEPROM_H

#include <stdint.h>

void eeprom_cs(uint8_t bank, uint16_t addr);

void eeprom_read(uint8_t *buf, uint16_t len);
void eeprom_write(uint8_t *buf, uint16_t len);

int eeprom_status(void);
int eeprom_status_wait(void);

#endif	/* EEPROM_H */
