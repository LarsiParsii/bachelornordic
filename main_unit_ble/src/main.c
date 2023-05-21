/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include "gss.h"
#include "gps.h"
#include "mob.h"
#include "ble.h"

LOG_MODULE_REGISTER(main_c, LOG_LEVEL_DBG);

#define RUN_LED_BLINK_INTERVAL 1000
#define INDICATE_INTERVAL 500

#define STACKSIZE 1024
#define PRIORITY 7

struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

static gss_cb_s app_callbacks = {
	.gps_cb = getGPSData,
	.mob_cb = getMOBStatus,
};


void send_gps_data_thread(void)
{
	while (1)
	{
		/* Send indication, the function sends notifications only if a client is subscribed */
		gss_send_gps_indicate(getGPSData());
		k_sleep(K_MSEC(INDICATE_INTERVAL));
	}
}

void send_mob_data_thread(void)
{
	while (1)
	{
		/* Send indication, the function sends notifications only if a client is subscribed */
		gss_send_mob_indicate(getMOBStatus());
		k_sleep(K_MSEC(INDICATE_INTERVAL));
	}
}

void main(void)
{
	LOG_ERR("Error log level enabled\n");
	LOG_WRN("Warning log level enabled\n");
	LOG_INF("Info log level enabled\n");
	LOG_DBG("Debug log level enabled\n\n");

	int blink_status = 0;
	int err;

	LOG_INF("Starting main unit application \n");

	err = dk_leds_init();
	if (err)
	{
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return;
	}

	err = gss_init(&app_callbacks);
	if (err)
	{
		LOG_ERR("Failed to init GSS (err:%d)\n", err);
		return;
	}

	ble_init(&connection_callbacks);

	for (;;)
	{
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}

K_THREAD_DEFINE(send_gps_data_thread_id, STACKSIZE, send_gps_data_thread, NULL, NULL,
				NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(send_mob_data_thread_id, STACKSIZE, send_mob_data_thread, NULL, NULL,
				NULL, PRIORITY, 0, 0);