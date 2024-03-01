#include "debug.h"
#include "fifo.h"
#include "thread.h"

#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

// SIGUSR1 - interrupting threads

pthread_t stdinThread;
pthread_t stdoutThread;

bool stdinThreadRunning = false;
bool stdoutThreadRunning = false;

int stdinFIFO = -1;
int stdoutFIFO = -1;

// Settings
bool debug = false;
bool reversed = false;

char stdinPathname[64] = "stdin";
char stdoutPathname[64] = "stdout";

void* stdout_routine(void* arg)
{
  stdoutThreadRunning = true;

  if(debug) info_print("Redirecting fifo -> stdout");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  int status;

  while((status = buffer_read(stdoutFIFO, buffer, sizeof(buffer))) > 0)
  {
    fputs(buffer, stdout);

    memset(buffer, '\0', sizeof(buffer));
  }
  if(debug) info_print("Stopped fifo -> stdout");

  // If stdin thread has interrupted stdout thread
  if(status == -1 && errno == EINTR)
  {
    if(debug) info_print("stdin routine interrupted"); 
  }

  // Interrupt stdin thread
  pthread_kill(stdinThread, SIGUSR1);

  stdoutThreadRunning = false;

  return NULL;
}

void* stdin_routine(void* arg)
{
  stdinThreadRunning = true;

  if(debug) info_print("Redirecting stdin -> fifo");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(fgets(buffer, sizeof(buffer), stdin) != NULL)
  {
    if(buffer_write(stdinFIFO, buffer, sizeof(buffer)) == -1) break;

    memset(buffer, '\0', sizeof(buffer));
  }
  if(debug) info_print("Stopped stdin -> fifo");

  // If stdout thread has interrupted stdin thread
  if(errno == EINTR)
  {
    if(debug) info_print("stdin routine interrupted"); 
  }

  // Interrupt stdout thread
  pthread_kill(stdoutThread, SIGUSR1);

  stdinThreadRunning = false;

  return NULL;
}

/*
 * Keyboard interrupt - close the program (the threads)
 */
void sigint_handler(int signum)
{
  if(debug) info_print("Keyboard interrupt");

  if(stdinThreadRunning) pthread_kill(stdinThread, SIGUSR1);
  if(stdoutThreadRunning) pthread_kill(stdoutThread, SIGUSR1);
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

int console_process(void)
{
  if(stdin_stdout_fifo_open(&stdinFIFO, stdinPathname, &stdoutFIFO, stdoutPathname, reversed, debug) != 0) return 1;

  int status = stdin_stdout_thread_start(&stdinThread, &stdin_routine, &stdoutThread, &stdout_routine, debug);

  if(stdin_stdout_fifo_close(&stdinFIFO, &stdoutFIFO, debug) != 0) return 2;

  return (status != 0) ? 3 : 0;
}

/*
 * Parse the current passed flag
 *
 * FLAGS
 * --debug         | Output debug messages
 * --reversed      | Open stdout FIFO before stdin FIFO
 * --stdin=<name>  | The name of stdin FIFO
 * --stdout=<name> | The name of stdout FIFO
 */
void flag_parse(char flag[])
{
  if(!strcmp(flag, "--debug"))
  {
    debug = true;
  }
  else if(!strcmp(flag, "--reversed"))
  {
    reversed = true;
  }
  else if(!strncmp(flag, "--stdin=", 8))
  {
    strcpy(stdinPathname, flag + 8);
  }
  else if(!strncmp(flag, "--stdout=", 9))
  {
    strcpy(stdoutPathname, flag + 9);
  }
}

/*
 * Parse every passed flag
 */
void flags_parse(int argc, char* argv[])
{
  for(int index = 1; index < argc; index += 1)
  {
    flag_parse(argv[index]);
  }
}

int main(int argc, char* argv[])
{
  flags_parse(argc, argv);

  signals_handler_setup();

  return console_process();
}
