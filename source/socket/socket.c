#include "../socket.h"

struct sockaddr_in sockaddr_create(int sockfd, const char address[], int port)
{
  struct sockaddr_in addr;

  if(strlen(address) == 0)
  {
    socklen_t addrlen = sizeof(addr);

    if(getsockname(sockfd, (struct sockaddr*) &addr, &addrlen) == -1)
    {
      error_print("Could not get sock name: %s", strerror(errno));
    }
  }
  else addr.sin_addr.s_addr = inet_addr(address);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  return addr;
}

bool socket_bind(int sockfd, const char address[], int port)
{
  struct sockaddr_in addr = sockaddr_create(sockfd, address, port);

  if(bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
  {
    error_print("Could not bind socket: %s", strerror(errno));

    return false;
  }
  return true;
}

bool socket_listen(int sockfd, int backlog)
{
  if(listen(sockfd, backlog) == -1)
  {
    error_print("Could not listen: %s", strerror(errno));

    return false;
  }
  return true;
}

bool socket_create(int* sockfd)
{
  if((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    error_print("Could not create socket: %s", strerror(errno));

    return false;
  }
  return true;
}

bool server_socket_create(int* sockfd, const char address[], int port, int backlog)
{
  if(!socket_create(sockfd)) return false;

  if(!socket_bind(*sockfd, address, port) || !socket_listen(*sockfd, backlog))
  {
    close(*sockfd);

    return false;
  }
  return true;
}

bool socket_connect(int sockfd, const char address[], int port)
{
  struct sockaddr_in addr = sockaddr_create(sockfd, address, port);

  if(connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
  {
    error_print("Could not connect: %s", strerror(errno));

    return false;
  }
  return true;
}

bool client_socket_create(int* sockfd, const char address[], int port)
{
  if(!socket_create(sockfd)) return false;

  if(!socket_connect(*sockfd, address, port))
  {
    close(*sockfd);

    return false;
  }
  return true;
}

bool socket_accept(int* acceptfd, int sockfd, const char address[], int port)
{
  struct sockaddr_in sockaddr = sockaddr_create(sockfd, address, port);

  int addrlen = sizeof(sockaddr);

  if((*acceptfd = accept(sockfd, (struct sockaddr*) &sockaddr, (socklen_t*) &addrlen)) == -1)
  {
    error_print("Could not accept: %s", strerror(errno));

    return false;
  }
  return true;
}

bool socket_close(int* sockfd)
{
  if(*sockfd == -1) return true;

  info_print("Closing socket");

  if(close(*sockfd) == -1)
  {
    error_print("Could not close socket: %s", strerror(errno));

    return false;
  }
  *sockfd = -1;

  return true;
}

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

int socket_write(int sockfd, const char* buffer, size_t size)
{
  int index;
  char symbol;

  for(index = 0; index < size; index++)
  {
    symbol = buffer[index];

    int status = send(sockfd, &symbol, 1, 0);

    if(status == -1) return -1;
    if(status == 0) break;

    if(symbol == '\0' || symbol == '\n') break;
  }
  return index;
}
