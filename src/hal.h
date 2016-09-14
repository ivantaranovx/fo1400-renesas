
#ifndef M30845_H
#define	M30845_H

typedef enum
{
    TMR_SYS = 0,
    TMR_ENGINE,
    TMR_LUB,

    TMR_1,
    TMR_2,
    TMR_3,
    TMR_4,
    TMR_5,
    TMR_6,
    TMR_7,
    TMR_8,
    TMR_9,
    TMR_10,
    TMR_11,
    TMR_12,
    TMR_13,
    TMR_14,
    TMR_15,
    TMR_16,
    TMR_17,
    TMR_18,
    TMR_19,

    TMR_MAX
}
TMR_NUM;

typedef enum
{
    TMR_SCALE_INJECT,
    TMR_SCALE_TPWM,

    TMR_SCALE_MAX
}
TMR_SCALE_NUM;

#define TMR_MAX_VALUE     0xFFFF

#include <stdint.h>

// LCD 4bit mode - data bus P11[3:0]

#define WH_PORT p11

#define WH_PORT_IN()    pd11 &= 0xF0
#define WH_PORT_OUT()   pd11 |= 0x0F

#define WH_RS   p8_6
#define WH_RW   p8_7
#define WH_OE   p11_4

/*
 * return board string Id
 */
char *brd_id(void);

/*
 * Init ports, modules, etc.
 */
void brd_init(void);

/*
 * for very small delays (block)
 * max 2048us
 */
void _delay_us(uint16_t delay);

/*
 * for general purpose delays (nonblock)
 * num - timer number
 * delay - delay in ms (65534 max)
 */
void set_timer(TMR_NUM num, uint16_t delay);

/*
 * num - timer number
 * return 1 if timer running
 * return 0 if timer stopped (once)
 * return -1 if timer stopped
 */
int8_t get_timer(TMR_NUM num);

void clr_scale_timer(TMR_SCALE_NUM num);
uint16_t get_scale_timer(TMR_SCALE_NUM num);

/*
 * return AD value for selected channel (0-7)
 */
double _get_adc_u(uint8_t ch);

/*
 * set DA value for selected channel (0-1)
 */
void _set_dac(uint8_t ch, uint8_t val);

/*
 * return keys scan code
 * each bit is one key
 */
uint16_t getKeyCode(void);

/*
 * return key code
 */
char get_key(void);

/*
 * system bus
 */
void _bus_enable(unsigned e);
void _bus_write(uint8_t addr, uint16_t data);
uint16_t _bus_read(uint8_t addr);

/*
 * I2C functions
 */
void i2c_start(void);
void i2c_rstart(void);
void i2c_stop(void);

unsigned i2c_send(uint8_t b);
uint8_t i2c_recv(unsigned ack);

/*
 * UART functions
 */
void uart_send_str(char *buf);

/*
 * LCD functions
 */
void _lcd_write(uint8_t v);
unsigned _lcd_busy(void);
void _lcd_set_rs(unsigned v);

#endif /* M30845_H */

