/**
 * @file gss.c
 * @brief Implementation of the GPS Sensor Service (GSS).
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

static gss_cb_s		gss_cb;
static gps_data_s	gps_data;
static bool			mob_status;

static bool indicate_enabled_gps;
static bool indicate_enabled_mob;
static struct bt_gatt_indicate_params ind_params_gps;	// Indication parameters for GPS data
static struct bt_gatt_indicate_params ind_params_mob;	// Indication parameters for MOB event

/* CALLBACKS */

/**
 * @brief Registers application callbacks for the GSS service.
 *
 * @param callbacks Pointer to the structure containing the callback functions.
 * @return int 0 on success, negative error code otherwise.
 */
int gss_init(gss_cb_s *callbacks)
{
	if (callbacks)
	{
		gss_cb.gps_cb = callbacks->gps_cb;
		gss_cb.mob_cb = callbacks->mob_cb;
	}
	
	return 0;
}

/**
 * @brief Callback invoked when reading MOB event status.
 *
 * @param conn Connection object.
 * @param attr Attribute object.
 * @param buf Buffer to store the attribute value.
 * @param len Length of the buffer.
 * @param offset Offset within the attribute value.
 * @return ssize_t Number of bytes read, or negative error code on failure.
 */
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

/* INDICATION FUNCTIONS */

static void gss_ccc_gps_changed(const struct bt_gatt_attr *attr,
								uint16_t value)
{
	indicate_enabled_gps = (value == BT_GATT_CCC_INDICATE);
}
static void gss_ccc_mob_changed(const struct bt_gatt_attr *attr,
								uint16_t value)
{
	indicate_enabled_mob = (value == BT_GATT_CCC_INDICATE);
}

//This function is called when a remote device has acknowledged the indication at its host layer
static void indicate_started_gps_cb(struct bt_conn *conn,
			struct bt_gatt_indicate_params *params, uint8_t err)
{
	LOG_DBG("GPS indication started %s\n", err != 0U ? "fail" : "success");
}
static void indicate_started_mob_cb(struct bt_conn *conn,
			struct bt_gatt_indicate_params *params, uint8_t err)
{
	LOG_DBG("MOB indication started %s\n", err != 0U ? "fail" : "success");
}
static void indicate_ended_gps_cb()
{
	LOG_DBG("GPS indication finished.\n");
}
static void indicate_ended_mob_cb()
{
	LOG_DBG("MOB indication finished.\n");
}

/* INDICATIONS */
int gss_send_gps_indicate(gps_data_s gps_data)
{
	if (!indicate_enabled_gps) {
		return -EACCES;
	}
	ind_params_gps.uuid = BT_UUID_GSS_GPS;
	ind_params_gps.func = indicate_started_gps_cb; //A remote device has ACKed at its host layer (ATT ACK)
	ind_params_gps.destroy = indicate_ended_gps_cb;
	ind_params_gps.data = &gps_data;
	ind_params_gps.len = sizeof(gps_data);
	LOG_DBG("Sending GPS indication");
	return bt_gatt_indicate(NULL, &ind_params_gps);
}

int gss_send_mob_indicate(bool mob_status)
{
	if (!indicate_enabled_mob)
	{
		return -EACCES;
	}
	ind_params_mob.uuid = BT_UUID_GSS_MOB;
	ind_params_mob.func = indicate_started_mob_cb; // A remote device has ACKed at its host layer (ATT ACK)
	ind_params_mob.destroy = indicate_ended_mob_cb;
	ind_params_mob.data = &mob_status;
	ind_params_mob.len = sizeof(mob_status);
	return bt_gatt_indicate(NULL, &ind_params_mob);
}

/* GPS Sensor Service Declaration */
/* Creates and adds the GSS service to the Bluetooth LE stack */
BT_GATT_SERVICE_DEFINE(gss_svc,
						BT_GATT_PRIMARY_SERVICE(BT_UUID_GSS),
						/* GPS Coordinate Characteristic */
						BT_GATT_CHARACTERISTIC(BT_UUID_GSS_GPS,
											  BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
											  BT_GATT_PERM_READ,
											  read_gps_data,
											  NULL,
											  &gps_data),
						BT_GATT_CCC(gss_ccc_gps_changed,
								BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
						/* MOB Event Characteristic */
						BT_GATT_CHARACTERISTIC(BT_UUID_GSS_MOB,
											  BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
											  BT_GATT_PERM_READ,
											  read_mob_event_status,
											  NULL,
											  &mob_status),
						BT_GATT_CCC(gss_ccc_mob_changed,
								BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

);