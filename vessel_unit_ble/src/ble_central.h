#ifndef BLE_CENTRAL_H
#define BLE_CENTRAL_H

#include <zephyr/types.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>

void start_scan(void);

/* CUSTOM SERVICE UUIDs */

/** @brief GSS Service UUID. */
#define BT_UUID_GSS_VAL \
	BT_UUID_128_ENCODE(0x6dea7370, 0x0bdf, 0x43db, 0x8ea8, 0x1c3a6a7e5909)

/** @brief GPS Characteristic UUID. */
#define BT_UUID_GSS_GPS_VAL \
	BT_UUID_128_ENCODE(0x6dea7371, 0x0bdf, 0x43db, 0x8ea8, 0x1c3a6a7e5909)

/** @brief Humidity Sensor Characteristic UUID. */
#define BT_UUID_GSS_MOB_VAL \
	BT_UUID_128_ENCODE(0x6dea7372, 0x0bdf, 0x43db, 0x8ea8, 0x1c3a6a7e5909)

#define BT_UUID_GSS BT_UUID_DECLARE_128(BT_UUID_GSS_VAL)
#define BT_UUID_GSS_GPS BT_UUID_DECLARE_128(BT_UUID_GSS_GPS_VAL)
#define BT_UUID_GSS_MOB BT_UUID_DECLARE_128(BT_UUID_GSS_MOB_VAL)
#endif /* BLE_CENTRAL_H */
