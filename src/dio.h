
#ifndef DIO_H
#define	DIO_H

void dio_init(void);
void dio_task(void);

unsigned dio_in(uint8_t num);
void dio_out(uint8_t num, unsigned v);
void dio_flush(void);

#endif	/* DIO_H */

