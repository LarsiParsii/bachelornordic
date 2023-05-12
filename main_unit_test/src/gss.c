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
#include "gps.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gss, LOG_LEVEL_DBG);

static struct gss_cb_s gss_cb;
static bool mob_status;
static gps_data_s gps_data;

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

static ssize_t read_mob_event_status(struct bt_conn *conn,
									 const struct bt_gatt_attr *attr,
									 void *buf,
									 uint16_t len,
									 uint16_t offset)
{
	const char *value;
	if (attr->user_data != NULL)
	{
		// LOG_DBG("User data is not NULL");
		value = attr->user_data;
	}
	else
	{
		// LOG_DBG("User data is NULL");
		return 0;
	}
	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,
			(void *)conn);

	if (gss_cb.mob_cb)
	{
		// Call the application callback function to update
		mob_status = gss_cb.mob_cb();
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
								 sizeof(*value));
	}
	return 0;
}

static ssize_t read_gps_data(struct bt_conn *conn,
									 const struct bt_gatt_attr *attr,
									 void *buf,
									 uint16_t len,
									 uint16_t offset)
{
	const gps_data_s *value;
	if (attr->user_data != NULL)
	{
		// LOG_DBG("User data is not NULL");
		value = attr->user_data;
	}
	else
	{
		// LOG_DBG("User data is NULL");
		return 0;
	}
	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,
			(void *)conn);

	if (gss_cb.gps_cb)
	{
		// Call the application callback function to update
		gps_data = gss_cb.gps_cb();
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
								 sizeof(*value));
	}
	return 0;
}

/* GPS Sensor Service Declaration */
/* Creates and adds the GSS service to the Bluetooth LE stack */
BT_GATT_SERVICE_DEFINE(gss_svc,
					   BT_GATT_PRIMARY_SERVICE(BT_UUID_GSS),
					   /* GPS Coordinate Characteristic */
					   BT_GATT_CHARACTERISTIC(BT_UUID_GSS_GPS,
											  BT_GATT_CHRC_READ,
											  BT_GATT_PERM_READ,
											  read_gps_data,
											  NULL,
											  &gps_data),
					   /* MOB Event Characteristic */
					   BT_GATT_CHARACTERISTIC(BT_UUID_GSS_MOB,
											  BT_GATT_CHRC_READ,
											  BT_GATT_PERM_READ,
											  read_mob_event_status,
											  NULL,
											  &mob_status),

);