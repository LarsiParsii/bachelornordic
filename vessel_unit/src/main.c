#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/sys/printk.h>

#include "custom_errno.h"
#include "gnss.h"
#include "lte.h"
#include "coap.h"
#include "led_button.h"

LOG_MODULE_REGISTER(main_c_, LOG_LEVEL_DBG);

#define DOWNLOAD_THREAD_PRIORITY 9
#define UPLOAD_THREAD_PRIORITY 8
#define GPS_THREAD_PRIORITY 7
#define STACKSIZE 2048

#define APP_COAP_MAX_MSG_LEN 1280
#define APP_COAP_SEND_MAX_MSG_LEN 128
static uint8_t coap_buf[APP_COAP_MAX_MSG_LEN];			// Holds the CoAP message
static uint8_t coap_buf_rec[APP_COAP_MAX_MSG_LEN];			// Holds the CoAP message
static uint8_t coap_databuf[APP_COAP_SEND_MAX_MSG_LEN]; // Holds the data to send in the CoAP message

struct nrf_modem_gnss_pvt_data_frame vessel_current_pvt;
struct nrf_modem_gnss_pvt_data_frame vessel_last_pvt;
struct nrf_modem_gnss_pvt_data_frame main_unit_current_pvt;
struct nrf_modem_gnss_pvt_data_frame main_unit_last_pvt;
static int resolve_address_lock = 0;
static int sock;
bool shutdown_flag = false;					   // Flag to signal a fault that should shut down the system
volatile bool faux_gnss_fix_requested = false; // Generate fake GPS data for testing purposes
volatile bool download_data = false;		   // Flag to signal that data should be downloaded

// Semaphores included from other files:
// K_SEM_DEFINE(lte_connected, 0, 1) from lte.c
// K_SEM_DEFINE(gnss_fix_sem, 0, 1) from gnss.c

K_THREAD_STACK_DEFINE(download_thread_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(upload_thread_stack, STACKSIZE);
K_THREAD_STACK_DEFINE(gps_thread_stack, STACKSIZE);

struct k_thread download_thread_data;
struct k_thread upload_thread_data;
struct k_thread gnss_thread_data;

k_tid_t download_thread_id;
k_tid_t upload_thread_id;
k_tid_t gnss_thread_id;

K_SEM_DEFINE(sem_lte_busy, 1, 1);
K_SEM_DEFINE(sem_send_new_data, 0, 1);
K_SEM_DEFINE(sem_get_new_data, 0, 1);

void gnss_thread(void *arg1, void *arg2, void *arg3)
{
	while (1)
	{
		if (shutdown_flag)
		{
			// Do any necessary cleanup here before exiting
			break;
		}
		// GNSS is event driven and will give a semaphore when a fix is found.
		// This happens in the gnss_event_handler() function in gnss.c.
		if (faux_gnss_fix_requested)
		{
			setOnboardLed(true);
			createFauxFix(&vessel_current_pvt);
			print_fix_data(&vessel_current_pvt);
			k_sem_give(&sem_send_new_data);	 // Signals that there's updated data to send
			faux_gnss_fix_requested = false; // Reset the flag
		}
		if (download_data == true)
		{
			download_data = false; // Reset the flag
			setOnboardLed(true);
			k_sem_give(&sem_get_new_data); // Signals that there's updated data to send
		}

		k_sleep(K_SECONDS(1));
	}
}

void download_thread(void *arg1, void *arg2, void *arg3)
{
	int err, received;

	while (1)
	{
		if (shutdown_flag)
		{
			lte_lc_power_off();
			LOG_ERR("Shutting down download thread...");
			break;
		}

		if ((k_sem_take(&sem_get_new_data, K_FOREVER) == 0) && (k_sem_take(&sem_lte_busy, K_FOREVER) == 0)) // Wait for data to send
		{
			LOG_INF("Downloading data over LTE\n\r");
			
			err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_NORMAL);
			if (err != 0)
			{
				LOG_ERR("Failed to activate LTE");
				break;
			}
			LOG_INF("Waiting for LTE connection\n\r");
			k_sem_take(&lte_connected, K_FOREVER); // Wait for the LTE connection to be established
			LOG_INF("LTE connected\n\r");

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

			LOG_INF("Downloading Data over LTE\r\n");
			if (server_connect(sock) != 0)
			{
				LOG_ERR("Failed to initialize CoAP client\n");
				return;
			}

			if (client_get_send(sock, coap_buf, sizeof(coap_buf)) != 0)
			{
				LOG_ERR("Failed to send GET request, exit...\n");
				break;
			}
			received = recv(sock, coap_buf_rec, sizeof(coap_buf_rec), 0);

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

			// Handle the response
			err = client_handle_get_response(coap_buf_rec, received);
			if (err < 0)
			{
				LOG_ERR("Invalid response, exit...\n");
				break;
			}

			(void)close(sock);
			err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_DEACTIVATE_LTE);
			if (err != 0)
			{
				LOG_ERR("Failed to deactivate LTE and enable GNSS functional mode");
				break;
			}

			LOG_INF("Data received successfully\n\r");
			k_sem_give(&sem_lte_busy); // Release the LTE semaphore
			setOnboardLed(false);
		}

		else
		{
			k_sleep(K_MSEC(200));
		}
	}
}


