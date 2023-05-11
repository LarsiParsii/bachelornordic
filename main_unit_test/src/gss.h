#ifndef GSS_H_
#define GSS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <dk_buttons_and_leds.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#define RUN_STATUS_LED          DK_LED1
#define CON_STATUS_LED          DK_LED2
/* LED to control */
#define USER_LED DK_LED3
/* Button to monitor */
#define USER_BUTTON DK_BTN1_MSK

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

#define BT_UUID_GSS           BT_UUID_DECLARE_128(BT_UUID_GSS_VAL)
#define BT_UUID_GSS_GPS       BT_UUID_DECLARE_128(BT_UUID_GSS_GPS_VAL)
#define BT_UUID_GSS_MOB       BT_UUID_DECLARE_128(BT_UUID_GSS_MOB_VAL)


/* CALLBACK STRUCTURE */

/** @brief Callback type for reading the GPS data */
typedef int (*gps_cb_t)(void);

/** @brief Callback type for humidity sensor reading */
typedef int (*mob_cb_t)(void);

/** @brief Callback type for when an LED state change is received. */
typedef void (*led_cb_t)(const bool led_state);

/** @brief Callback type for when the button state is pulled. */
typedef bool (*button_cb_t)(void);

/** @brief Callback struct used by the GSS Service. */
struct gss_cb_s
{
	/** GPS reading callback. */
	gps_cb_t gps_cb;
	/** Humidity change callback. */
	mob_cb_t mob_cb;
};


/* FUNCTION PROTOTYPES */

/* Callbacks */
/** @brief Initialize the GSS Service.
 * This function registers application callback functions with the GSS Service
 * @param[in] callbacks Struct containing pointers to callback functions
 *			used by the service. This pointer can be NULL
 *			if no callback functions are defined.
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int gss_init(struct gss_cb_s *callbacks);
static ssize_t write_led(struct bt_conn *conn,
						 const struct bt_gatt_attr *attr,
						 const void *buf,
						 uint16_t len, uint16_t offset, uint8_t flags);



#ifdef __cplusplus
}
#endif

#endif /* GSS_H_ */