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
#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H 

#if defined(STM32F1) || defined(STM32F2)
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#endif
#ifndef SSD1306_NOSTDLIB
#include <stdlib.h>
#endif



#ifndef SSD1306_I2C_TIMEOUT
#define SSD1306_I2C_TIMEOUT					1000
#endif



/**
 * @brief  Sends soft reset sequence to selected I2C periphery
 * @param i2cx: I2C used
 */
void ssd1306_i2c_reset(uint32_t i2cx);

/**
 * @brief I2C init function. Can be edited for your needs.
 * @param i2cx: I2C used
 */
void ssd1306_i2c_init(uint32_t i2cx);

/**
 * @brief  Writes single byte to slave
 * @param  *i2cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  data: data to be written
 * @retval None
 */
void ssd1306_i2c_write(uint32_t i2cx, uint8_t address, uint8_t reg, uint8_t data);

/**
 * @brief  Writes multi bytes to slave
 * @param  *i2cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  *data: pointer to data array to write it to slave
 * @param  count: how many bytes will be written
 * @retval None
 */
void ssd1306_i2c_write_multi(uint32_t i2cx, uint8_t address, uint8_t reg, uint8_t *data, uint16_t count);

/**
 * @brief  I2C Start condition
 * @param  *i2cx: I2C used
 * @param  address: slave address
 * @param  direction: master to slave or slave to master
 * @param  ack: ack enabled or disabled
 * @retval Start condition status
 * @note   For private use
 */
uint8_t ssd1306_i2c_start(uint32_t i2cx, uint8_t address, uint8_t direction);

/**
 * @brief  Stop condition on I2C
 * @param  *i2cx: I2C used
 * @retval Stop condition status
 * @note   For private use
 */
uint8_t ssd1306_i2c_stop(uint32_t i2cx);

/**
 * @brief  Writes to slave
 * @param  *i2cx: I2C used
 * @param  data: data to be sent
 * @retval None
 * @note   For private use
 */
void ssd1306_i2c_write_data(uint32_t i2cx, uint8_t data);

#endif

