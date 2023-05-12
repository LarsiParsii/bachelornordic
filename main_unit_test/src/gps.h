#ifndef gps_H_
#define gps_H_

// Includes

// Structs
typedef struct {
	double longitude;
	double latitude;
	float altitude;
	float accuracy;
} gps_data_s;

// Function prototypes
gps_data_s getGPSData(void);

#endif /* gps_H_ */