void upload_thread(void *arg1, void *arg2, void *arg3)
{
	int err, received;

	while (1)
	{
		if (shutdown_flag)
		{
			lte_lc_power_off();
			LOG_ERR("Shutting down upload thread...");
			break;
		}

		if ((k_sem_take(&sem_send_new_data, K_FOREVER) == 0) && (k_sem_take(&sem_lte_busy, K_FOREVER) == 0)) // Wait for data to send
		{
			LOG_INF("Sending data over LTE\n\r");
			
			err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_NORMAL);
			if (err != 0)
			{
				LOG_ERR("Failed to activate LTE");
				break;
			}
			LOG_INF("Waiting for LTE connection\n\r");
			k_sem_take(&lte_connected, K_FOREVER); // Wait for the LTE connection to be established
			LOG_INF("LTE connected\n\r");

			// Send the data to the server
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

			if (client_post_send(sock, coap_buf, sizeof(coap_buf), coap_databuf, sizeof(coap_databuf),
								 vessel_current_pvt, NULL) != 0)
			{
				LOG_ERR("Failed to send POST request, exit...\n");
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

			// Read it back
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

			LOG_INF("Data transmitted and readback successfull\n\r");
			k_sem_give(&sem_lte_busy); // Release the LTE semaphore
			setOnboardLed(false);
		}

		else
		{
			k_sleep(K_MSEC(200));
		}
	}
}

void main(void)
{
	LOG_INF("Vessel Unit Version %s started\n", CONFIG_DEVICE_VERSION);
	int err;

	err = led_button_init();
	if (err)
	{
		LOG_ERR("Failed to initialize LEDs and buttons: %d\n", err);
		return;
	}
	setOnboardLed(false);
	
	// Setup SEC_TAG_DOWNLOAD and SEC_TAG_UPLOAD
	setup_sec_tags();
	
	
	modem_configure();

	LOG_INF("Starting GNSS....");
	err = gnss_init_and_start();
	if (!err)
	{
		LOG_INF("GNSS started successfully\n");
	}
	k_sem_give(&sem_lte_busy); // Release the LTE semaphore

	download_thread_id = k_thread_create(&download_thread_data, download_thread_stack,
										 STACKSIZE, download_thread, NULL, NULL, NULL,
										 UPLOAD_THREAD_PRIORITY, 0, K_NO_WAIT);

	upload_thread_id = k_thread_create(&upload_thread_data, upload_thread_stack,
									   STACKSIZE, upload_thread, NULL, NULL, NULL,
									   UPLOAD_THREAD_PRIORITY, 0, K_NO_WAIT);

	gnss_thread_id = k_thread_create(&gnss_thread_data, gps_thread_stack,
									 STACKSIZE, gnss_thread, NULL, NULL, NULL,
									 GPS_THREAD_PRIORITY, 0, K_NO_WAIT);
}