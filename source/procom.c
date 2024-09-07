/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-09-07
 */

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT    5555

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>
#include <signal.h>

#include "debug.h"
#include "fifo.h"
#include "socket.h"
#include "thread.h"

pthread_t stdin_thread;
bool      stdin_running = false;

pthread_t stdout_thread;
bool      stdout_running = false;

int sockfd = -1;
int servfd = -1;

bool fifo_reverse = false;

int stdin_fifo  = -1;
int stdout_fifo = -1;

static char doc[] = "procom - process communication";

static char args_doc[] = "";

static struct argp_option options[] =
{
  { "stdin",   'i', "FIFO",    0, "Stdin FIFO" },
  { "stdout",  'o', "FIFO",    0, "Stdout FIFO" },
  { "address", 'a', "ADDRESS", 0, "Network address" },
  { "port",    'p', "PORT",    0, "Network port" },
  { "debug",   'd', 0,         0, "Print debug messages" },
  { 0 }
};

struct args
{
  char*  stdin_path;
  char*  stdout_path;
  char*  address;
  int    port;
  bool   debug;
};

struct args args =
{
  .stdin_path  = NULL,
  .stdout_path = NULL,
  .address     = NULL,
  .port        = -1,
  .debug       = false
};

/*
 * This is the option parsing function used by argp
 */
