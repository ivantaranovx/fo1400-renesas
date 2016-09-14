
#ifndef E24LC512_H
#define	E24LC512_H

#include <stdint.h>

void chipSelect(uint8_t addr);

uint16_t readArray(uint16_t addr, uint8_t *buf, uint16_t len);
uint8_t writeArray(uint16_t addr, uint8_t *buf, uint8_t len);

#endif	/* E24LC512_H */

