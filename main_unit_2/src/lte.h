#ifndef LTE_H
#define LTE_H

/* INCLUDES */
#include <zephyr/kernel.h>
#include <modem/lte_lc.h>

/* DEFINES */
#define SEC_TAG 12

/* SEMAPHORES */
extern struct k_sem lte_connected;

/* VARIABLES */

/* FUNCTION PROTOTYPES */
void modem_configure(void);

#endif // LTE_H