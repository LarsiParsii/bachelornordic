// gnss.h
#ifndef GNSS_H
#define GNSS_H

/* INCLUDES */
#include <zephyr/kernel.h>
#include <nrf_modem_gnss.h>

/* DEFINES */

/* SEMAPHORES */
extern struct k_sem sem_send_new_data;

/* VARIABLES */
extern struct nrf_modem_gnss_pvt_data_frame current_pvt;
extern struct nrf_modem_gnss_pvt_data_frame last_pvt;

/* FUNCTION PROTOTYPES */
void print_fix_data(struct nrf_modem_gnss_pvt_data_frame *pvt_data);
int gnss_init_and_start(void);
void createFauxFix(void);

#endif // GNSS_H