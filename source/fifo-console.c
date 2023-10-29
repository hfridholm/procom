#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include <signal.h>

int stdinFIFO = -1;
int stdoutFIFO = -1;

void error_print(char* format, ...)
{
  fprintf(stderr, "[ ERROR ]: %s\n", format);
}

void info_print(char* format, ...)
{
  fprintf(stdout, "[ INFO ]: %s\n", format);
}

bool stdin_fifo_open(int* stdinFIFO)
{
  info_print("Opening stdin-fifo");

  *stdinFIFO = open("stdin-fifo", O_WRONLY);

  if(*stdinFIFO == -1)
  {
    error_print("Could not open stdin-fifo");
    
    return false;
  }
  return true;
}

bool stdout_fifo_open(int* stdoutFIFO)
{
  info_print("Opening stdout-fifo");

  *stdoutFIFO = open("stdout-fifo", O_RDONLY);
  
  if(*stdoutFIFO == -1)
  {
    error_print("Could not open stdout-fifo");

    return false;
  }
  return true;
}

bool stdin_fifo_close(int* stdinFIFO)
{
  info_print("Closing stdin-fifo");

  if(close(*stdinFIFO) == -1)
  {
    error_print("Could not close stdin-fifo: %s", strerror(errno));

    return false;
  }
  *stdinFIFO = -1;

  return true;
}

bool stdout_fifo_close(int* stdoutFIFO)
{
  info_print("Closing stdout-fifo");

  if(close(*stdoutFIFO) == -1)
  {
    error_print("Could not close stdout-fifo: %s", strerror(errno));

    return false;
  }
  *stdoutFIFO = -1;

  return true;
}

bool stdin_stdout_fifo_open(int* stdinFIFO, int* stdoutFIFO)
{
  if(!stdin_fifo_open(stdinFIFO)) return false;

  if(!stdout_fifo_open(stdoutFIFO))
  {
    stdin_fifo_close(stdinFIFO);

    return false;
  }
  return true;
}

bool stdin_stdout_fifo_close(int* stdinFIFO, int* stdoutFIFO)
{
  if(!stdin_fifo_close(stdinFIFO)) return false;

  if(!stdout_fifo_close(stdoutFIFO)) return false;

  return true;
}

int buffer_read(int fd, char* buffer, size_t size)
{
  char symbol = '\0';
  int index;

  for(index = 0; index < size && symbol != '\n'; index++)
  {
    int status = read(fd, &symbol, 1);

    if(status == -1) return -1; // ERROR

    buffer[index] = symbol;

    if(status == 0) break; // END OF FILE
  }
  return index;
}

int buffer_write(int fd, const char* buffer, size_t size)
{
  int index;
  char symbol;

  for(index = 0; index < size; index++)
  {
    symbol = buffer[index];

    int status = write(fd, &symbol, 1);

    if(status == -1) return -1;
    if(status == 0) break;

    if(symbol == '\0' || symbol == '\n') break;
  }
  return index;
}

void* stdout_routine(void* arg)
{
  info_print("Redirecting from stdout-fifo to stdout");

  char buffer[1024];

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

  while(fgets(buffer, sizeof(buffer), stdin) != NULL)
  {
    int status = buffer_write(stdinFIFO, buffer, sizeof(buffer));

    if(status == -1) break;
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

  if(!stdin_stdout_fifo_open(&stdinFIFO, &stdoutFIFO)) return 1;

  pthread_t stdinThread, stdoutThread;

  if(stdin_stdout_thread_create(&stdinThread, &stdoutThread))
  {
    stdin_stdout_thread_join(stdinThread, stdoutThread);
  }

  if(!stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO)) return 2;

  return 0;
}
