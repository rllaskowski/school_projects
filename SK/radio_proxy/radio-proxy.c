/** @file
 * Internet radio proxy implementation.
 *
 * @author Robert Laskowski <robert.laskowski0@gmail.com>
 * @copyright Robert Laskowski
 * @date 30.05.2020
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#include "err.h"
#include "utils.h"

#define BSIZE 10000
#define TIMEOUT 5000
#define MAX_CLIENTS 10
#define MAX_ARGLEN 300

#define MSG_DISCOVER 1
#define MSG_IAM 2
#define MSG_ALIVE 3
#define MSG_AUDIO 4
#define MSG_META 5

#define USAGE_STR \
"usage:\n"\
"-h <host> (required)\n"\
"-p <port> (required)\n"\
"-r <resource> (default '/')\n"\
"-m <yes | no> (default 'yes')\n"\
"-t <timeout> (default 5)\n" \
"-P <port> (required to start proxy mode)\n"\
"-T <timeout> (default 5 in proxy mode)\n"\

enum parser_state {
	HEAD,
	AUDIO,
	METAB,
	META
};

struct http_head {
	char *status;
	unsigned metaint;
	char *data;
	size_t len;
};

struct parameters {
	char *i_host;
	char *i_resource;
	unsigned i_timeout;
	char *i_port;
	bool i_meta;
	unsigned p_port;
	unsigned p_timeout;
	bool proxy;
};

struct parser {
	unsigned block_left;
	enum parser_state state;
	struct http_head head;
};

struct client {
	struct sockaddr_in address;
	int64_t ack_time;
};

struct proxy {
	struct client clients[MAX_CLIENTS];
	int sock;
	unsigned timeout;
	int64_t time;
};

static bool finish = false;

static void catch_int() {
	finish = true;
}

static void set_sighandler() {
	struct sigaction action;
	sigset_t block_mask;

	sigemptyset (&block_mask);
	action.sa_handler = catch_int;
	action.sa_mask = block_mask;
	action.sa_flags = SA_RESTART;

	if (sigaction (SIGINT, &action, 0) == -1)
		syserr("sigaction");
}


static int parse_params(
		int argc,
		char *argv[],
		struct parameters *params) {
	memset(params, 0, sizeof(*params));

	/* setting default parameters */
	params->i_meta = false;
	params->i_timeout = TIMEOUT;
	params->p_timeout = TIMEOUT;
	params->proxy = false;
	int opt;

	while ((opt = getopt(argc, argv, "h:r:t:p:m:P:T:")) != -1) {
		if (!optarg) {
			return -1;
		}
		/* check for future safety */
		if (strnlen(optarg, MAX_ARGLEN) >= MAX_ARGLEN) {
			fatal("too long option argument");
		}

		switch (opt) {
			case 'h':
				params->i_host = optarg;
				break;
			case 'r':
				params->i_resource = optarg;
				break;
			case 'p':
				params->i_port = optarg;
				break;
			case 't':
				if (!parse_ui(optarg, &(params->i_timeout)) ||
					params->i_timeout == 0) {
					return -1;
				}
				params->i_timeout *= 1000;
				break;
			case 'm':
				if (strcmp(optarg, "yes") == 0) {
					params->i_meta = true;
				} else if (strcmp(optarg, "no") == 0) {
					params->i_meta = false;
				} else {
					return -1;
				}
				break;
			case 'P':
				if (!parse_ui(optarg, &(params->p_port))) {
					return -1;
				}
				params->proxy = true;
				break;
			case 'T':
				if (!parse_ui(optarg, &(params->p_timeout)) ||
					params->p_timeout == 0) {
					return -1;
				}
				params->p_timeout *= 1000;
				break;
			default:
				return -1;
		}
	}

	if (!params->i_host || !params->i_port ||
		!params->i_resource){
		return -1;
	}

	return 0;
}

