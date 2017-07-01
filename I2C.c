/*
 * I2C.c
 *
 *  Created on: 26-04-2017
 *      Author: psm8
 */
#include "I2C.h"
#include <stdio.h>

void I2C_init(void){
	TWBR = 4;
	TWSR |= 2;
}
void I2C_start(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}
void I2C_stop(void){
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);

}
void I2C_write(uint8_t byte){
	TWDR = byte;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}
uint8_t I2C_read(uint8_t nack){
	TWCR = (1<<TWINT) | (1<<TWEN);
	if(!nack) TWCR |= 1<< TWEA;
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}
