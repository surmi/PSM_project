/*
 * rtc.h
 *
 *  Created on: 26-04-2017
 *      Author: psm8
 */

#ifndef RTC_H_
#define RTC_H_
#include <avr/io.h>

void rtc_init(void);
void rtc_control_reg(uint8_t out, uint8_t sqwe, uint8_t rs1, uint8_t rs0);
void rtc_set_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t h12_h24,
uint8_t PM_AM);
void rtc_set_date(uint8_t day, uint8_t date, uint8_t month, uint8_t year);
void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s);
void rtc_get_date(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year);

#endif /* RTC_H_ */
