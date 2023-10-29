#include "debug.h"
#include "fifo.h"

#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

int stdinFIFO = -1;
int stdoutFIFO = -1;

void* stdout_routine(void* arg)
{
  info_print("Redirecting from stdout-fifo to stdout");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(buffer_read(stdoutFIFO, buffer, sizeof(buffer)) > 0)
  {
    fputs(buffer, stdout);

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped redirecting from stdout-fifo");

  return NULL;
}

void* stdin_routine(void* arg)
{
  info_print("Redirecting from stdin to stdin-fifo");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(fgets(buffer, sizeof(buffer), stdin) != NULL)
  {
    int status = buffer_write(stdinFIFO, buffer, sizeof(buffer));

    if(status == -1) break;

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped redirecting to stdin-fifo");

  return NULL;
}

void signal_sigint_handler(int sig)
{
  error_print("Keyboard interrupt");

  stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO);

  exit(1);
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

void signals_handler_setup(void)
{
  signal(SIGINT, signal_sigint_handler); // Handles SIGINT

  signal(SIGPIPE, SIG_IGN); // Ignores SIGPIPE
}

int main(int argc, char* argv[])
{
  signals_handler_setup();

  char stdinFIFOname[64];
  char stdoutFIFOname[64];

  if(argc >= 3)
  {
    strcpy(stdinFIFOname, argv[1]);
    strcpy(stdoutFIFOname, argv[2]);
  }
  else 
  {
    strcpy(stdinFIFOname, "stdin-fifo");
    strcpy(stdoutFIFOname, "stdout-fifo");
  }

  bool openOrder = true;

  if(argc >= 4)
  {
    if(strcmp(argv[3], "reverse") == 0) openOrder = false;
  }

  fprintf(stdout, "stdinFIFO: (%s)\n", stdinFIFOname);
  fprintf(stdout, "stdoutFIFO: (%s)\n", stdoutFIFOname);
  fprintf(stdout, "openOrder: %d\n", openOrder);

  if(!stdin_stdout_fifo_open(&stdinFIFO, stdinFIFOname, &stdoutFIFO, stdoutFIFOname, openOrder)) return 1;

  pthread_t stdinThread, stdoutThread;

  if(stdin_stdout_thread_create(&stdinThread, &stdoutThread))
  {
    stdin_stdout_thread_join(stdinThread, stdoutThread);
  }

  if(!stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO)) return 2;

  return 0;
}