static int get_isocket(char *host, char *port) {
	struct addrinfo addr_hints;
	struct addrinfo *addr_result;

	memset(&addr_hints, 0, sizeof(struct addrinfo));
	addr_hints.ai_family = AF_INET;
	addr_hints.ai_socktype = SOCK_STREAM;
	addr_hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(host, port, &addr_hints, &addr_result) != 0) {
		syserr("obtaining address info");
	}

	int sock = socket(addr_result->ai_family,
			addr_result->ai_socktype, addr_result->ai_protocol);

	if (sock < 0) {
		syserr("Creating TCP socket");
	}

	if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0) {
		syserr("Connecting to TCP socket");
	}

	return sock;
}

static int get_psocket(struct parameters *params) {
	int sock;
	struct sockaddr_in server_address;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		syserr("creating UDP socket");
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(params->p_port);

	if (bind(sock, (struct sockaddr *) &server_address,
			 (socklen_t) sizeof(server_address)) < 0) {
		syserr("binding");
	}

	return sock;
}

static size_t create_request(
		struct parameters *params,
		char **request) {
	static const char *format = "GET %s HTTP/1.0\r\n"
								"Host: %s\r\n"
								"Accept: */*\r\n"
								"Icy-MetaData: %d\r\n"
								"Connection: close\r\n"
								"\r\n";
	size_t r_len = strlen(format)+strlen(params->i_host)+
			strlen(params->i_resource);

	(*request) = calloc(r_len, sizeof(char));
	if (!(*request)) {
		syserr("allocating request string memory");
	}
	snprintf((*request), r_len, format, params->i_resource,
			params->i_host, params->i_meta);

	return r_len;
}

static int parse_httphead(
		char **buff,
		size_t *buff_len,
		struct http_head *h) {
	h->data = realloc(h->data, h->len+(*buff_len)+1);
	if (!h->data) {
		syserr("allocating head data memory");
	}
	memcpy(h->data+h->len, (*buff), (*buff_len));

	h->len += (*buff_len);
	h->data[h->len] = 0; /* for safety reasons */

	/* correct http head should end with two crlf */
	char *h_end = strstr(h->data, "\r\n\r\n");
	if (!h_end) {
		(*buff) += (*buff_len);
		(*buff_len) = 0;
		return 1;
	}
	/* whole response head has been read */
	*(h_end+2) = 0;
	str_lower(h->data);

	logd( "%.*s", (int)(h_end+4-h->data), h->data);

	char *stl_end = strstr(h->data, "\r\n");

	(*stl_end) = 0; /* separate status line from headers */
	h->status = h->data; /* head starts with status line */

	if (strcmp(h->status, "icy 200 ok") != 0 &&
		strcmp(h->status, "http/1.1 200 ok") != 0 &&
		strcmp(h->status, "http/1.0 200 ok") != 0) {
		/* invalid status line or status code is not 200 ok */
		free(h->data);
		return -1;
	}

	char *mint_h = strstr(stl_end+2, "icy-metaint:");
	if (mint_h) {
		char *mint_h_end = strstr(mint_h, "\r\n");
		(*mint_h_end) = 0;
		/* 12 is length of "icy-metaint:" string */
		char *value_str = (mint_h+12)+strspn((mint_h+12), " ");

		if (!parse_ui(value_str, &h->metaint) || value_str == 0) {
			free(h->data);
			return -1;
		}
	}

	/* how much buffer data was used to complete the head */
	unsigned buff_used = (h_end+4)-h->data-h->len+(*buff_len);
	(*buff) += buff_used;
	(*buff_len) -= buff_used;

	free(h->data);

	return 0;
}

static void send_data(
		char *data,
		uint16_t data_len,
		uint16_t type,
		struct proxy *proxy) {
	struct client *client = proxy->clients;

	memcpy(data-4, &type, 2);
	memcpy(data-2, &data_len, 2);

	socklen_t snda_len = (socklen_t) sizeof(client->address);

	for (size_t i = 0; i < MAX_CLIENTS; i++, ++client) {
		if (proxy->time-client->ack_time <= proxy->timeout) {
			sendto(proxy->sock, data-4, data_len+4, 0,
					   (struct sockaddr *) &client->address, snda_len);
		}
	}
}

