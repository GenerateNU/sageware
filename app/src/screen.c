#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include "screen.h"

/* Use the display chosen by the shield/board */
#define DISPLAY_NODE DT_CHOSEN(zephyr_display)

LOG_MODULE_REGISTER(screen, LOG_LEVEL_INF);

static const struct device *display_dev;
static struct display_capabilities caps;

/* Max line width we’ll support (320px, matches Adafruit 2.8" TFT) */
#define SCREEN_MAX_WIDTH 320
static uint16_t line_buf[SCREEN_MAX_WIDTH];

int screen_init(void)
{
    display_dev = DEVICE_DT_GET(DISPLAY_NODE);
    if (!device_is_ready(display_dev)) {
        printk("Display device not ready!\n");
        return -ENODEV;
    }

    display_get_capabilities(display_dev, &caps);

    LOG_INF("Display caps: %ux%u, cur_fmt=%u, supported_fmt=0x%x, info=0x%x, orient=%u",
            caps.x_resolution,
            caps.y_resolution,
            caps.current_pixel_format,
            caps.supported_pixel_formats,
            caps.screen_info,
            caps.current_orientation);

    if (caps.x_resolution > SCREEN_MAX_WIDTH) {
        printk("Screen width %u exceeds line buffer %u\n",
               caps.x_resolution, SCREEN_MAX_WIDTH);
        return -EINVAL;
    }

    /* Force RGB565 if supported */
    if (caps.supported_pixel_formats & PIXEL_FORMAT_RGB_565) {
        int r = display_set_pixel_format(display_dev, PIXEL_FORMAT_RGB_565);
        if (r) {
            LOG_INF("display_set_pixel_format RGB565 failed (%d)", r);
        } else {
            LOG_INF("Pixel format set to RGB565");
        }
    } else {
        LOG_INF("PIXEL_FORMAT_RGB_565 not in supported_pixel_formats=0x%x",
                caps.supported_pixel_formats);
    }

    /* Force normal orientation (0°) if the driver supports it */
    int r = display_set_orientation(display_dev, DISPLAY_ORIENTATION_NORMAL);
    if (r) {
        LOG_INF("display_set_orientation(NORMAL) failed (%d)", r);
    } else {
        LOG_INF("Orientation set to NORMAL");
    }

    /* Finally, unblank the display */
    int ret = display_blanking_off(display_dev);
    if (ret) {
        printk("Failed to turn off display blanking (%d)\n", ret);
        /* not fatal, but log it */
    } else {
        LOG_INF("Display blanking off");
    }

    return 0;
}

void screen_fill(uint16_t color)
{
    if (!display_dev) {
        return;
    }

    /* Prepare one line of pixels in the desired color */
    for (uint16_t x = 0; x < caps.x_resolution; x++) {
        line_buf[x] = color;
    }

    struct display_buffer_descriptor desc = {
        .width = caps.x_resolution,
        .height = 1,
        .pitch = caps.x_resolution,
        .buf_size = caps.x_resolution * sizeof(uint16_t),
    };

    for (uint16_t y = 0; y < caps.y_resolution; y++) {
        int ret = display_write(display_dev,
                                0,                /* x */
                                y,                /* y */
                                &desc,
                                line_buf);
        if (ret) {
            printk("display_write failed at line %u (%d)\n", y, ret);
            break;
        }
    }
}

void screen_draw_test_pattern(void)
{
    if (!display_dev) {
        return;
    }

    struct display_buffer_descriptor desc = {
        .width = caps.x_resolution,
        .height = 1,
        .pitch = caps.x_resolution,
        .buf_size = caps.x_resolution * sizeof(uint16_t),
    };

    /* Simple vertical gradient on blue channel */
    for (uint16_t y = 0; y < caps.y_resolution; y++) {
        /* Map y (0..height-1) to 0..31 for blue channel */
        uint8_t blue = (y * 31) / (caps.y_resolution - 1);
        uint16_t color = (blue & 0x1F);  /* RGB565: only blue bits */

        for (uint16_t x = 0; x < caps.x_resolution; x++) {
            line_buf[x] = color;
        }

        int ret = display_write(display_dev, 0, y, &desc, line_buf);
        if (ret) {
            printk("display_write failed in pattern at line %u (%d)\n", y, ret);
            break;
        }
    }
}
