#include "debug.h"

#include <stdlib.h>
#include <signal.h>
#include <string.h>

// Settings
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

/*
 * Parse the current passed flag
 *
 * FLAGS
 * --debug | Output debug messages
 */
void flag_parse(char flag[])
{
  if(!strcmp(flag, "--debug"))
  {
    debug = true;
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
