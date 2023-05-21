#ifndef COAP_H
#define COAP_H

/* INCLUDES */
#include <stdint.h>
#include <zephyr/net/socket.h>
#include <nrf_modem_gnss.h>

/* DEFINES */
#define APP_COAP_SEND_INTERVAL_MS 60000
#define APP_COAP_VERSION 1

/* VARIABLES */

/* FUNCTION PROTOTYPES */
int server_resolve(void);
int server_connect(int sock);
int client_handle_get_response(uint8_t *buf, int received);
int client_post_send(int sock, uint8_t *buf, size_t, uint8_t *sendbuf, size_t sendbuf_size,
							struct nrf_modem_gnss_pvt_data_frame current_pvt, bool mob_event);

#endif// COAP_H