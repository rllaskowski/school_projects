#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "err.h"

#define BUFFER_SIZE 100000

void generate_rand_bytes(int n, char *msg) {
	for (int i = 0; i < n; i++) {
		msg[i] = rand();
	}
}

int main(int argc, char *argv[])
{
  int sock;
  struct addrinfo addr_hints;
  struct addrinfo *addr_result;

  int i, err;
  char buffer[BUFFER_SIZE+1];
  char msg[BUFFER_SIZE+1];
  ssize_t rcv_len;

  if (argc != 5) {
		fatal("Usage: %s host port packets_number packets_size ...\n", argv[0]);
	}

	int n = atoi(argv[3]);
  int k = atoi(argv[4]);

  if (k > BUFFER_SIZE) {
    fatal("given packets_size is greater than %d\n", BUFFER_SIZE);
  }


  // 'converting' host/port in string to struct addrinfo
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET; // IPv4
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;
  err = getaddrinfo(argv[1], argv[2], &addr_hints, &addr_result);
  if (err == EAI_SYSTEM) { // system error
    syserr("getaddrinfo: %s", gai_strerror(err));
  }
  else if (err != 0) { // other error (host not found, etc.)
    fatal("getaddrinfo: %s", gai_strerror(err));
  }

  // initialize socket according to getaddrinfo results
  sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
  if (sock < 0)
    syserr("socket");

  // connect socket to the server
  if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
    syserr("connect");

  freeaddrinfo(addr_result);

  for (i = 0; i < n; i++) {
    generate_rand_bytes(k, msg);
    msg[k] = '\0';
    printf("writing to socket %d bytes\n", k);
    if (write(sock, msg, k) != k) {
      syserr("partial / failed write");
    }

    unsigned rcv_sum = 0;
    while (rcv_sum < k) {
      memset(buffer, 0, sizeof(buffer));

      rcv_len = read(sock, buffer, sizeof(buffer) - 1);
      if (rcv_len < 0) {
        syserr("read");
      }
      rcv_sum += rcv_len;
      printf("read from socket: %zd bytes\n", rcv_len);
    }
  }

  (void) close(sock); // socket would be closed anyway when the program ends

  return 0;
}
