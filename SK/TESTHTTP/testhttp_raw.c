#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_LEN 10000

typedef struct input {
	char *host;
	char *port;
	char *cookie_filename;
	char *url;
} input_t;

void error(char *msg) {
	fprintf(stderr, "ERROR: %s\n", msg);
	exit(1);
}

void parse_args(input_t *input, const int argc, char *argv[]) {
	if (argc != 4) {
		error("wrong number of arguments");
	}

	input->host = strtok(argv[1], ":");
	input->port = strtok(NULL, ":");

	if (input->port == NULL) {
		error("no port given");
	}

	input->cookie_filename = argv[2];
	input->url = argv[3];
}

int get_socket(char *host, char *port) {
	struct addrinfo addr_hints;
	struct addrinfo *addr_result;

	memset(&addr_hints, 0, sizeof(struct addrinfo));
	addr_hints.ai_family = AF_INET;
	addr_hints.ai_socktype = SOCK_STREAM;
	addr_hints.ai_protocol = IPPROTO_TCP;

	uint32_t err = getaddrinfo(host, port, &addr_hints, &addr_result);

	if (err < 0) {
		error("getaddr");
	}

	int sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);

	if (sock < 0) {
		error("can't create socket");
	}

	if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0) {
		error("cant't connect");
	}

	freeaddrinfo(addr_result);

	return sock;
}

char * get_fcontent(char *filename) {
	int file_desc = open(filename, O_RDONLY);

	if (file_desc < 0) {
		error("can't open file");
	}

	struct stat stats;
	fstat(file_desc, &stats);
	uint32_t size = stats.st_size;

	char *content = malloc(size);

	if (!content) {
		error("can't allocate file content memory");
	}

	read(file_desc, content, size);

	return content;
}

char * create_get_request(char *host, char *resource, char *cookies) {
	char *format = "GET %s HTTP/1.1\r\n"
				   "Host: %s\r\n"
				   "Connection: close\r\n"
				   "Cookies: %s\r\n\r\n";

	char *request = malloc(strlen(format)+strlen(host)+strlen(resource)+strlen(cookies));

	if (!request) {
		error("can't allocate request memory");
	}
	sprintf(request, format, resource, host, cookies);

	return request;
}

void replace_char(char *str, char from, char to) {
	for (uint32_t i = 0; str[i] != 0; ++i) {
		if (str[i] == from) {
			str[i] = to;
		}
	}
}

char * get_resource(char *url) {
	char *pos = url;

	for (uint32_t i = 0; i < 3; i++) {
		if (pos == NULL) {
			error("wrong url");
		}
		pos = strchr(pos+1, '/');
	}

	char *resource;

	if (pos == NULL) {
		resource = strdup("/");
	} else {
		resource = strdup(pos);
	}

	if (!resource) {
		error("can't allocate resource memory");
	}

	return resource;
}

int main(int argc, char *argv[]) {
	input_t input;
	char buffer[BUFFER_LEN];

	parse_args(&input, argc, argv);

	int sock = get_socket(input.host, input.port);
	char *cookies = get_fcontent(input.cookie_filename);
	replace_char(cookies, '\n', ';');

	char *resource = get_resource(input.url);

	char *request = create_get_request(input.host, resource, cookies);

	printf("%s\n", request);

	uint32_t len = strlen(request);

	if (write(sock, request, len) != len) {
		error("write");
	}

	while (true) {
		memset(buffer, 0, BUFFER_LEN);
		uint32_t rcv = read(sock, buffer, BUFFER_LEN);

		if (rcv == 0) {
			break;
		}
		printf("%.*s\n", rcv, buffer);
	}

	free(cookies);
	free(request);
	free(resource);

	close(sock);
	return 0;
}