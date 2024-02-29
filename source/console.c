#include "debug.h"

#include <stdlib.h>
#include <signal.h>
#include <string.h>

bool debug = false;

void sigint_handler(int signum)
{
  if(debug) info_print("Keyboard interrupt");

  exit(1); // Exits the program with status 1
}

void sigpipe_handler(int signum)
{
  if(debug) error_print("Pipe has been broken");

  exit(2); // Exits the program with status 2
}

void sigint_handler_setup(void)
{
  struct sigaction sigAction;

  sigAction.sa_handler = sigint_handler;
  sigAction.sa_flags = 0;
  sigemptyset(&sigAction.sa_mask);

  sigaction(SIGINT, &sigAction, NULL);
}

void sigpipe_handler_setup(void)
{
  struct sigaction sigAction;

  sigAction.sa_handler = sigpipe_handler;
  sigAction.sa_flags = 0;
  sigemptyset(&sigAction.sa_mask);

  sigaction(SIGPIPE, &sigAction, NULL);
}

void signals_handler_setup(void)
{
  sigint_handler_setup();

  sigpipe_handler_setup();
}

int main(int argc, char* argv[])
{
  signals_handler_setup();

  debug = true;

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(fgets(buffer, sizeof(buffer), stdin) != NULL)
  {
    fputs(buffer, stdout);
    
    memset(buffer, '\0', sizeof(buffer));
  }
  if(debug) info_print("Input pipe interrupted");

  return 0; // Exits the program with status 0
}
