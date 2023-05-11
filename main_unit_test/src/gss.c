/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief LED Button Service (LBS) sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "gss.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gss, LOG_LEVEL_DBG);

static struct gss_cb_s gss_cb;

/* GPS Sensor Service Declaration */
/* Creates and adds the GSS service to the Bluetooth LE stack */
BT_GATT_SERVICE_DEFINE(gss_svc,
					   BT_GATT_PRIMARY_SERVICE(BT_UUID_GSS),
					   /* STEP 3 - Create and add the Button characteristic */
					   BT_GATT_CHARACTERISTIC(BT_UUID_GSS_GPS,
											  BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
											  BT_GATT_PERM_READ, NULL, NULL,
											  NULL),
					   /* STEP 4 - Create and add the LED characteristic. */
					   BT_GATT_CHARACTERISTIC(BT_UUID_GSS_MOB,
											  BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
											  BT_GATT_PERM_WRITE,
											  NULL, NULL, NULL),

);

/* CALLBACKS */

/* A function to register application callbacks */
int gss_init(struct gss_cb_s *callbacks)
{
	if (callbacks)
	{
		gss_cb.gps_cb = callbacks->gps_cb;
		gss_cb.mob_cb = callbacks->mob_cb;
	}

	return 0;
}