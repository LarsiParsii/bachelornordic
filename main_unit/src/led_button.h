#ifndef led_button_H
#define led_button_H

#include <zephyr/kernel.h>

/* Function declarations */
int led_button_init(void);
bool readButton0(void);
bool readButton1(void);
void setOnboardLed(bool state);
void setBlueLed(bool state);

extern volatile bool faux_gnss_fix_requested;
extern volatile bool mob_event;

#endif  // led_button_H