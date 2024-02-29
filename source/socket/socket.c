#include "../socket.h"

/*
 * Create sockaddr from an address and a port
 *
 * RETURN
 * - struct sockaddr_in addr | The created sockaddr
 */
struct sockaddr_in sockaddr_create(int sockfd, const char address[], int port)
{
  struct sockaddr_in addr;

  if(strlen(address) == 0)
  {
    socklen_t addrlen = sizeof(addr);

    if(getsockname(sockfd, (struct sockaddr*) &addr, &addrlen) == -1)
    {
      error_print("Failed to get sock name: %s", strerror(errno));
    }
  }
  else addr.sin_addr.s_addr = inet_addr(address);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  return addr;
}

/*
 * bind, with debug messages
 *
 * RETURN (same as bind)
 * - 0  | Success!
 * - -1 | Failed to bind socket
 */
int socket_bind(int sockfd, const char address[], int port)
{
  struct sockaddr_in addr = sockaddr_create(sockfd, address, port);

  info_print("Binding socket");

  if(bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
  {
    error_print("Failed to bind socket: %s", strerror(errno));

    return -1;
  }
  
  info_print("Binded socket");

  return 0;
}

/*
 * listen, with debug messages
 *
 * RETURN (same as listen)
 * - 0  | Success!
 * - -1 | Failed to listen to socket
 */
int socket_listen(int sockfd, int backlog)
{
  info_print("Start listen to socket");

  if(listen(sockfd, backlog) == -1)
  {
    error_print("Failed to listen to socket: %s", strerror(errno));

    return -1;
  }

  info_print("Listening to socket");

  return 0;
}

/*
 * socket, with debug messages
 *
 * RETURN (same as socket)
 * - SUCCESS | File descriptor of created socket
 * - ERROR   | -1
 */
int socket_create(void)
{
  info_print("Creating socket");

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd == -1)
  {
    error_print("Failed to create socket: %s", strerror(errno));

    return -1;
  }

  info_print("Created socket (%d)", sockfd);

  return sockfd;
}

/*
 * Create a server socket, bind it and start listening for clients
 *
 * RETURN (same as socket)
 * - SUCCESS | File descriptor of created server socket
 * - ERROR   | -1
 */
int server_socket_create(const char address[], int port, int backlog)
{
  int sockfd = socket_create();

  if(sockfd == -1) return -1;

  if(socket_bind(sockfd, address, port) == -1 || socket_listen(sockfd, backlog) == -1)
  {
    socket_close(&sockfd);

    return -1;
  }
  return sockfd;
}

/*
 * connect, with debug messages
 *
 * RETURN (same as connect)
 * - 0  | Success!
 * - -1 | Failed to connect to server socket
 */
int socket_connect(int sockfd, const char address[], int port)
{
  struct sockaddr_in addr = sockaddr_create(sockfd, address, port);

  info_print("Connecting socket");

  if(connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
  {
    error_print("Failed to connect socket: %s", strerror(errno));

    return -1;
  }

  info_print("Connected socket");

  return 0;
}

/*
 * Create a client socket and connect it to the server socket
 *
 * RETURN (same as socket)
 * - SUCCESS | File descriptor of created client socket
 * - ERROR   | -1
 */
int client_socket_create(const char address[], int port)
{
  int sockfd = socket_create();

  if(sockfd == -1) return -1;

  if(socket_connect(sockfd, address, port) == -1)
  {
    socket_close(&sockfd);

    return -1;
  }
  return sockfd;
}

/*
 * accept, but with address and port, and with debug messages
 *
 * RETURN (same as accept)
 * - SUCCESS | File descriptor to accepted socket
 * - ERROR   | -1
 */
int socket_accept(int sockfd, const char address[], int port)
{
  struct sockaddr_in sockaddr = sockaddr_create(sockfd, address, port);

  int addrlen = sizeof(sockaddr);

  info_print("Accepting socket");

  int acceptfd = accept(sockfd, (struct sockaddr*) &sockaddr, (socklen_t*) &addrlen);

  if(acceptfd == -1)
  {
    error_print("Failed to accept socket: %s", strerror(errno));

    return -1;
  }

  info_print("Accepted socket (%d)", acceptfd);

  return acceptfd;
}

/*
 * close, but with pointer to file descriptor, and with debug messages
 *
 * RETURN (same as close)
 * - 0 | Success!
 * - 1 | Failed to close socket
 */
int socket_close(int* sockfd)
{
  // No need to close an already closed socket
  if(*sockfd == -1) return 0;

  info_print("Closing socket (%d)", *sockfd);

  if(close(*sockfd) == -1)
  {
    error_print("Failed to close socket: %s", strerror(errno));

    return -1;
  }
  *sockfd = -1;

  info_print("Closed socket");

  return 0;
}

/*
 * Read a single line to a buffer from a socket connection
 *
 * RETURN
 * - SUCCESS | The number of read characters
 * - ERROR   | -1
 */
int socket_read(int sockfd, char* buffer, size_t size)
{
  char symbol = '\0';
  int index;

  for(index = 0; index < size && symbol != '\n'; index++)
  {
    int status = recv(sockfd, &symbol, 1, 0);

    if(status == -1) return -1; // ERROR

    buffer[index] = symbol;

    if(status == 0) break; // END OF FILE
  }
  return index;
}

/*
 * Write a single line from a buffer to a socket connection
 *
 * RETURN
 * - SUCCESS | The number of written characters
 * - ERROR   | -1
 */
int socket_write(int sockfd, const char* buffer, size_t size)
{
  int index;
  char symbol;

  for(index = 0; index < size; index++)
  {
    symbol = buffer[index];

    int status = send(sockfd, &symbol, 1, 0);

    if(status == -1) return -1; // ERROR

    if(status == 0 || symbol == '\0' || symbol == '\n') break; // END OF FILE
  }
  return index;
}
