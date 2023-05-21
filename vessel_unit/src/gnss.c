#include "gnss.h"
#include <stdio.h>
#include <zephyr/logging/log.h>
#include <nrf_modem_gnss.h>
#include <zephyr/random/rand32.h>

LOG_MODULE_REGISTER(GNSS, LOG_LEVEL_DBG);

/* VARIABLES */
extern enum tracker_status device_status;

/* FUNCTION DEFINITIONS */
void print_fix_data(struct nrf_modem_gnss_pvt_data_frame *pvt_data)
{
	printk("Latitude:       %.06f\n", pvt_data->latitude);
	printk("Longitude:      %.06f\n", pvt_data->longitude);
	printk("Altitude:       %.01f m\n", pvt_data->altitude);
	printk("Time (UTC):     %02u:%02u:%02u.%03u\n",
		   pvt_data->datetime.hour,
		   pvt_data->datetime.minute,
		   pvt_data->datetime.seconds,
		   pvt_data->datetime.ms);
}

static void gnss_event_handler(int event)
{
	int retval;
	switch (event)
	{
	case NRF_MODEM_GNSS_EVT_PVT:
		LOG_DBG("Searching for GNSS Satellites....\n\r");
		device_status = status_searching;
		break;
	case NRF_MODEM_GNSS_EVT_FIX:
		LOG_INF("GNSS fix event\n\r");
		break;
	case NRF_MODEM_GNSS_EVT_PERIODIC_WAKEUP:
		LOG_INF("GNSS woke up in periodic mode\n\r");
		break;
	case NRF_MODEM_GNSS_EVT_BLOCKED:
		LOG_INF("GNSS is blocked by LTE event\n\r");
		break;
	case NRF_MODEM_GNSS_EVT_SLEEP_AFTER_FIX:
		LOG_INF("GNSS enters sleep because fix was achieved in periodic mode\n\r");
		device_status = status_fixed;
		retval = nrf_modem_gnss_read(&vessel_last_pvt, sizeof(vessel_last_pvt), NRF_MODEM_GNSS_DATA_PVT);
		if (retval == 0)
		{
			current_pvt = vessel_last_pvt;
			print_fix_data(&current_pvt);
			k_sem_give(&sem_send_new_data);
		}
		break;
	case NRF_MODEM_GNSS_EVT_SLEEP_AFTER_TIMEOUT:
		LOG_DBG("GNSS enters sleep because fix retry timeout was reached\n\r");
		break;

	default:

		break;
	}
}

int gnss_init_and_start(void)
{
	int err;
#if defined(CONFIG_GNSS_HIGH_ACCURACY_TIMING_SOURCE)
	err = nrf_modem_gnss_timing_source_set(NRF_MODEM_GNSS_TIMING_SOURCE_TCXO);
	if (err < 0)
	{
		LOG_ERR("Failed to set TCXO timing source: %d", err);
		return err;
	}
#endif
#if defined(CONFIG_GNSS_LOW_ACCURACY) || defined(CONFIG_BOARD_THINGY91_NRF9160_NS)
	uint8_t use_case;
	use_case = NRF_MODEM_GNSS_USE_CASE_MULTIPLE_HOT_START | NRF_MODEM_GNSS_USE_CASE_LOW_ACCURACY;
	err = nrf_modem_gnss_use_case_set(use_case);
	if (err < 0)
	{
		LOG_ERR("Failed to set low accuracy use case: %d", err);
		return err;
	}
#endif
	/* Configure GNSS event handler . */
	err = nrf_modem_gnss_event_handler_set(gnss_event_handler);
	if (err < 0)
	{
		LOG_ERR("Failed to set GNSS event handler: %d", err);
		return err;
	}
	
	err = nrf_modem_gnss_fix_interval_set(CONFIG_TRACKER_PERIODIC_INTERVAL);
	if (err < 0)
	{
		LOG_ERR("Failed to set GNSS fix interval: %d", err);
		return err;
	}
	
	err = nrf_modem_gnss_fix_retry_set(CONFIG_TRACKER_PERIODIC_TIMEOUT);
	if (err < 0)
	{
		LOG_ERR("Failed to set GNSS fix retry: %d", err);
		return err;
	}
	
	err = nrf_modem_gnss_start();
	if (err < 0)
	{
		LOG_ERR("Failed to start GNSS");
		return err;
	}
	
	err = nrf_modem_gnss_prio_mode_disable();		// DISABLED GNSS PRIORITY MODE
	if (err < 0)
	{
		LOG_ERR("Error setting GNSS priority mode");
		return err;
	}
	return 0;
}

double generate_random_double(double min, double max)
{
	uint32_t randNum = sys_rand32_get();
	double fraction = ((double)randNum) / UINT32_MAX; // Normalize to [0, 1]
	return min + fraction * (max - min);			  // Scale to [min, max]
}

void createFauxFix(void)
{
	LOG_INF("Faux GNSS fix requested");
	current_pvt = vessel_last_pvt;
	
	current_pvt.latitude = generate_random_double(-90, 90);
	current_pvt.longitude = generate_random_double(-180, 180);
	current_pvt.altitude = generate_random_double(0, 1000);
	current_pvt.accuracy = generate_random_double(0, 100);
	current_pvt.datetime.day = sys_rand32_get() % 31;
	current_pvt.datetime.month = sys_rand32_get() % 12;
	current_pvt.datetime.year = 2023;
	current_pvt.datetime.hour = sys_rand32_get() % 24;
	current_pvt.datetime.minute = sys_rand32_get() % 60;
	current_pvt.datetime.seconds = sys_rand32_get() % 60;
	current_pvt.datetime.ms = sys_rand32_get() % 1000;
	
	print_fix_data(&current_pvt);
	k_sem_give(&sem_send_new_data);
}