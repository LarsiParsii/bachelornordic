#ifndef led_button_H
#define led_button_H

#include <zephyr/kernel.h>

/* Function declarations */
int led_button_init(void);
bool readButton0(void);
bool readButton1(void);
void setLed0(bool state);
void setBlueLed(bool state);

#endif  // led_button_H