static int parse_data(
		char *data,
		size_t data_left,
		struct parameters *params,
		struct parser *parser,
		struct proxy *proxy) {
	if (parser->state == HEAD) {
		int parsed = parse_httphead(&data,
						&data_left, &parser->head);
		if (parsed < 0) {
			return -1;
		} else if (parsed == 0) {
			if (!params->i_meta && parser->head.metaint) {
				/* we didn't ask for meta data */
				return -1;
			}
			/* body starts with audio data */
			parser->block_left = parser->head.metaint;
			parser->state = AUDIO;
		}
	}

	while (parser->state != HEAD && data_left > 0) {
		if (parser->head.metaint == 0) {
			parser->state = AUDIO;
			parser->block_left = data_left;
		}

		size_t block = min(parser->block_left, data_left);

		parser->block_left -= block;
		data_left -= block;

		if (parser->state == META) {
			if (proxy->sock < 0) {
				fwrite(data, sizeof(char), block, stderr);
			} else {
				logd("metadata: %.*s", block, data);
				send_data(data, block, MSG_META, proxy);
			}

			if (parser->block_left == 0) {
				parser->state = AUDIO;
				parser->block_left = parser->head.metaint;
			}
		} else if (parser->state == AUDIO) {
			if (proxy->sock < 0) {
				fwrite(data, sizeof(char), block, stdout);
			} else {
				send_data(data, block, MSG_AUDIO, proxy);
			}

			if (parser->block_left == 0) {
				parser->state = METAB;
				parser->block_left = 1;
			}
		} else if (parser->state == METAB) {
			uint8_t meta_byte = *((uint8_t *)data);
			parser->block_left = meta_byte*16;

			if (parser->block_left == 0) {
				parser->block_left = parser->head.metaint;
				parser->state = AUDIO;
			} else {
				parser->state = META;
			}
		}
		data += block;
	}

	return 0;
}

static struct client * find_client(
		struct proxy *proxy,
		struct sockaddr_in *addr) {
	struct client *client = proxy->clients;

	for (size_t i = 0; i < MAX_CLIENTS; ++i, ++client) {
		if (client->address.sin_addr.s_addr == addr->sin_addr.s_addr &&
			client->address.sin_port == addr->sin_port &&
			proxy->time-client->ack_time <= proxy->timeout) {
			return client;
		}
	}

	return NULL;
}

static struct client * find_slot(
		struct proxy *proxy) {
	struct client *client = proxy->clients;

	for (size_t i = 0; i < MAX_CLIENTS; ++i, ++client) {
		if (proxy->time-client->ack_time > proxy->timeout) {
			return client;
		}
	}

	return NULL;
}

static struct client * insert_client(
		struct sockaddr_in *addr,
		struct proxy *proxy) {
	struct client *slot = find_slot(proxy);
	if (slot) {
		memcpy(&slot->address, addr, sizeof(slot->address));
		slot->ack_time = proxy->time;

		return slot;
	}
	return NULL;
}

static void send_greeting(
		struct client *client,
		char *buffer,
		struct proxy *proxy,
		struct parameters *params) {
	char *format = "Host='%s';Port='%s';Resource='%s';Timeout='%u';";
	sprintf(buffer+4,format,
			params->i_host, params->i_port,
			params->i_resource, params->p_timeout/1000);

	uint16_t msg_type = MSG_IAM;
	memcpy(buffer, &msg_type, 2);

	uint16_t msg_len = strlen(buffer+4);
	memcpy(buffer+2, &msg_len, 2);

	socklen_t snda_len = (socklen_t) sizeof(client->address);

	sendto(proxy->sock, buffer, msg_len+1+4, 0,
		   (struct sockaddr *) &client->address, snda_len);

}

