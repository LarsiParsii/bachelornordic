/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sensors.h"
#include "custom_errno.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensors, LOG_LEVEL_INF);

/**
 * @brief Initialize the sensors
 *
 * @param[in,out] sensors Where to store the sensor data
 *
 * @retval 0 All sensors initialized successfully.
 * @retval -EINVAL Invalid sensors structure pointer.
 * @retval -ENODEVINIT No sensors were initialized.
 * @retval -ESOMEDEVINIT Some sensors were initialized.
 */
int sensors_init(sensors_s *sensors) {
    uint8_t init_count = 0;
    
	if (sensors == NULL) {
		return -EINVAL;
	}
	
    if ((sensors->bme280.dev = DEVICE_DT_GET_ANY(bosch_bme280)) != NULL) {
        if (device_is_ready(sensors->bme280.dev)) {
			LOG_DBG("BME280 device initialized.\n");
			init_count++;
        } else {
			LOG_DBG("BME280 device not ready.\n");
		}
    } else {
		LOG_DBG("No BME280 device found.\n");
	}

    if ((sensors->adxl362.dev = DEVICE_DT_GET_ANY(analog_devices_adxl362)) != NULL) {
        if (device_is_ready(sensors->adxl362.dev)) {
			LOG_DBG("ADXL362 device initialized.\n");
			init_count++;
        } else {
			LOG_DBG("ADXL362 device not ready.\n");
		}
    } else {
		LOG_DBG("No ADXL362 device found.\n");
	}

    if ((sensors->adxl372.dev = DEVICE_DT_GET_ANY(analog_devices_adxl372)) != NULL) {
        if (device_is_ready(sensors->adxl372.dev)) {
			LOG_DBG("ADXL372 device initialized.\n");
			init_count++;
        } else {
            LOG_DBG("ADXL372 device not ready.\n");
        }
    } else {
		LOG_DBG("No ADXL372 device found.\n");
	}

    if (init_count == 0) {
        return -ENODEVINIT;
    } else if (init_count < 3) {
        return -ESOMEDEVINIT;
    }

    return 0;
}

/**
 * @brief Read the BME280 sensor data.
 *
 * @param[in,out] sensor The BME280 sensor data.
 *
 * @retval 0 Sensor data read successfully.
 * @retval -EINVAL Invalid sensor device pointer.
 * @retval -EIO Failed to read sensor data.
 */
static int readBME280(bme280_data *sensor) {
    if (sensor == NULL) {
        return -EINVAL;
    }

    if (sensor_sample_fetch(sensor->dev) < 0 ||
        sensor_channel_get(sensor->dev, SENSOR_CHAN_AMBIENT_TEMP, &sensor->temperature) < 0 ||
        sensor_channel_get(sensor->dev, SENSOR_CHAN_PRESS, &sensor->pressure) < 0 ||
        sensor_channel_get(sensor->dev, SENSOR_CHAN_HUMIDITY, &sensor->humidity) < 0) {
        return -EIO;
    }

    return 0;
}

/**
 * @brief Read the ADXL362 sensor data.
 *
 * @param[in,out] sensor The ADXL362 sensor data.
 *
 * @retval 0 Sensor data read successfully.
 * @retval -EINVAL Invalid sensor device pointer.
 * @retval -EIO Failed to read sensor data.
 */
static int readADXL362(adxl362_data *sensor) {
	if (sensor->dev == NULL) {
		return -EINVAL;
	}

	if (sensor_sample_fetch(sensor->dev) < 0 ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_X, &sensor->accel_x) < 0 ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_Y, &sensor->accel_y) < 0 ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_Z, &sensor->accel_z) < 0 ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_XYZ, &sensor->accel_xyz) < 0 ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_DIE_TEMP, &sensor->die_temp) < 0) {
		return -EIO;
	}

	return 0;
}

/**
 * @brief Read the ADXL372 sensor data.
 *
 * @param[in,out] sensor The ADXL372 sensor data.
 *
 * @retval 0 Sensor data read successfully.
 * @retval -EINVAL Invalid sensor device pointer.
 * @retval -EIO Failed to read sensor data.
 */
static int readADXL372(adxl372_data *sensor) {
	if (sensor->dev == NULL) {
		return -EINVAL;
	}

	if (sensor_sample_fetch(sensor->dev) ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_X, &sensor->accel_x) ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_Y, &sensor->accel_y) ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_Z, &sensor->accel_z) ||
		sensor_channel_get(sensor->dev, SENSOR_CHAN_ACCEL_XYZ, &sensor->accel_xyz)) {
		return -EIO;
	}

	return 0;
}

/**
 * @brief Read sensor data from all available sensors.
 *
 * @param[in,out] sensors The sensors data structure.
 *
 * @retval 0 if all sensor data read successfully.
 * @retval -EINVAL Invalid sensors structure pointer.
 * @retval -ENODEVREAD No sensors read successfully.
 * @retval -ESOMEDEVREAD Some sensors read successfully.
 */
int read_sensors(sensors_s *sensors) {
	if (sensors == NULL) {
		return -EINVAL;
	}
	
	uint8_t read_count = 0;
	
	if (readBME280(&sensors->bme280)) {
		LOG_DBG("Failed to read BME280 sensor data.\n");
	} else {
		read_count++;
	}
	if (readADXL362(&sensors->adxl362)) {
		LOG_DBG("Failed to read ADXL362 sensor data.\n");
	} else {
		read_count++;
	}
	if (readADXL372(&sensors->adxl372)) {
		LOG_DBG("Failed to read ADXL372 sensor data.\n");
	} else {
		read_count++;
	}
	
	if (read_count == 0) {
		return -ENODEVREAD;
	} else if (read_count < 3) {
		return -ESOMEDEVREAD;
	}
	
	return 0;
}