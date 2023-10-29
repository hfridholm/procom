#include "debug.h"
#include "socket.h"
#include "fifo.h"

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

pthread_t stdinThread;
pthread_t stdoutThread;

int serverfd = -1;
int sockfd = -1;

int stdinFIFO = -1;
int stdoutFIFO = -1;

void* stdout_routine(void* arg)
{
  info_print("Redirecting socket -> fifo");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(socket_read(sockfd, buffer, sizeof(buffer)) > 0)
  {
    fprintf(stdout, "[socket -> fifo]: %s", buffer);

    if(buffer_write(stdinFIFO, buffer, sizeof(buffer)) == -1) break;

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped socket -> fifo");

  return NULL;
}

void* stdin_routine(void* arg)
{
  info_print("Redirecting fifo -> socket");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(buffer_read(stdoutFIFO, buffer, sizeof(buffer)) > 0)
  {
    fprintf(stdout, "[fifo -> socket]: %s", buffer);

    if(socket_write(sockfd, buffer, sizeof(buffer)) == -1) break;

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped fifo -> socket");

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

  stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO);

  socket_close(&sockfd);
  socket_close(&serverfd);

  exit(1);
}

void signals_handler_setup(void)
{
  signal(SIGINT, signal_sigint_handler); // Handles SIGINT

  signal(SIGPIPE, SIG_IGN); // Ignores SIGPIPE
}

int server_process(const char address[], int port, const char stdinFIFOname[], const char stdoutFIFOname[], bool openOrder)
{
  if(!stdin_stdout_fifo_open(&stdinFIFO, stdinFIFOname, &stdoutFIFO, stdoutFIFOname, openOrder)) return 1;

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
  if(!stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO)) return 2;

  return 0;
}

int client_process(const char address[], int port, const char stdinFIFOname[], const char stdoutFIFOname[], bool openOrder)
{
  if(!stdin_stdout_fifo_open(&stdinFIFO, stdinFIFOname, &stdoutFIFO, stdoutFIFOname, openOrder)) return 1;

  info_print("Creating socket client");

  if(client_socket_create(&sockfd, address, port))
  {
    if(stdin_stdout_thread_create(&stdinThread, &stdoutThread))
    {
      stdin_stdout_thread_join(stdinThread, stdoutThread);
    }
    socket_close(&sockfd);
  }
  if(!stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO)) return 2;
  
  return 0;
}

int main(int argc, char* argv[])
{
  signals_handler_setup();

  char address[] = "";
  int port = 5555;

  char stdinFIFOname[] = "stdin-fifo";
  char stdoutFIFOname[] = "stdout-fifo";

  bool openOrder = true;
  
  if(argc >= 2 && strcmp(argv[1], "server") == 0)
  {
    return server_process(address, port, stdinFIFOname, stdoutFIFOname, openOrder);
  }
  else return client_process(address, port, stdinFIFOname, stdoutFIFOname, openOrder);
}
