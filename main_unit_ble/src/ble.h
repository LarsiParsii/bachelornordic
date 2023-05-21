#ifndef ble_H
#define ble_H

/* Includes */
#include <zephyr/bluetooth/conn.h>

/* Defines */

/* Function prototypes */
int ble_init(struct bt_conn_cb *bt_connection_cb);

/* Bluetooth connection callbacks */
void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t reason);

#endif  // ble_H