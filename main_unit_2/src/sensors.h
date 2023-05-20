#ifndef sensors_H
#define sensors_H

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

// Struct that holds bme280 sensor data
typedef struct
{
	struct device *dev;
	struct sensor_value temperature; // Temperature in degrees Celsius
	struct sensor_value pressure;	 // Pressure in hPa (I think)
	struct sensor_value humidity;	 // Humidity in %RH (relative humidity)
} bme280_data;

// Struct that holds ADXL362 sensor data
typedef struct
{
	struct device *dev;			   // Pointer to the ADXL362 device
	struct sensor_value accel_x;   // Acceleration in the x direction in m/s^2
	struct sensor_value accel_y;   // Acceleration in the y direction in m/s^2
	struct sensor_value accel_z;   // Acceleration in the z direction in m/s^2
	struct sensor_value accel_xyz; // Acceleration in all directions in m/s^2
	struct sensor_value die_temp;  // Internal temperature of the ADXL362 in degrees Celsius
} adxl362_data;

// Struct that holds ADXL362 sensor data
typedef struct
{
	struct device *dev;			   // Pointer to the ADXL362 device
	struct sensor_value accel_x;   // Acceleration in the x direction in m/s^2
	struct sensor_value accel_y;   // Acceleration in the y direction in m/s^2
	struct sensor_value accel_z;   // Acceleration in the z direction in m/s^2
	struct sensor_value accel_xyz; // Acceleration in all directions in m/s^2
} adxl372_data;

// Struct that holds multiple sensors' data
typedef struct
{
	bme280_data bme280;
	adxl362_data adxl362;
	adxl372_data adxl372;
} sensors_s;


int sensors_init(sensors_s *sensors);
int read_sensors(sensors_s *sensors);

#endif // sensors_H