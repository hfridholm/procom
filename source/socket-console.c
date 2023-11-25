#include "debug.h"
#include "socket.h"

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

pthread_t stdinThread;
pthread_t stdoutThread;

int serverfd = -1;
int sockfd = -1;

void* stdout_routine(void* arg)
{
  info_print("Redirecting socket -> stdout");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(socket_read(sockfd, buffer, sizeof(buffer)) > 0)
  {
    fputs(buffer, stdout);

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped socket -> stdout");

  return NULL;
}

void* stdin_routine(void* arg)
{
  info_print("Redirecting stdin -> socket");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(fgets(buffer, sizeof(buffer), stdin) != NULL)
  {
    if(socket_write(sockfd, buffer, sizeof(buffer)) == -1) break;
  
    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped stdin -> socket");

  return NULL;
}

void stdin_stdout_thread_cancel(pthread_t stdinThread, pthread_t stdoutThread)
{
  if(pthread_cancel(stdinThread != 0))
  {
    error_print("Could not cancel stdin thread");
  }
  if(pthread_cancel(stdoutThread != 0))
  {
    error_print("Could not cancel stdout thread");
  }
}

bool stdin_stdout_thread_create(pthread_t* stdinThread, pthread_t* stdoutThread)
{
  if(pthread_create(stdinThread, NULL, &stdin_routine, NULL) != 0)
  {
    error_print("Could not create stdin thread");

    return false;
  }
  if(pthread_create(stdoutThread, NULL, &stdout_routine, NULL) != 0)
  {
    error_print("Could not create stdout thread");

    return false;
  }
  return true;
}

void stdin_stdout_thread_join(pthread_t stdinThread, pthread_t stdoutThread)
{
  if(pthread_join(stdinThread, NULL) != 0)
  {
    error_print("Could not join stdin thread");
  }
  if(pthread_join(stdoutThread, NULL) != 0)
  {
    error_print("Could not join stdout thread");
  }
}

void signal_sigint_handler(int sig)
{
  error_print("Keyboard interrupt");

  // stdin_stdout_thread_cancel(stdinThread, stdoutThread);

  socket_close(&sockfd);
  socket_close(&serverfd);

  exit(1);
}

void signals_handler_setup(void)
{
  signal(SIGINT, signal_sigint_handler); // Handles SIGINT

  signal(SIGPIPE, SIG_IGN); // Ignores SIGPIPE
}

void server_process(const char address[], int port)
{
  info_print("Creating socket server");

  if(server_socket_create(&serverfd, address, port, 1))
  {
    info_print("Accepting client");

    if(socket_accept(&sockfd, serverfd, address, port))
    {
      pthread_t stdinThread, stdoutThread;

      if(stdin_stdout_thread_create(&stdinThread, &stdoutThread))
      {
        stdin_stdout_thread_join(stdinThread, stdoutThread);
      }
      socket_close(&sockfd);
    }
    socket_close(&serverfd);
  }
}

void client_process(const char address[], int port)
{
  info_print("Creating socket client");

  if(client_socket_create(&sockfd, address, port))
  {
    if(stdin_stdout_thread_create(&stdinThread, &stdoutThread))
    {
      stdin_stdout_thread_join(stdinThread, stdoutThread);
    }
    socket_close(&sockfd);
  }
}

int main(int argc, char* argv[])
{
  signals_handler_setup();

  char address[] = "127.0.0.1";
  int port = 5555;
  
  if(argc >= 2 && strcmp(argv[1], "server") == 0)
  {
    server_process(address, port);
  }
  else client_process(address, port);

  return 0;
}
