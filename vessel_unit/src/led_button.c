#include "led_button.h"
#include "custom_errno.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_button, LOG_LEVEL_DBG);

#define ONBOARD_LED_NODE DT_NODELABEL(led0)
#define BLUE_LED_NODE DT_NODELABEL(blueled)
#define BUTTON1_NODE DT_NODELABEL(nrf91_btn_1)
#define BUTTON2_NODE DT_NODELABEL(nrf91_btn_2)

static const struct gpio_dt_spec onboard_led = GPIO_DT_SPEC_GET(ONBOARD_LED_NODE, gpios);
static const struct gpio_dt_spec blue_led = GPIO_DT_SPEC_GET(BLUE_LED_NODE, gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(BUTTON1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(BUTTON2_NODE, gpios);

/* Interrupt callback functions */
// Button 1 callback
void button1_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{	
    LOG_DBG("Button 1 pressed\n");
	faux_gnss_fix_requested = true;
}

// Button 2 callback
void button2_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{	
    LOG_DBG("Button 2 pressed\n");
    gpio_pin_toggle_dt(&blue_led);
    download_data = true;
}

static struct gpio_callback button1_cb_data;
static struct gpio_callback button2_cb_data;

int led_button_init(void)
{
	int ret;
	LOG_INF("Initializing LEDs and buttons\n");
	if (!device_is_ready(onboard_led.port))
	{
		LOG_ERR("LED0 device not ready\n");
		return -EIONOTREADY;
	}
	if (!device_is_ready(blue_led.port))
	{
		LOG_ERR("BLUE_LED device not ready\n");
		return -EIONOTREADY;
	}
	if (!device_is_ready(button1.port))
	{
		LOG_ERR("BUTTON1 device not ready\n");
		return -EIONOTREADY;
	}
	if (!device_is_ready(button2.port))
	{
		LOG_ERR("BUTTON2 device not ready\n");
		return -EIONOTREADY;
	}
	ret = gpio_pin_configure_dt(&onboard_led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		LOG_ERR("Error %d: failed to configure LED0\n", ret);
		return ret;
	}
	ret = gpio_pin_configure_dt(&blue_led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		LOG_ERR("Error %d: failed to configure BLUE_LED\n", ret);
		return ret;
	}
	ret = gpio_pin_configure_dt(&button1, GPIO_INPUT | GPIO_PULL_UP );
	if (ret < 0)
	{
		LOG_ERR("Error %d: failed to configure BUTTON1\n", ret);
		return ret;
	}
	ret = gpio_pin_configure_dt(&button2, GPIO_INPUT | GPIO_PULL_UP);
	if (ret < 0)
	{
		LOG_ERR("Error %d: failed to configure BUTTON2\n", ret);
		return ret;
	}
	
	// Set up the button callbacks
	ret = gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
	{
		LOG_ERR("Error %d: failed to configure BUTTON1 interrupt\n", ret);
		return ret;
	}
	
	ret = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
	{
		LOG_ERR("Error %d: failed to configure BUTTON2 interrupt\n", ret);
		return ret;
	}
	gpio_init_callback(&button1_cb_data, button1_pressed, BIT(button1.pin));
    gpio_add_callback(button1.port, &button1_cb_data);
	
    gpio_init_callback(&button2_cb_data, button2_pressed, BIT(button2.pin));
    gpio_add_callback(button2.port, &button2_cb_data);
	
	return 0;
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

void setOnboardLed(bool state)
{
	gpio_pin_set_dt(&onboard_led, (int)state);
}

void setBlueLed(bool state)
{
	gpio_pin_set_dt(&blue_led, (int)state);
}