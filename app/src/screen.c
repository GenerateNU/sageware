//LCD display functions with SPI communication built-in
#include "screen.h"
#include <string.h>
/* ST7735-like command definitions (subset) */
#define CMD_SWRESET 0x01
#define CMD_SLPOUT  0x11
#define CMD_DISPON  0x29
#define CMD_CASET   0x2A
#define CMD_RASET   0x2B
#define CMD_RAMWR   0x2C
#define CMD_MADCTL  0x36
#define CMD_COLMOD  0x3A
/* low-level helpers */
static void write_cmd(uint8_t cmd) {
    lcd_hw_cs_low();
    lcd_hw_dc_cmd();
    lcd_hw_spi_write(&cmd, 1);
    lcd_hw_cs_high();
}
static void write_data(const uint8_t *data, size_t len) {
    lcd_hw_cs_low();
    lcd_hw_dc_data();
    lcd_hw_spi_write(data, len);
    lcd_hw_cs_high();
}
static void write_data_u8(uint8_t b) {
    lcd_hw_cs_low();
    lcd_hw_dc_data();
    lcd_hw_spi_write(&b, 1);
    lcd_hw_cs_high();
}
/* set column/row window (inclusive) */
static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t buf[4];
    write_cmd(CMD_CASET); /* column addr set */
    buf[0] = (x0 >> 8) & 0xFF; buf[1] = x0 & 0xFF;
    buf[2] = (x1 >> 8) & 0xFF; buf[3] = x1 & 0xFF;
    write_data(buf, 4);
    write_cmd(CMD_RASET); /* row addr set */
    buf[0] = (y0 >> 8) & 0xFF; buf[1] = y0 & 0xFF;
    buf[2] = (y1 >> 8) & 0xFF; buf[3] = y1 & 0xFF;
    write_data(buf, 4);
    write_cmd(CMD_RAMWR);
}
/* send a block of 16-bit pixels (big buffer -> may be large) */
static void send_pixels_rgb565(const uint16_t *pixels, size_t count) {
    /* create a temporary byte buffer in 2-byte pairs; optimize as needed */
    /* We will send in chunks to avoid huge stack buffers */
    const size_t CHUNK = 64;
    uint8_t buf[CHUNK * 2];
    size_t sent = 0;
    while (sent < count) {
        size_t to_send = (count - sent) < CHUNK ? (count - sent) : CHUNK;
        for (size_t i = 0; i < to_send; ++i) {
            uint16_t p = pixels[sent + i];
            buf[2*i]   = (p >> 8) & 0xFF;
            buf[2*i+1] = p & 0xFF;
        }
        /* raw send of data (DC already set by set_window->CMD_RAMWR) */
        lcd_hw_cs_low();
        lcd_hw_dc_data();
        lcd_hw_spi_write(buf, to_send * 2);
        lcd_hw_cs_high();
        sent += to_send;
    }
}
/* convenience: send repeated color for count pixels (faster) */
static void send_fill_rgb565(uint16_t color, size_t count) {
    const size_t CHUNK_PIXELS = 128;
    uint8_t buf[CHUNK_PIXELS * 2];
    for (size_t i = 0; i < CHUNK_PIXELS; ++i) {
        buf[2*i]   = (color >> 8) & 0xFF;
        buf[2*i+1] = color & 0xFF;
    }
    size_t sent = 0;
    while (sent < count) {
        size_t to_send = (count - sent) < CHUNK_PIXELS ? (count - sent) : CHUNK_PIXELS;
        lcd_hw_cs_low();
        lcd_hw_dc_data();
        lcd_hw_spi_write(buf, to_send * 2);
        lcd_hw_cs_high();
        sent += to_send;
    }
}
/* hardware reset wrapper */
void lcd_reset(void) {
    lcd_hw_rst_low();
    k_msleep(10);  /* 10ms reset pulse */
    lcd_hw_rst_high();
    k_msleep(120); /* 120ms recovery time */
}
/* initialization (basic) */
void lcd_init(void) {
    lcd_reset();
    
    write_cmd(CMD_SWRESET);
    k_msleep(150); /* Wait for reset to complete */
    
    write_cmd(CMD_SLPOUT);
    k_msleep(120); /* Wait for sleep out */
    
    /* 16-bit color */
    uint8_t colmode = 0x55; /* 16-bit/pixel */
    write_cmd(CMD_COLMOD);
    write_data(&colmode, 1);
    
    /* memory access control - row/col order & RGB/BGR */
    uint8_t madctl = 0x00; /* choose as needed: 0x00, 0x48 etc. */
    write_cmd(CMD_MADCTL);
    write_data(&madctl, 1);
    
    write_cmd(CMD_DISPON);
    k_msleep(10); /* Display on delay */
}
/* clear display */
void lcd_clear(uint16_t color) {
    set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    send_fill_rgb565(color, (size_t)LCD_WIDTH * (size_t)LCD_HEIGHT);
}
/* draw a single pixel */
void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    set_window(x, y, x, y);
    uint8_t buf[2] = { (uint8_t)(color >> 8), (uint8_t)color };
    lcd_hw_cs_low();
    lcd_hw_dc_data();
    lcd_hw_spi_write(buf, 2);
    lcd_hw_cs_high();
}
/* fill rectangle */
void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    set_window(x, y, x + w - 1, y + h - 1);
    send_fill_rgb565(color, (size_t)w * (size_t)h);
}
/* --- tiny 5x8 font --- */
static const uint8_t font5x8[][5] = {
    /* ASCII 32..127 - include only 32..127 mapping for simplicity.
       For brevity, implement a minimal set; you can expand this table.
       Here we define a small subset for example characters ' '..'~'. */
    /* space (32) */
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    /* '!' .. etc... For a full font include a full table. */
};
/* fallback: draw rectangle for unsupported font (visible placeholder) */
void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg) {
    /* If you have a full font table, render 5x8 here. For now draw an 6x8 box per char */
    uint16_t w = 6, h = 8;
    /* Draw bg box */
    lcd_fill_rect(x, y, w, h, bg);
    /* Draw a simple pixel pattern for letters (placeholder) */
    if (c != ' ') {
        /* draw a centered vertical line to indicate a glyph */
        for (uint16_t yy = y + 1; yy < y + h - 1; ++yy)
            lcd_draw_pixel(x + 2, yy, fg);
    }
}
void lcd_draw_text(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg) {
    uint16_t cx = x;
    while (*s) {
        lcd_draw_char(cx, y, *s++, fg, bg);
        cx += 6;
        if (cx + 6 > LCD_WIDTH) break;
    }
}