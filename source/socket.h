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

// https://man7.org/linux/man-pages/man2/recv.2.html 
// https://www.man7.org/linux/man-pages/man2/send.2.html 
// https://www.man7.org/linux/man-pages/man2/socket.2.html 
// https://www.man7.org/linux/man-pages/man2/bind.2.html 
// https://www.man7.org/linux/man-pages/man2/listen.2.html 
// https://man7.org/linux/man-pages/man2/connect.2.html 
// https://www.man7.org/linux/man-pages/man2/accept.2.html 
// https://www.man7.org/linux/man-pages/man2/close.2.html 
// https://man7.org/linux/man-pages/man2/getsockname.2.html

extern bool server_socket_create(int* sockfd, const char address[], int port, int backlog);

extern bool client_socket_create(int* sockfd, const char address[], int port);

extern bool socket_accept(int* acceptfd, int sockfd, const char address[], int port);

extern bool socket_close(int* sockfd);

extern int socket_write(int sockfd, const char* buffer, size_t size);

extern int socket_read(int sockfd, char* buffer, size_t size);

#endif // SOCKET_H
