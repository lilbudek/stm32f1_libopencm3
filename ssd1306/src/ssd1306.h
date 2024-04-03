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


#ifndef SSD1306_H
#define SSD1306_H 

 /* C++ detection */
#ifdef __cplusplus
extern C{
#endif

  /**
   * This SSD1306 LCD uses I2C for communication
   *
   * Library features functions for drawing lines, rectangles and circles.
   *
   * It also allows you to draw texts and characters using appropriate functions provided in library.
   *
   * Default pinout
   *
  SSD1306    |STM32F10x    |DESCRIPTION

  VCC        |3.3V         |
  GND        |GND          |
  SCL        |PB6          |Serial clock line
  SDA        |PB7          |Serial data line
   */

  #include "ssd1306_i2c.h"
  #include "fonts.h"
   //#include "stm32f4_delay.h"
  #include <stdint.h>


   /* I2C settings */
   #ifndef SSD1306_I2C
   #define SSD1306_I2C              I2C2
   #endif

   /* I2C address */
   #ifndef SSD1306_I2C_ADDR
   #define SSD1306_I2C_ADDR         60
   #endif

   /* SSD1306 settings */
   /* SSD1306 width in pixels */
   #ifndef SSD1306_WIDTH
   #define SSD1306_WIDTH            128
   #endif
   /* SSD1306 LCD height in pixels */
   #ifndef SSD1306_HEIGHT
   #define SSD1306_HEIGHT           64
   #endif

   /**
    * @brief  SSD1306 color enumeration
    */
   typedef enum {
     SSD1306_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
     SSD1306_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
   } SSD1306_COLOR_t;




/**
 * @brief Checks if device is connected via sending start and stop sequences
 * @param  None
 * @retval Connection status:
 *           - 0: LCD is not connected or error occured
 *           - > 0: LCD is OK and ready to use
*/
uint8_t ssd1306_is_device_connected();

/**
 * @brief  Initializes SSD1306 LCD
 * @param  None
 * @retval Initialization status:
 *           - 0: LCD was not detected on I2C port
 *           - > 0: LCD initialized OK and ready to use
 */
uint8_t ssd1306_init(void);
/**
 * @brief Turns display on
 * @param  None
 * @retval None
 */
void ssd1306_on(void);
/**
 * @brief Turns display off
 * @param  None
 * @retval None
 */
void ssd1306_off(void);
/**
 * @brief Resets I2C and reinits display
 * @param  None
 * @retval None
 */
void ssd1306_reset(void);

/**
 * @brief  Drawing bitmap image on display
 * 
 * @param  x: X location. This parameter can be a value between 0 and SSD1306_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SSD1306_HEIGHT - 1
 * @param bitmap: Bitmap data array 
 * @param w: Bitmap width. This parameter can be a value between 0 and SSD1306_WIDTH
 * @param h: Bitmap height. This parameter can be a value between 0 and SSD1306_HEIGHT
 * @param color: Color to be used for screen fill. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval
 */
void ssd1306_draw_bitmap(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t w, uint8_t h, SSD1306_COLOR_t color);

/**
 * @brief  Updates buffer from internal RAM to LCD
 * @note   This function must be called each time you do some changes to LCD, to update buffer from RAM to LCD
 * @param  None
 * @retval None
 */
void ssd1306_update_screen(void);

/**
 * @brief  Toggles pixels invertion inside internal RAM
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  None
 * @retval None
 */
void ssd1306_toggle_invert(void);

/**
 * @brief  Fills entire LCD with desired color
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  Color: Color to be used for screen fill. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_fill(SSD1306_COLOR_t Color);

/**
 * @brief  Draws pixel at desired location
 * @note   @ref ssd1306_update_screen() must called after that in order to see updated LCD screen
 * @param  x: X location. This parameter can be a value between 0 and SSD1306_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SSD1306_HEIGHT - 1
 * @param  color: Color to be used for screen fill. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_draw_pixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);

/**
 * @brief  Sets cursor pointer to desired location for strings
 * @param  x: X location. This parameter can be a value between 0 and SSD1306_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SSD1306_HEIGHT - 1
 * @retval None
 */
void ssd1306_goto_xy(uint16_t x, uint16_t y);

/**
 * @brief  Puts character to internal RAM
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  ch: Character to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval Character written
 */
char ssd1306_putc(char ch, FontDef_t *Font, SSD1306_COLOR_t color);

/**
 * @brief  Puts string to internal RAM
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  *str: String to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval Zero on success or character value when function failed
 */
char ssd1306_puts(char *str, FontDef_t *Font, SSD1306_COLOR_t color);

/**
 * @brief  Draws line on LCD
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  x0: Line X start point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y0: Line Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x1: Line X end point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y1: Line Y end point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c);

/**
 * @brief  Draws rectangle on LCD
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  x: Top left X start point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Top left Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  w: Rectangle width in units of pixels
 * @param  h: Rectangle height in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);

/**
 * @brief  Draws filled rectangle on LCD
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  x: Top left X start point. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Top left Y start point. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  w: Rectangle width in units of pixels
 * @param  h: Rectangle height in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_draw_filled_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);

/**
 * @brief  Draws triangle on LCD
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  x1: First coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y1: First coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x2: Second coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y2: Second coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  x3: Third coordinate X location. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y3: Third coordinate Y location. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_draw_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color);

/**
 * @brief  Draws circle to STM buffer
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  x: X location for center of circle. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Y location for center of circle. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  r: Circle radius in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_draw_circle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c);

/**
 * @brief  Draws filled circle to STM buffer
 * @note   @ref ssd1306_update_screen() must be called after that in order to see updated LCD screen
 * @param  x: X location for center of circle. Valid input is 0 to SSD1306_WIDTH - 1
 * @param  y: Y location for center of circle. Valid input is 0 to SSD1306_HEIGHT - 1
 * @param  r: Circle radius in units of pixels
 * @param  c: Color to be used. This parameter can be a value of @ref SSD1306_COLOR_t enumeration
 * @retval None
 */
void ssd1306_draw_filled_circle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
