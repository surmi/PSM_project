/*
 * I2C.h
 *
 *  Created on: 26-04-2017
 *      Author: psm8
 */

#ifndef I2C_H_
#define I2C_H_
#include <avr/io.h>

void I2C_init(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_write(uint8_t byte);
uint8_t I2C_read(uint8_t nack);


#endif /* I2C_H_ */