static void handle_msg(
		struct proxy *proxy,
		struct sockaddr_in *addr,
		uint16_t msg_type,
		char *buffer,
		struct parameters *params) {
	if (msg_type != MSG_ALIVE && msg_type != MSG_DISCOVER) {
		return;
	}

	struct client *client = find_client(proxy, addr);

	if (!client && msg_type == MSG_DISCOVER) {
		client = insert_client(addr, proxy);

		if (client) {
			logd("new client at slot %ld", client-proxy->clients);
			send_greeting(client, buffer, proxy, params);
		}
	} else if (client) {
		logd("client %d - ACK diff: %ld",
				client-proxy->clients,
				proxy->time-client->ack_time);
		client->ack_time = proxy->time;
	}
}

static void proxy_work(
		int i_sock,
		int p_sock,
		struct parameters *params) {
	int evt = 0, rcv = 0;
	socklen_t rcva_len;
	struct pollfd fds[2];
	struct sockaddr_in client_addr;
	char buffer[BSIZE+5];
	uint16_t msg_type;

	struct parser parser;
	memset(&parser, 0, sizeof(parser));
	parser.state = HEAD;

	struct proxy proxy;
	memset(&proxy, 0, sizeof(proxy));
	proxy.timeout = params->p_timeout;
	proxy.sock = p_sock;

	memset(fds, 0, sizeof(*fds));
	fds[0].fd = i_sock;
	fds[0].events = POLLIN;
	fds[1].fd = p_sock;
	fds[1].events = POLLIN;

	struct timeval time;
	gettimeofday(&time, NULL);

	int64_t ack_time = to_msec(&time);
	int64_t poll_time, time_left;

	while (!finish) {
		gettimeofday(&time, NULL);
		poll_time = to_msec(&time);
		time_left = (ack_time+params->i_timeout+1000)-poll_time;

		if (time_left < 0) {
			break;
		}
		evt = poll(fds, 2, time_left);
		if (evt <= 0) {
			break;
		}

		gettimeofday(&time, NULL);
		proxy.time = to_msec(&time);

		if (fds[1].revents & POLLIN) {
			fds[1].revents = 0;
			rcva_len = (socklen_t) sizeof(client_addr);

			rcv = recvfrom(fds[1].fd, buffer, 4,0,
					(struct sockaddr*)&client_addr, &rcva_len);

			if (rcv < 0) {
				break;
			}

			if (rcv >= 2) {
				memcpy(&msg_type, buffer, 2);
				handle_msg(&proxy, &client_addr, msg_type,
						buffer, params);
			}
		}

		if (fds[0].revents & POLLIN) {
			fds[0].revents = 0;
			ack_time = proxy.time;

			/* leave at least 4 bytes
			 * at the beginning of buffer for msg header */
			rcv = read(fds[0].fd, buffer+4, BSIZE);
			if (rcv <= 0) {
				break;
			}

			if (parse_data(buffer+4, rcv,
					params, &parser, &proxy) < 0) {
				fatal("parsing response");
			}
		}
	}
	if (errno != EINTR) {
		if (rcv < 0) {
			syserr("reading from socket");
		}

		if (evt < 0) {
			syserr("poll fail");
		}
	}
}

int main(int argc, char *argv[]) {
	struct parameters params;
	int i_sock, p_sock;

	if (parse_params(argc, argv, &params) < 0) {
		fprintf(stderr, USAGE_STR);
		return 1;
	}

	i_sock = get_isocket(params.i_host, params.i_port);

	char *request;
	size_t r_len = create_request(&params, &request);

	int sent = write(i_sock, request, r_len);
	free(request);

	if (sent != (int)r_len) {
		syserr("sending request");
	}

	if (params.proxy) {
		p_sock = get_psocket(&params);
	} else {
		p_sock = -1;
	}

	set_sighandler();
	proxy_work(i_sock, p_sock, &params);

	if(close(i_sock) < 0) {
		syserr("closing TCP socket");
	}

	if (p_sock != -1) {
		if (close(p_sock) < 0) {
			syserr("closing UDP socket");
		}
	}

	return 0;
}