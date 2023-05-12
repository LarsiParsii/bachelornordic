#include <zephyr/logging/log.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>

#include "ble.h"
#include "gss.h"

LOG_MODULE_REGISTER(conn);

/* ADVERTISING PARAMETERS */
/* Connectable advertising and use identity address */
static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY),
														   800,	  /*Min Advertising Interval 500ms (800*0.625ms) */
														   2400,  /*Max Advertising Interval 1500ms (2400*0.625ms)*/
														   NULL); /* Set to NULL for undirected advertising*/

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, CONFIG_BT_DIS_MODEL, sizeof(CONFIG_BT_DIS_MODEL) - 1),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, CONFIG_BT_DIS_SERIAL_NUMBER_STR, sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR) - 1),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_GSS_VAL),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

int ble_init(struct bt_conn_cb *connection_cb)
{
	int err;
	err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return err;
	}
	bt_conn_cb_register(connection_cb);

	LOG_INF("Bluetooth initialized\n");
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad),
						  sd, ARRAY_SIZE(sd));
	if (err)
	{
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return err;
	}

	LOG_INF("Advertising successfully started\n");
	return 0;
}

/* BLUETOOTH CONNECTION CALLBACKS */

void on_connected(struct bt_conn *conn, uint8_t err)
{
	if (err)
	{
		printk("Connection failed (err %u)\n", err);
		return;
	}

	printk("Connected\n");

	dk_set_led_on(CON_STATUS_LED);
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	dk_set_led_off(CON_STATUS_LED);
}