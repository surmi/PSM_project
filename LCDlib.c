/*
 * LCDlib.c
 *
 *  Created on: 15-03-2017
 *      Author: psm8
 */
#include "LCDlib.h"

void lcdinit(void){
	_delay_ms(45);
	LCDPORT &= ~(1 << LCD_RS );

	lcd_write4(3);
	_delay_ms(5);
	lcd_write4(3);
	_delay_us(100);
	lcd_write4(3);
	_delay_us(100);
	lcd_write4(2);
	_delay_us(100);
	lcd_write8((1<<5) | 1<<3 | 1<<2);// 2 linie i matryca 5x10
	//lcd_write8(1<<3);//display off
	//lcd_write8(1);//display clear
	lcd_clear();
	//lcd_write_instr(0x0F);//blinking 
	lcd_write_instr(12);
}

void lcd_write4(uint8_t data){
	LCDPORT |= (1 << LCD_E);
	LCDPORT = (LCDPORT & 0x0F) | ((data << 4));
	_delay_us(40);
	LCDPORT &= ~(1 << LCD_E);
	_delay_us(40);
}

void lcd_write8(uint8_t data){
	lcd_write4(data >> 4);
	_delay_us(100);
	lcd_write4(data);
	_delay_us(100);
}

void lcd_clear(void){
	LCDPORT &= ~(1 << LCD_RS );
	lcd_write8(1);
	_delay_ms(2);
}

void lcd_write_instr(uint8_t data){
	LCDPORT &= ~(1 << LCD_RS );
	lcd_write8(data);
	_delay_ms(2);
	LCDPORT |= (1 << LCD_RS );

}
void lcd_set_xy(uint8_t r, uint8_t k){
	lcd_write_instr((1<<7)|(k+r*40));
}
void lcd_write_text_xy(uint8_t r, uint8_t k, char *text){
	lcd_set_xy(r,k);
	while(*text){
		lcd_write8(*text);
		text++;
	}
}