static error_t opt_parse(int key, char* arg, struct argp_state* state)
{
  struct args* args = state->input;

  switch(key)
  {
    case 'i':
      // If the output fifo has already been inputted,
      // open the output fifo before the input fifo
      if(args->stdout_path) fifo_reverse = true;

      args->stdin_path = arg;
      break;

    case 'o':
      args->stdout_path = arg;
      break;

    case 'a':
      args->address = arg;
      break;

    case 'p':
      int port = atoi(arg);

      if(port != 0) args->port = port;
      break;

    case 'd':
      args->debug = true;
      break;

    case ARGP_KEY_ARG:
      break;

    case ARGP_KEY_END:
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

/*
 * The stdin thread takes input from either terminal or fifo
 */
static ssize_t stdin_thread_read(char* buffer, size_t size)
{
  // 1. If both stdin fifo AND socket are connected, read from stdin fifo
  if(stdin_fifo != -1 && sockfd != -1)
  {
    return buffer_read(stdin_fifo, buffer, size);
  }
  // 2. If not both stdin fifo AND socket are connected, read from stdin
  else
  {
    return buffer_read(0, buffer, size);
  }
}

/*
 * The stdin thread writes to either fifo, socket or terminal
 */
static ssize_t stdin_thread_write(const char* buffer, size_t size)
{
  // 1. If both stdin fifo and socket are connected, write to socket
  if(stdin_fifo != -1 && sockfd != -1)
  {
    if(args.debug) info_print("Forwared message [stdin fifo] => [socket]", buffer);

    return socket_write(sockfd, buffer, size);
  }
  // 2. If stdout fifo, but not socket, is connected, write to stdout fifo
  else if(stdout_fifo != -1)
  {
    return buffer_write(stdout_fifo, buffer, size);
  }
  // 3. If socket, but not stdout fifo, is connected, write to socket
  else if(sockfd != -1)
  {
    return socket_write(sockfd, buffer, size);
  }
  // 4. If neither stdout fifo nor socket are connected, write to stdout
  else
  {
    return buffer_write(1, buffer, size);
  }
} 

/*
 * The stdout thread takes input from either fifo, socket
 * If neither fifo nor socket is connected, nothing is done
 */
static ssize_t stdout_thread_read(char* buffer, size_t size)
{
  // 1. If both stdin fifo and socket are connected, read from socket
  if(stdin_fifo != -1 && sockfd != -1)
  {
    return socket_read(sockfd, buffer, size);
  }
  // 2. If socket, but not stdin fifo, is connected, read from socket
  else if(sockfd != -1)
  {
    return socket_read(sockfd, buffer, size);
  }
  // 3. If stdin fifo, but not socket, is connected, read from stdin fifo
  else if(stdin_fifo != -1)
  {
    return buffer_read(stdin_fifo, buffer, size);
  }
  // 4. If neither stdin fifo nor socket are connected, stdout thread should not be running
  else return -1;
}

/*
 * The stdout thread writes to either fifo or terminal
 */
static ssize_t stdout_thread_write(const char* buffer, size_t size)
{
  // 1. If both stdout fifo and socket are connected, write to stdout fifo
  if(stdout_fifo != -1 && sockfd != -1)
  {
    if(args.debug) info_print("Forwared message [socket] => [stdout fifo]", buffer);

    return buffer_write(stdout_fifo, buffer, size);
  }
  // 2. Else, write to stdout
  else
  {
    return buffer_write(1, buffer, size);
  }
} 

/*
 *
 */
void* stdout_routine(void* arg)
{
  // No need for a recieving routine if neither fifo nor socket is connected
  if(stdin_fifo == -1 && sockfd == -1) return NULL;

  if(args.debug) info_print("Start of stdout routine");

  stdout_running = true;

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  int read_status  = -1;
  int write_status = -1;

  while((read_status = stdout_thread_read(buffer, sizeof(buffer) - 1)) > 0)
  {
    if((write_status = stdout_thread_write(buffer, sizeof(buffer) - 1)) <= 0) break;

    memset(buffer, '\0', sizeof(buffer));
  }

  if(errno != 0)
  {
    if(args.debug) error_print("%s", strerror(errno));
  }

  if(stdin_running)
  {
    if(args.debug) info_print("Interrupting stdin routine");

    pthread_kill(stdin_thread, SIGUSR1);
  }

  stdout_running = false;

  if(args.debug) info_print("End of stdout routine");

  return NULL;
}

/*
 *
 */
void* stdin_routine(void* arg)
{
  // No need for an inputting end, if ONLY stdin fifo is connected
  if(stdin_fifo != -1 && sockfd == -1 && stdout_fifo == -1) return NULL;

  if(args.debug) info_print("Start of stdin routine");

  stdin_running = true;

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  int read_status  = -1;
  int write_status = -1;

  while((read_status = stdin_thread_read(buffer, sizeof(buffer) - 1)) > 0)
  {
    if((write_status = stdin_thread_write(buffer, sizeof(buffer) - 1)) <= 0) break;

    memset(buffer, '\0', sizeof(buffer));
  }

  if(errno != 0)
  {
    if(args.debug) error_print("%s", strerror(errno));
  }

  if(stdout_running)
  {
    if(args.debug) info_print("Interrupting stdout routine");

    pthread_kill(stdout_thread, SIGUSR1);
  }

  stdin_running = false;

  if(args.debug) info_print("End of stdin routine");

  return NULL;
}

// Try to remove this
static void fifos_socket_close(void)
{
  stdin_stdout_fifo_close(&stdin_fifo, &stdout_fifo, args.debug);

  socket_close(&sockfd, args.debug);

  socket_close(&servfd, args.debug);
}

/*
 * Keyboard interrupt - close the program (the threads)
 */
static void sigint_handler(int signum)
{
  if(args.debug) info_print("Keyboard interrupt");

  fifos_socket_close();

  // Maybe remove these if statements
  if(stdin_running)  pthread_kill(stdin_thread, SIGUSR1);

  if(stdout_running) pthread_kill(stdout_thread, SIGUSR1);
}

/*
 * Broken pipe - close the program (the threads)
 */
static void sigpipe_handler(int signum)
{
  if(args.debug) error_print("Pipe has been broken");

  fifos_socket_close();

  if(stdin_running)  pthread_kill(stdin_thread, SIGUSR1);

  if(stdout_running) pthread_kill(stdout_thread, SIGUSR1);
}

/*
 *
 */
static void sigusr1_handler(int signum)
{
  fifos_socket_close();
}

/*
 *
 */
static void sigint_handler_setup(void)
{
  struct sigaction sig_action;

  sig_action.sa_handler = sigint_handler;
  sig_action.sa_flags = 0;
  sigemptyset(&sig_action.sa_mask);

  sigaction(SIGINT, &sig_action, NULL);
}

/*
 *
 */
static void sigpipe_handler_setup(void)
{
  struct sigaction sig_action;

  sig_action.sa_handler = sigpipe_handler;
  sig_action.sa_flags = 0;
  sigemptyset(&sig_action.sa_mask);

  sigaction(SIGPIPE, &sig_action, NULL);
}

/*
 *
 */
static void sigusr1_handler_setup(void)
{
  struct sigaction sig_action;

  sig_action.sa_handler = sigusr1_handler;
  sig_action.sa_flags = 0;
  sigemptyset(&sig_action.sa_mask);

  sigaction(SIGUSR1, &sig_action, NULL);
}

/*
 *
 */
static void signals_handler_setup(void)
{
  sigpipe_handler_setup();
  
  sigint_handler_setup();

  sigusr1_handler_setup();
}

/*
 *
 * If either an address or a port has been inputted,
 * the program should connect to a socket
 *
 * RETURN (same as client_or_server_socket_create)
 * - 0 | Success
 * - 1 | Failed to create socket
 *
 * Note: Success can be omitted, without a socket being created
 */
static int args_socket_create(void)
{
  if(!args.address && args.port == -1) return 0;

  if(!args.address) args.address = DEFAULT_ADDRESS;

  if(args.port == -1) args.port = DEFAULT_PORT;

  return client_or_server_socket_create(&sockfd, &servfd, args.address, args.port, args.debug);
}

static struct argp argp = { options, opt_parse, args_doc, doc };

/*
 * This is the main function
 */
int main(int argc, char* argv[])
{
  argp_parse(&argp, argc, argv, 0, 0, &args);

  signals_handler_setup();


  if(args_socket_create() == 0)
  {
    if(stdin_stdout_fifo_open(&stdin_fifo, args.stdin_path, &stdout_fifo, args.stdout_path, fifo_reverse, args.debug) == 0)
    {
      stdin_stdout_thread_start(&stdin_thread, &stdin_routine, &stdout_thread, &stdout_routine, args.debug);
    }
  }


  stdin_stdout_fifo_close(&stdin_fifo, &stdout_fifo, args.debug);

  socket_close(&sockfd, args.debug);

  socket_close(&servfd, args.debug);

  if(args.debug) info_print("End of main");

  return 0;
}
