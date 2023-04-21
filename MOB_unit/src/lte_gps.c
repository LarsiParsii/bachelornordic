#include <device.h>
#include <logging/log.h>
#include <modem/lte_lc.h>
#include <drivers/gps.h>

#include "lte_gps.h"
#include "lte_gps.h"

LOG_MODULE_REGISTER(lte_gps, CONFIG_APP_LOG_LEVEL);

static struct device *gps_dev;

int init_lte_gps(void)
{
    int err;

    // Initialize LTE
    err = lte_lc_init();
    if (err) {
        LOG_ERR("Failed to initialize LTE: %d", err);
        return err;
    }

    // Initialize GPS
    gps_dev = device_get_binding(CONFIG_GPS_DEV_NAME);
    if (gps_dev == NULL) {
        LOG_ERR("Failed to get GPS device");
        return -ENODEV;
    }

    return 0;
}

int enable_lte(void)
{
    int err = lte_lc_normal();
    if (err) {
        LOG_ERR("Failed to enable LTE: %d", err);
    }
    return err;
}

int disable_lte(void)
{
    int err = lte_lc_power_off();
    if (err) {
        LOG_ERR("Failed to disable LTE: %d", err);
    }
    return err;
}

int enable_gps(void)
{
    int err = gps_start(gps_dev, GPS_MODE_CONTINUOUS);
    if (err) {
        LOG_ERR("Failed to start GPS: %d", err);
    }
    return err;
}

int disable_gps(void)
{
    int err = gps_stop(gps_dev);
    if (err) {
        LOG_ERR("Failed to stop GPS: %d", err);
    }
    return err;
}
