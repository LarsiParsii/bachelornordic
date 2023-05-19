#include "led_button.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

const struct device *led0 = DEVICE_DT_GET(DT_NODELABEL(led0));
const struct device *blue_led = DEVICE_DT_GET(DT_NODELABEL(blue_led));
const struct device *button1 = DEVICE_DT_GET(DT_NODELABEL(nrf91_btn_1));
const struct device *button2 = DEVICE_DT_GET(DT_NODELABEL(nrf91_btn_2));

