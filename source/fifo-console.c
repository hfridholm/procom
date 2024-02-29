#include "debug.h"
#include "fifo.h"

#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

// SIGUSR1 - interrupting threads

pthread_t stdinThread;
pthread_t stdoutThread;

int stdinFIFO = -1;
int stdoutFIFO = -1;

bool debug = false;

void* stdout_routine(void* arg)
{
  info_print("Redirecting fifo -> stdout");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  int status;

  while((status = buffer_read(stdoutFIFO, buffer, sizeof(buffer))) > 0)
  {
    fputs(buffer, stdout);

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped fifo -> stdout");

  // If stdout thread has interrupted stdin thread
  if(status == -1 && errno == EINTR)
  {
    info_print("stdin routine interrupted"); 
  }

  // Interrupt stdout thread
  pthread_kill(stdoutThread, SIGUSR1);

  return NULL;
}

void* stdin_routine(void* arg)
{
  info_print("Redirecting stdin -> fifo");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(fgets(buffer, sizeof(buffer), stdin) != NULL)
  {
    if(buffer_write(stdinFIFO, buffer, sizeof(buffer)) == -1) break;

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped stdin -> fifo");

  // If stdout thread has interrupted stdin thread
  if(errno == EINTR)
  {
    info_print("stdin routine interrupted"); 
  }

  // Interrupt stdout thread
  pthread_kill(stdoutThread, SIGUSR1);

  return NULL;
}

/*
 * Create stdin and stdout threads
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create stdin thread
 * - 2 | Failed to create stdout thread
 */
int stdin_stdout_thread_create(pthread_t* stdinThread, pthread_t* stdoutThread)
{
  if(pthread_create(stdinThread, NULL, &stdin_routine, NULL) != 0)
  {
    error_print("Failed to create stdin thread");

    return 1;
  }
  if(pthread_create(stdoutThread, NULL, &stdout_routine, NULL) != 0)
  {
    error_print("Failed to create stdout thread");

    // Interrupt stdin thread
    pthread_kill(*stdinThread, SIGUSR1);

    return 2;
  }
  return 0;
}

/*
 * Join stdin and stdout threads
 */
void stdin_stdout_thread_join(pthread_t stdinThread, pthread_t stdoutThread)
{
  if(pthread_join(stdinThread, NULL) != 0)
  {
    error_print("Failed to join stdin thread");
  }
  if(pthread_join(stdoutThread, NULL) != 0)
  {
    error_print("Failed to join stdout thread");
  }
}

// This is executed when the user interrupts the program
// - interrupt and stop the threads
// - close stdin and stdout FIFOs
// - exit the program
void sigint_handler(int signum)
{
  info_print("Keyboard interrupt");

  stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO, debug);

  exit(1); // Exits the program with status 1
}

void sigint_handler_setup(void)
{
  struct sigaction sigAction;

  sigAction.sa_handler = sigint_handler;
  sigAction.sa_flags = 0;
  sigemptyset(&sigAction.sa_mask);

  sigaction(SIGINT, &sigAction, NULL);
}

void sigusr1_handler(int signum) {}

void sigusr1_handler_setup(void)
{
  struct sigaction sigAction;

  sigAction.sa_handler = sigusr1_handler;
  sigAction.sa_flags = 0;
  sigemptyset(&sigAction.sa_mask);

  sigaction(SIGUSR1, &sigAction, NULL);
}

void signals_handler_setup(void)
{
  signal(SIGPIPE, SIG_IGN); // Ignores SIGPIPE
  
  sigint_handler_setup();

  sigusr1_handler_setup();
}

/*
 * Start stdin and stdout thread
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create stdin and stdout threads
 */
int stdin_stdout_thread_start(pthread_t* stdinThread, pthread_t* stdoutThread)
{
  if(stdin_stdout_thread_create(stdinThread, stdoutThread) != 0) return 1;
  
  stdin_stdout_thread_join(*stdinThread, *stdoutThread);

  return 0;
}

int console_process(const char stdinFIFOname[], const char stdoutFIFOname[], bool reversed)
{
  if(stdin_stdout_fifo_open(&stdinFIFO, stdinFIFOname, &stdoutFIFO, stdoutFIFOname, reversed, debug) != 0) return 1;

  int status = stdin_stdout_thread_start(&stdinThread, &stdoutThread);

  if(stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO, debug) != 0) return 2;

  return (status != 0) ? 3 : 0;
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
    strcpy(stdinFIFOname, "stdin");
    strcpy(stdoutFIFOname, "stdout");
  }

  bool reversed = false;

  if(argc >= 4)
  {
    if(strcmp(argv[3], "reverse") == 0) reversed = true;
  }

  return console_process(stdinFIFOname, stdoutFIFOname, reversed);
}
