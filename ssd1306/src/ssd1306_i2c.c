/**
 * original author:  Tilen Majerle<tilen@majerle.eu>
 * modification for STM32f10x HAL: Alexander Lutsai<s.lyra@ya.ru>
 * modification for STM32f10x libopencm3: Nicetrym8<atlantik03053@gmail.com>
 
    Copyright (C) 2015 Tilen Majerle
	Copyright (C) 2016 Alexander Lutsai
	Copyrigth (C) 2023 Nicetrym8


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "ssd1306_i2c.h"


static uint32_t ssd1306_i2c_timeout;

#if defined(STM32F1) || defined(STM32F2)
void ssd1306_i2c_reset(uint32_t i2cx){
	I2C_CR1(i2cx) |= I2C_CR1_SWRST;
	I2C_CR1(i2cx) &= ~I2C_CR1_SWRST;
}

void ssd1306_i2c_init(uint32_t i2cx) {

	i2c_peripheral_disable(i2cx);
	i2c_set_clock_frequency(i2cx, 36);//ABP1=36MHz
	i2c_set_fast_mode(i2cx); //400kHz
	i2c_set_ccr(i2cx, 0x1e); //fclock
	i2c_set_trise(i2cx, 0x0a); //optimal trise
	i2c_enable_ack(i2cx);
	i2c_set_own_7bit_slave_address(i2cx, 0x32);
	i2c_peripheral_enable(i2cx);	
}

void ssd1306_i2c_write_multi(uint32_t i2cx, uint8_t address, uint8_t reg, uint8_t *data, uint16_t count) {
	uint8_t i;
	ssd1306_i2c_start(i2cx, address, I2C_WRITE);
	ssd1306_i2c_write_data(i2cx, reg);
	for (i = 0; i < count; i++) {

		ssd1306_i2c_write_data(i2cx, data[i]);
	}
	ssd1306_i2c_stop(i2cx);
}


uint32_t reg32 __attribute__((unused));

uint8_t ssd1306_i2c_start(uint32_t i2cx, uint8_t address, uint8_t direction) {
	ssd1306_i2c_timeout = SSD1306_I2C_TIMEOUT;
	i2c_send_start(i2cx);
	while ((I2C_SR1(i2cx) & I2C_SR1_SB) == 0) {
		if (--ssd1306_i2c_timeout == 0x00) {
			return 1;
		}
	}
	ssd1306_i2c_timeout = SSD1306_I2C_TIMEOUT;
	i2c_send_7bit_address(i2cx, address, direction);
	while ((I2C_SR1(i2cx) & I2C_SR1_ADDR) == 0) {
		if (--ssd1306_i2c_timeout == 0x00) {
			return 1;
		}
	}
	/*clear ADDR flag in SR1*/
	reg32 = I2C_SR2(i2cx);
	return 0;
}

void ssd1306_i2c_write_data(uint32_t i2cx, uint8_t data) {
	ssd1306_i2c_timeout = SSD1306_I2C_TIMEOUT;
	while ((I2C_SR1(i2cx) & I2C_SR1_TxE) == 0) {
		if (--ssd1306_i2c_timeout == 0x00) {
			return;
		}
	}
	i2c_send_data(i2cx, data);
}

void ssd1306_i2c_write(uint32_t i2cx, uint8_t address, uint8_t reg, uint8_t data) {
	ssd1306_i2c_start(i2cx, address, I2C_WRITE);
	ssd1306_i2c_write_data(i2cx, reg);
	ssd1306_i2c_write_data(i2cx, data);
	ssd1306_i2c_stop(i2cx);
}

uint8_t ssd1306_i2c_stop(uint32_t i2cx) {
	ssd1306_i2c_timeout = SSD1306_I2C_TIMEOUT;
	while ((((I2C_SR1(i2cx) & I2C_SR1_TxE) == 0)) || ((I2C_SR1(i2cx) & I2C_SR1_BTF) == 0)) {
		if (--ssd1306_i2c_timeout == 0x00) {
			return 1;
		}
	}
	i2c_send_stop(i2cx);
	return 0;
}
#endif


