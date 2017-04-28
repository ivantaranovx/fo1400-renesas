
#ifndef M30845_H
#define M30845_H

typedef enum {
    TMR_SYS = 0,
    TMR_ENGINE,
    TMR_LUB,
    TMR_GUARD,
    TMR_UI,

    TMR_TCP,
    TMR_EEPROM,

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

typedef enum {
    TMR_SCALE_INJECT,
    TMR_SCALE_TPWM,

    TMR_SCALE_MAX
}
TMR_SCALE_NUM;

#define TMR_MAX_VALUE     0xFFFF

#include <stdint.h>
#include <stdbool.h>

// LCD 4bit mode - data bus P11[3:0]

#define WH_PORT p11

#define WH_PORT_IN()    pd11 &= 0xF0
#define WH_PORT_OUT()   pd11 |= 0x0F

#define WH_RS   p8_6
#define WH_RW   p8_7
#define WH_OE   p11_4

/*
 * watchdog
 */
void wdt_reset(void);
bool is_wdt_rst(void);

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

/*
 * stop timer
 */
void kill_timer(TMR_NUM num);

void clr_scale_timer(TMR_SCALE_NUM num);
uint16_t get_scale_timer(TMR_SCALE_NUM num);

/*
 * return AD value for selected channel (0-7)
 */
double adc_get_u(uint8_t ch);

/*
 * set DA value for selected channel (0-1)
 */
void dac_set(uint8_t ch, uint8_t val);

/*
 * return keys scan code
 * each bit is one key
 */
uint16_t get_key_code(void);

/*
 * return key code
 */
char get_key(void);

/*
 * system bus
 */
void bus_enable(unsigned e);
void bus_write(uint8_t addr, uint16_t data);
uint16_t bus_read(uint8_t addr);

/*
 * I2C functions
 */

/*
 * void i2c_io(uint8_t addr, uint8_t *buf, uint16_t len, uint8_t flags);
 * if rw = 0 then wite mode:
 * start/restart, write addr and write len bytes from buf to slave, stop;
 * if rw = 1 then read mode:
 * start/restart, write addr and read len bytes from slave to buf, stop;
 * flags:
 * 1 - don`t stop; if fail then stop;
 */
void i2c_io(uint8_t addr, uint8_t *buf, uint16_t len, uint8_t flags);

/*
 * int i2c_io_status(void);
 * -1 if io in progress;
 * 0 if done and fail;
 * 1 if done and success;
 */
int i2c_io_status(void);

/*
 * UART functions
 */
typedef void (*uart_rx_cb_func)(uint8_t b);
void uart_rx_cb_register(uart_rx_cb_func f);
void uart_print(char *buf);
void uart_printf(const char *fmt, ...);

/*
 * LCD functions
 */
void lcd_write(uint8_t v);
unsigned lcd_busy(void);
void lcd_set_rs(unsigned v);

/*
 * SPI, ENC finctions
 */
void spi_select(unsigned val);
uint8_t spi_io(uint8_t b);
unsigned spi_enc_int(void);
void spi_enc_rst(unsigned val);

#endif /* M30845_H */

