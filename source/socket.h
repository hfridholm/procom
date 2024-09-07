#ifndef SOCKET_H
#define SOCKET_H

#include "debug.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

extern int client_or_server_socket_create(int* sockfd, int* servfd, const char* address, int port, bool debug);

extern int socket_close(int* sockfd, bool debug);


extern ssize_t socket_write(int sockfd, const char* buffer, size_t size);

extern ssize_t socket_read(int sockfd, char* buffer, size_t size);

#endif // SOCKET_H
