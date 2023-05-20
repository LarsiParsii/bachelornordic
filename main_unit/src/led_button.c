#include "led_button.h"
#include "custom_errno.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_button, LOG_LEVEL_INF);

#define ONBOARD_LED_NODE DT_NODELABEL(led0)
#define BLUE_LED_NODE DT_NODELABEL(blue_led)
#define BUTTON1_NODE DT_NODELABEL(nrf91_btn_1)
#define BUTTON2_NODE DT_NODELABEL(nrf91_btn_2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(ONBOARD_LED_NODE, gpios);
static const struct gpio_dt_spec blue_led = GPIO_DT_SPEC_GET(BLUE_LED_NODE, gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(BUTTON1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(BUTTON2_NODE, gpios);

// Interrupt callback function
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&led0);
}

static struct gpio_callback button_cb_data;

int led_button_init(void)
{
	int ret;
	if (!device_is_ready(led0.port))
	{
		printk("LED0 device is ready\n");
		return -EIONOTREADY;
	}
	if (!device_is_ready(blue_led.port))
	{
		printk("BLUE_LED device is ready\n");
		return -EIONOTREADY;
	}
	if (!device_is_ready(button1.port))
	{
		printk("BUTTON1 device is ready\n");
		return -EIONOTREADY;
	}
	if (!device_is_ready(button2.port))
	{
		printk("BUTTON2 device is ready\n");
		return -EIONOTREADY;
	}
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printk("Error %d: failed to configure LED0\n", ret);
		return ret;
	}
	ret = gpio_pin_configure_dt(&blue_led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printk("Error %d: failed to configure BLUE_LED\n", ret);
		return ret;
	}
	ret = gpio_pin_configure_dt(&button1, GPIO_INPUT | GPIO_PULL_UP);
	if (ret < 0)
	{
		printk("Error %d: failed to configure BUTTON1\n", ret);
		return ret;
	}
	ret = gpio_pin_configure_dt(&button2, GPIO_INPUT | GPIO_PULL_UP);
	if (ret < 0)
	{
		printk("Error %d: failed to configure BUTTON2\n", ret);
		return ret;
	}
	
	// Set up the button callbacks
	ret = gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
	{
		printk("Error %d: failed to configure BUTTON1 interrupt\n", ret);
		return ret;
	}
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button1.pin));
	
	ret = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
	{
		printk("Error %d: failed to configure BUTTON2 interrupt\n", ret);
		return ret;
	}
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button2.pin));
	gpio_add_callback(button1.port, &button_cb_data);
}

// A more descriptive name for the function
static bool readPin(const struct gpio_dt_spec *button_spec)
{
	return gpio_pin_get_dt(button_spec);
}

bool readButton0(void)
{
	return readPin(&button1);
}

bool readButton1(void)
{
	return readPin(&button2);
}

static void setLed(const struct gpio_dt_spec *led_spec, bool state)
{
	int ret;

	ret = gpio_pin_configure_dt(led_spec, GPIO_OUTPUT);
	if (ret != 0)
	{
		printk("Error %d: failed to configure LED\n", ret);
		return;
	}

	gpio_pin_set_dt(led_spec, (int)state);
}

void setLed0(bool state)
{
	setLed(&led0, state);
}

void setBlueLed(bool state)
{
	setLed(&blue_led, state);
}