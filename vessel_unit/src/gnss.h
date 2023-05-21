// gnss.h
#ifndef GNSS_H
#define GNSS_H

/* INCLUDES */
#include <zephyr/kernel.h>
#include <nrf_modem_gnss.h>
#include <dk_buttons_and_leds.h>

/* DEFINES */

/* SEMAPHORES */
extern struct k_sem sem_send_new_data;

/* VARIABLES */
extern struct nrf_modem_gnss_pvt_data_frame vessel_current_pvt;
extern struct nrf_modem_gnss_pvt_data_frame vessel_last_pvt;
extern struct nrf_modem_gnss_pvt_data_frame main_unit_current_pvt;
extern struct nrf_modem_gnss_pvt_data_frame main_unit_last_pvt;

/* FUNCTION PROTOTYPES */
void print_fix_data(struct nrf_modem_gnss_pvt_data_frame *pvt_data);
int gnss_init_and_start(void);
void createFauxFix(struct nrf_modem_gnss_pvt_data_frame *pvt_data);

#endif // GNSS_H