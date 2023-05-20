#include "coap.h"
#include <stdint.h>
#include <stdio.h>
#include <nrf_modem_gnss.h>
#include <zephyr/net/coap.h>
#include <zephyr/net/socket.h>
#include <zephyr/random/rand32.h>
#include <zephyr/net/tls_credentials.h>

LOG_MODULE_REGISTER(CoAP, LOG_LEVEL_DBG);

/* VARIABLES */
static uint16_t next_token;
static struct sockaddr_storage server;

/* FUNCTION DEFINITIONS */

/**@brief Resolves the configured hostname. */
int server_resolve(void)
{
	int err;
	struct addrinfo *result;
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM};
	char ipv4_addr[NET_IPV4_ADDR_LEN];

	err = getaddrinfo(CONFIG_COAP_SERVER_HOSTNAME, NULL, &hints, &result);
	if (err != 0)
	{
		LOG_ERR("ERROR: getaddrinfo failed %d\n", err);
		return -EIO;
	}

	if (result == NULL)
	{
		LOG_ERR("ERROR: Address not found\n");
		return -ENOENT;
	}

	/* IPv4 Address. */
	struct sockaddr_in *server4 = ((struct sockaddr_in *)&server);

	server4->sin_addr.s_addr =
		((struct sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
	server4->sin_family = AF_INET;
	server4->sin_port = htons(CONFIG_COAP_SERVER_PORT);

	inet_ntop(AF_INET, &server4->sin_addr.s_addr, ipv4_addr,
			  sizeof(ipv4_addr));
	LOG_INF("IPv4 Address found %s\n", ipv4_addr);

	/* Free the address. */
	freeaddrinfo(result);

	return 0;
}

/**@brief Initialize the CoAP client */
int server_connect(int sock)
{
	int err;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_DTLS_1_2);
	if (sock < 0)
	{
		LOG_ERR("Failed to create CoAP socket: %d.\n", errno);
		return -errno;
	}

	int verify;
	sec_tag_t sec_tag_list[] = {12};

	enum
	{
		NONE = 0,
		OPTIONAL = 1,
		REQUIRED = 2,
	};

	verify = REQUIRED;

	err = setsockopt(sock, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));
	if (err)
	{
		LOG_ERR("Failed to setup peer verification, errno %d\n", errno);
		return -errno;
	}

	err = setsockopt(sock, SOL_TLS, TLS_HOSTNAME, CONFIG_COAP_SERVER_HOSTNAME,
					 strlen(CONFIG_COAP_SERVER_HOSTNAME));
	if (err)
	{
		LOG_ERR("Failed to setup TLS hostname (%s), errno %d\n",
				CONFIG_COAP_SERVER_HOSTNAME, errno);
		return -errno;
	}

	err = setsockopt(sock, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_list,
					 sizeof(sec_tag_t) * ARRAY_SIZE(sec_tag_list));
	if (err)
	{
		LOG_ERR("Failed to setup socket security tag, errno %d\n", errno);
		return -errno;
	}

	err = connect(sock, (struct sockaddr *)&server,
				  sizeof(struct sockaddr_in));
	if (err < 0)
	{
		LOG_ERR("Connect failed : %d\n", errno);
		return -errno;
	}

	/* Randomize token. */
	next_token = sys_rand32_get();

	return 0;
}

/**@brief Handles responses from the remote CoAP server. */
int client_handle_get_response(uint8_t *buf, int received)
{
	int err;
	struct coap_packet reply;
	const uint8_t *payload;
	uint16_t payload_len;
	uint8_t token[8];
	uint16_t token_len;
	static uint8_t temp_buf[128];

	err = coap_packet_parse(&reply, buf, received, NULL, 0);
	if (err < 0)
	{
		LOG_ERR("Malformed response received: %d\n", err);
		return err;
	}

	payload = coap_packet_get_payload(&reply, &payload_len);
	token_len = coap_header_get_token(&reply, token);

	if ((token_len != sizeof(next_token)) ||
		(memcmp(&next_token, token, sizeof(next_token)) != 0))
	{
		LOG_ERR("Invalid token received: 0x%02x%02x\n",
				token[1], token[0]);
		return 0;
	}

	if (payload_len > 0)
	{
		snprintf(temp_buf, MIN(payload_len + 1, sizeof(temp_buf)), "%s", payload);
	}
	else
	{
		strcpy(temp_buf, "EMPTY");
	}

	LOG_INF("CoAP response: Code 0x%x, Token 0x%02x%02x, Payload: %s",
			coap_header_get_code(&reply), token[1], token[0], temp_buf);
	return 0;
}

int client_post_send(int sock, uint8_t *buf, size_t buf_size, uint8_t *sendbuf, size_t sendbuf_size,
							struct nrf_modem_gnss_pvt_data_frame current_pvt, bool mob_value)
{
	int err, ret;
	struct coap_packet request;

	next_token++;

	err = coap_packet_init(&request, buf, buf_size,
						   APP_COAP_VERSION, COAP_TYPE_CON,
						   sizeof(next_token), (uint8_t *)&next_token,
						   COAP_METHOD_POST, coap_next_id());
	if (err < 0)
	{
		LOG_ERR("Failed to create CoAP request, %d\n", err);
		return err;
	}

	err = coap_packet_append_option(&request, COAP_OPTION_URI_PATH,
									(uint8_t *)CONFIG_COAP_POST_RESOURCE,
									strlen(CONFIG_COAP_POST_RESOURCE));
	if (err < 0)
	{
		LOG_ERR("Failed to encode CoAP option, %d\n", err);
		return err;
	}

	err = coap_append_option_int(&request, COAP_OPTION_CONTENT_FORMAT,
								 COAP_CONTENT_FORMAT_TEXT_PLAIN);
	if (err < 0)
	{
		LOG_ERR("Failed to encode CoAP CONTENT_FORMAT option, %d", err);
		return err;
	}

	err = coap_packet_append_option(&request, COAP_OPTION_URI_QUERY,
									(uint8_t *)"keep",
									strlen("keep"));
	if (err < 0)
	{
		LOG_ERR("Failed to encode CoAP URI-QUERY option 'keep', %d", err);
		return err;
	}

	err = coap_packet_append_payload_marker(&request);
	if (err < 0)
	{
		LOG_ERR("Failed to append payload marker, %d\n", err);
		return err;
	}

	ret = snprintf(sendbuf, sendbuf_size, "%.06f,%.06f\n%.01f m\n%04u-%02u-%02u %02u:%02u:%02u\n%d",
				   current_pvt.latitude, current_pvt.longitude, current_pvt.accuracy, current_pvt.datetime.year, current_pvt.datetime.month, current_pvt.datetime.day,
				   current_pvt.datetime.hour, current_pvt.datetime.minute, current_pvt.datetime.seconds, mob_value);
	if (err < 0)
	{
		LOG_ERR("snprintf failed to format string, %d\n", err);
		return err;
	}
	err = coap_packet_append_payload(&request, (uint8_t *)sendbuf, ret);
	if (err < 0)
	{
		LOG_ERR("Failed to append payload, %d\n", err);
		return err;
	}

	err = send(sock, request.data, request.offset, 0);
	if (err < 0)
	{
		LOG_ERR("Failed to send CoAP request, %d\n", errno);
		return -errno;
	}

	LOG_INF("CoAP request sent: token 0x%04x\n", next_token);

	return 0;
}