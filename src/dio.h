
#ifndef DIO_H
#define DIO_H

#include <stdint.h>

#define INPUTS_MAX      64
#define OUTPUTS_MAX     64

void dio_init(void);
void dio_task(void);

unsigned dio_in(uint8_t num);
void dio_out(uint8_t num, unsigned v);
unsigned dio_out_state(uint8_t num);
void dio_out_reset(void);

void dio_flush(void);

#endif /* DIO_H */

