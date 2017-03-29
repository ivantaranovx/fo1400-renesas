
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
unsigned lcd_clr_str(uint8_t pos);

unsigned lcd_set_cursor(uint8_t pos, unsigned show);
unsigned lcd_put_char(char c);

/*
 * print buffer ()
 */

unsigned lcd_print_rom(uint8_t pos, const char *buf);
unsigned lcd_printd(uint8_t pos, char *buf, uint8_t dot);
#define lcd_print(pos, buf) lcd_printd(pos, buf, 0)

void lcd_printf(unsigned char pos, const char *fmt, ...);

#endif	/* LCD_H */

