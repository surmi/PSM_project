/*
 * rtc.c
 *
 *  Created on: 26-04-2017
 *      Author: psm8
 */
#include "rtc.h"
#include "I2C.h"
#include <stdio.h>
#define DS1307 0b11010000
#define	rtc_sec 0
#define	rtc_min 1
#define	rtc_h 2
#define	rtc_DAY 3
#define	rtc_date 4
#define	rtc_month 5
#define	rtc_year 6
#define BCD_TO_INT(x) ((x)&0x0F)+10*(((x)&0xF0)>>4)
#define INT_TO_BCD(x) ((x)%10) | (((x)/10)<<4)


void rtc_init(void){
	I2C_start();
	I2C_write(DS1307);
	I2C_write(rtc_sec);
	I2C_write(0);//sec
	I2C_write(0);//min
	I2C_write(0);//hour
	I2C_write(0);//DAY
	I2C_write(0);//date
	I2C_write(0);//month
	I2C_write(0);//year
	I2C_stop();
}
void rtc_control_reg(uint8_t out, uint8_t sqwe, uint8_t rs1, uint8_t rs0){
	uint8_t ctrl = (rs0<<0) | (rs1<<1) | (sqwe<<4) | (out<<7);
	I2C_start();
	I2C_write(DS1307);
	I2C_write(0x07);
	I2C_write(ctrl);
	I2C_stop();
}
void rtc_set_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t h12_h24,
uint8_t PM_AM){
	I2C_start();
	I2C_write(DS1307);
	I2C_write(rtc_sec);
	I2C_write(INT_TO_BCD(seconds));//sec
	I2C_write(INT_TO_BCD(minutes));//min
	I2C_write(INT_TO_BCD(hours) | (h12_h24<<6) | (PM_AM<<5));//hour
	I2C_stop();
}
void rtc_set_date(uint8_t day, uint8_t date, uint8_t month, uint8_t year){
	I2C_start();
	I2C_write(DS1307);
	I2C_write(rtc_DAY);
	I2C_write(INT_TO_BCD(day));//day
	I2C_write(INT_TO_BCD(date));//date
	I2C_write(INT_TO_BCD(month));//month
	I2C_write(INT_TO_BCD(year));//year
	I2C_stop();
}
void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s){
	I2C_start();
	I2C_write(DS1307);
	I2C_write(rtc_sec);
	I2C_start();
	I2C_write(DS1307 | 0x01);
	*s = I2C_read(0);
	*s = BCD_TO_INT(*s);
	*m = I2C_read(0);
	*m = BCD_TO_INT(*m);
	*h = I2C_read(1);
	*h = *h & ~(1 << 6);
	*h = BCD_TO_INT(*h);
	I2C_stop();
}
void rtc_get_date(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year){
	I2C_start();
	I2C_write(DS1307);
	I2C_write(rtc_DAY);
	I2C_start();
	I2C_write(DS1307 | 0x01);
	*day = I2C_read(0);
	*day = BCD_TO_INT(*day);
	*date = I2C_read(0);
	*date = BCD_TO_INT(*date);
	*month= I2C_read(0);
	*month = BCD_TO_INT(*month);
	*year = I2C_read(1);
	*year = BCD_TO_INT(*year);
	I2C_stop();
}
