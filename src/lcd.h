
#ifndef LCD_H
#define	LCD_H

#include <stdint.h>

#define STR_MAX_LENGTH  20

#define STR1_ADDR   0x00
#define STR2_ADDR   0x40
#define STR3_ADDR   0x14
#define STR4_ADDR   0x54

/*
 * return 1 if success
 * return 0 if error
 */
unsigned lcd_init(void);

/*
 * clear display
 */
unsigned lcd_clear(void);
unsigned lcd_clr_str(int pos);

unsigned lcd_set_cursor(int pos, unsigned show);
unsigned lcd_put_char(char c);

/*
 * print buffer ()
 */

unsigned lcd_print_rom(int pos, const char *buf);
unsigned lcd_printd(int pos, char *buf, uint8_t dot);
#define lcd_print(pos, buf) lcd_printd(pos, buf, 0)

void lcd_printf(int pos, const char *fmt, ...);

#endif	/* LCD_H */

