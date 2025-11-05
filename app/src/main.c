// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>

// This is accessing the DeviceTree node through its label (can see the entire devicetree in generated zephyr.dts file in the build directory)
#define LED2 DT_ALIAS(led0)

//TEST WHITE LED
#define WHITE_LED DT_ALIAS(whiteled)

// 'gpios' refers to the property name within the led0 node, which is itself inside the leds parent node in the devicetree
// you can see this^^ in code if you search for the 'zephyr/boards/nordic/nrf54l15dk/nrf54l15dk_common.dtsi' file
static const struct gpio_dt_spec led_2 = GPIO_DT_SPEC_GET(LED2, gpios);
static const struct gpio_dt_spec white_led = GPIO_DT_SPEC_GET(WHITE_LED, gpios);

int main(void)
{
    // led_2.port is used because you need to check if the gpio controller itself is ready to be used, if you just do led_2.pin you will get a number like '9'
    // also check white_led port is ready
    if (!device_is_ready(led_2.port) || !device_is_ready(white_led.port))
    {
        printk("GPIO device not ready\n");
        return 0;
    }

    int state = 0;
    int ret;

    // configuration of led_2 pin (on-board LED) - optional
    ret = gpio_pin_configure_dt(&led_2, GPIO_OUTPUT);
    if (ret != 0)
    {
        printk("Failed to configure led_2: %d\n", ret);
        return 0;
    }

    // configuration of white_led pin (external LED)
    ret = gpio_pin_configure_dt(&white_led, GPIO_OUTPUT);
    if (ret != 0)
    {
        printk("Failed to configure white_led: %d\n", ret);
        return 0;
    }

    while (true)
{
    state = !state;

    // blink on-board LED
    ret = gpio_pin_set_dt(&led_2, state);
    if (ret != 0)
    {
        printk("ERROR: failed to set LED2 (on-board)\n");
        return 0;
    }

    // blink external LED
    ret = gpio_pin_set_dt(&white_led, state);
    if (ret != 0)
    {
        printk("ERROR: failed to set WHITE_LED (external)\n");
        return 0;
    }
    printk("----------------------\n");
    printk("white_led: port=%s pin=%d flags=0x%x\n",
       white_led.port->name, white_led.pin, white_led.dt_flags);

    printk("LED2 (on-board) state: %d\n", state);
    printk("WHITE_LED (external) state: %d\n", state);

    k_msleep(4000);
}


    return 0;
}
