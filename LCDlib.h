#ifndef LCDLIB_H_
#define LCDLIB_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define LCDPORT PORTA
#define LCD_RS PINA2
#define LCD_E PINA3

static int LCD_putchar(char c, FILE *stream);


void lcdinit(void);
void lcd_write_instr(uint8_t data);
void lcd_clear(void);
void lcd_write8(uint8_t data);
void lcd_write4(uint8_t data);
void lcd_set_xy(uint8_t r, uint8_t k);
void lcd_write_text_xy(uint8_t r, uint8_t k, char *text);

#endif /* LCDLIB_H_ */