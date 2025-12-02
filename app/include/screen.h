//LCD display functions with SPI communication built-in
//SPI1_SCK : PA5
//SPI1_MOSI: PA7
//SPI1_MISO : PG9
//CS: PB8

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h> //delete?
#include <stddef.h> //delete?

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Provide these on your platform: */
//Set CS high: When the device is not in use or when the subnode is deselected.
//Set CS low: When the device is in use or when the subnode is selected. The timing requirements for the CS signal are crucial for proper SPI communication. 
void lcd_hw_cs_low(void);
void lcd_hw_cs_high(void);
void lcd_hw_dc_cmd(void); //write data?
void lcd_hw_dc_data(void);//write command
void lcd_hw_rst_low(void);
void lcd_hw_rst_high(void);

/* send a buffer of bytes over SPI (blocking) */
void lcd_hw_spi_write(const uint8_t *buf, size_t len); //write data buffer
// void LCD_WriteCommand(uint8_t cmd); write data
// void LCD_WriteData8(uint8_t data); write command

/* LCD dimensions (adjust for your panel) */
#ifndef LCD_WIDTH
#define LCD_WIDTH  320
#endif
#ifndef LCD_HEIGHT
#define LCD_HEIGHT 240
#endif

/* API */
void lcd_init(void);
void lcd_reset(void);

void lcd_clear(uint16_t color);
void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/* text */
void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg);
void lcd_draw_text(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg);

/* helper convert RGB888 -> RGB565 */
static inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

#endif /* LCD_H */
