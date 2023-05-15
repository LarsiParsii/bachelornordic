#include <zephyr/kernel.h>
#include <modem/modem_key_mgmt.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include "gnss.h"
#include "lte.h"
#include "coap.h"
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
		faux_gnss_fix_requested = true;
		k_sem_give(&gnss_fix_sem);
	}
}

void main(void)
{
	int err, received;
	LOG_INF("Main Unit Version %d.%d.%d started\n", CONFIG_TRACKER_VERSION_MAJOR, CONFIG_TRACKER_VERSION_MINOR, CONFIG_TRACKER_VERSION_PATCH);

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

	LOG_INF("Starting GNSS....");
	gnss_init_and_start();

	while (1)
	{
		k_sem_take(&gnss_fix_sem, K_FOREVER);
		if (faux_gnss_fix_requested)
		{
			LOG_INF("Faux GNSS fix requested");
			faux_gnss_fix_requested = false;
			current_pvt = last_pvt;
			current_pvt.latitude = 59.3294;
			current_pvt.longitude = 18.0686;
			current_pvt.altitude = 0;
			current_pvt.datetime.day = 15;
			current_pvt.datetime.month = 5;
			current_pvt.datetime.year = 2023;
			current_pvt.datetime.hour = 02;
			current_pvt.datetime.minute = 47;
			current_pvt.datetime.seconds = 06;
			current_pvt.datetime.ms = 365;
			current_pvt.accuracy = 5;
			print_fix_data(&current_pvt);
		}
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
