#ifndef led_button_H
#define led_button_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED_BLUE_NODE DT_ALIAS(blue_led)
#define BUTTON1_NODE DT_ALIAS(nrf91_btn_1)
#define BUTTON2_NODE DT_ALIAS(nrf91_btn_2)

const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(LED_BLUE_NODE, gpios);
const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(BUTTON1_NODE, gpios);
const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(BUTTON2_NODE, gpios);

#endif  // led_button_H