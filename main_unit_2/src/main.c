#include <zephyr/kernel.h>
#include <modem/modem_key_mgmt.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include "gnss.h"
#include "lte.h"
#include "coap.h"
#include "sensors.h"
LOG_MODULE_REGISTER(main_c, LOG_LEVEL_INF);

#define APP_COAP_MAX_MSG_LEN 1280
#define APP_COAP_SEND_MAX_MSG_LEN 64

enum tracker_status device_status;
static uint8_t coap_buf[APP_COAP_MAX_MSG_LEN];
static uint8_t coap_sendbuf[APP_COAP_SEND_MAX_MSG_LEN];
struct nrf_modem_gnss_pvt_data_frame current_pvt;
struct nrf_modem_gnss_pvt_data_frame last_pvt;
static int resolve_address_lock = 0;
static int sock;
bool faux_gnss_fix_requested = false; // Generate fake GPS data for testing purposes
sensors_s sensors;


static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	static bool toogle = 1;
	if (has_changed & DK_BTN1_MSK && button_state & DK_BTN1_MSK)
	{
		if (toogle == 1)
		{
			dk_set_led_on(device_status);
		}
		else
		{
			dk_set_led_off(DK_LED1);
			dk_set_led_off(DK_LED2);
			dk_set_led_off(DK_LED3);
		}
		toogle = !toogle;
	}
	if (has_changed & DK_BTN2_MSK && button_state & DK_BTN2_MSK)
	{
		LOG_INF("Fake GNSS Fix Requested\n\r");
		faux_gnss_fix_requested = true;
	}
}

void sensor_thread(void *arg1, void *arg2, void *arg3)
{
	int err;
	// Initialize the sensors
	err = sensors_init(&sensors);
	/*
	if (err != 0)
	{

		return;
	}
	*/

	// Sensor data read and print loop
	while (1)
	{
		read_sensors(&sensors);
		LOG_INF("BME280: Temperature: %d.%06d C, Pressure: %d.%06d hPa, Humidity: %d.%06d %%RH\n",
				sensors.bme280.temperature.val1, sensors.bme280.temperature.val2, sensors.bme280.pressure.val1, sensors.bme280.pressure.val2,
				sensors.bme280.humidity.val1, sensors.bme280.humidity.val2);

		if (sensors.bme280.humidity.val1 > 50)
		{
			LOG_WRN("MOB ALERT!");
			k_sem_give(&sem_send_data);
		}

		k_sleep(K_SECONDS(3));
	}
}

void gps_thread(void *arg1, void *arg2, void *arg3)
{
	while (1)
	{
		// GNSS is event driven and will give a semaphore when a fix is found.
		// This happens in the gnss_event_handler() function in gnss.c.
		if (faux_gnss_fix_requested)
		{
			createFauxFix();
			k_sem_give(&sem_send_data);		 // Signals that there's updated data to send
			faux_gnss_fix_requested = false; // Reset the flag
		}
		k_sleep(K_SECONDS(10));
	}
}

void main(void)
{
	int err, received;
	LOG_INF("Main Unit Version %d.%d.%d started\n", CONFIG_TRACKER_VERSION_MAJOR, CONFIG_TRACKER_VERSION_MINOR, CONFIG_TRACKER_VERSION_PATCH);

	LOG_INF("Starting GNSS....");
	gnss_init_and_start();
	
	err = modem_key_mgmt_write(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_IDENTITY, CONFIG_COAP_DEVICE_NAME, strlen(CONFIG_COAP_DEVICE_NAME));
	if (err)
	{
		LOG_ERR("Failed to write identity: %d\n", err);
		return;
	}

	err = modem_key_mgmt_write(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_PSK, CONFIG_COAP_SERVER_PSK, strlen(CONFIG_COAP_SERVER_PSK));
	if (err)
	{
		LOG_ERR("Failed to write identity: %d\n", err);
		return;
	}

	err = dk_leds_init();
	if (err)
	{
		LOG_ERR("Failed to initlize the LEDs Library");
	}
	device_status = status_nolte;

	modem_configure();

	err = dk_buttons_init(button_handler);
	if (err)
	{
		LOG_ERR("Failed to initlize button handler: %d\n", err);
		return;
	}

	while (1)
	{
		err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_NORMAL);
		if (err != 0)
		{
			LOG_ERR("Failed to activate LTE");
			break;
		}
		k_sem_take(&lte_connected, K_FOREVER);
		if (resolve_address_lock == 0)
		{
			LOG_INF("Resolving the server address\n\r");
			if (server_resolve() != 0)
			{
				LOG_ERR("Failed to resolve server name\n");
				return;
			}
			resolve_address_lock = 1;
		}
		LOG_INF("Sending Data over LTE\r\n");
		if (server_connect(sock) != 0)
		{
			LOG_ERR("Failed to initialize CoAP client\n");
			return;
		}

		if (client_post_send(sock, coap_buf, sizeof(coap_buf), coap_sendbuf, sizeof(coap_sendbuf),
							 current_pvt, last_pvt) != 0)
		{
			LOG_ERR("Failed to send GET request, exit...\n");
			break;
		}
		received = recv(sock, coap_buf, sizeof(coap_buf), 0);

		if (received < 0)
		{
			LOG_ERR("Error reading response\n");
			break;
		}

		if (received == 0)
		{
			LOG_ERR("Disconnected\n");
			break;
		}
		err = client_handle_get_response(coap_buf, received);
		if (err < 0)
		{
			LOG_ERR("Invalid response, exit...\n");
			break;
		}

		(void)close(sock);
		err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_DEACTIVATE_LTE);
		if (err != 0)
		{
			LOG_ERR("Failed to decativate LTE and enable GNSS functional mode");
			break;
		}
	}
	device_status = status_nolte;
	lte_lc_power_off();
	LOG_ERR("Error occoured. Shutting down modem");
}
