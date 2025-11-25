//LCD display functions with SPI communication built-in
//SPI1_SCK : PA5
//SPI1_MOSI: PA7
//SPI1_MISO : PG9
//CS: PB8
#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* LCD dimensions (adjust for your panel) */
#ifndef LCD_WIDTH
#define LCD_WIDTH  320
#endif

#ifndef LCD_HEIGHT
#define LCD_HEIGHT 240
#endif

/* Common RGB565 color definitions */
#define LCD_COLOR_BLACK   0x0000
#define LCD_COLOR_WHITE   0xFFFF
#define LCD_COLOR_RED     0xF800
#define LCD_COLOR_GREEN   0x07E0
#define LCD_COLOR_BLUE    0x001F
#define LCD_COLOR_YELLOW  0xFFE0
#define LCD_COLOR_CYAN    0x07FF
#define LCD_COLOR_MAGENTA 0xF81F

/* Hardware abstraction layer - platform must provide these: */
void lcd_hw_cs_low(void);
void lcd_hw_cs_high(void);
void lcd_hw_dc_cmd(void);   /* Set DC pin low for command mode */
void lcd_hw_dc_data(void);  /* Set DC pin high for data mode */
void lcd_hw_rst_low(void);
void lcd_hw_rst_high(void);
void lcd_hw_spi_write(const uint8_t *buf, size_t len);

/* Public LCD API */
void lcd_reset(void);
void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg);
void lcd_draw_text(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg);

#endif /* LCD_H_ */