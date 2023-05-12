#include "gps.h"

// Dummy function that returns a gps data struct
gps_data_s getGPSData(void) {
	gps_data_s gps_data = {
		.longitude = 12.3456789,
		.latitude = 0.987654321,
		.altitude = 123.456,
		.accuracy = 0.123
	};
	return gps_data;
}