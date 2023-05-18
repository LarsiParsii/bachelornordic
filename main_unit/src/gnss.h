// gnss.h
#ifndef GNSS_H
#define GNSS_H

/* INCLUDES */
#include <zephyr/kernel.h>
#include <nrf_modem_gnss.h>
#include <dk_buttons_and_leds.h>

/* DEFINES */

/* SEMAPHORES */
extern struct k_sem gnss_fix_sem;

/* VARIABLES */
extern struct nrf_modem_gnss_pvt_data_frame current_pvt;
extern struct nrf_modem_gnss_pvt_data_frame last_pvt;
enum tracker_status
{
	status_nolte = DK_LED1,
	status_searching = DK_LED2,
	status_fixed = DK_LED3
};

/* FUNCTION PROTOTYPES */
void print_fix_data(struct nrf_modem_gnss_pvt_data_frame *pvt_data);
int gnss_init_and_start(void);
void createFauxFix(void);

#endif // GNSS_H