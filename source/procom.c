/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-07-02
 */

#include "procom.h"

pthread_t stdin_thread;
bool      stdin_running = false;

pthread_t stdout_thread;
bool      stdout_running = false;

int sockfd = -1;
int servfd = -1;

bool fifo_reverse = false;

int stdin_fifo  = -1;
int stdout_fifo = -1;

static char doc[] = "procom - proccess communication";

static char args_doc[] = "[FILE...]";

static struct argp_option options[] =
{
  { "stdin",   'i', "STRING", 0, "Stdin FIFO" },
  { "stdout",  'o', "STRING", 0, "Stdout FIFO" },
  { "address", 'a', "STRING", 0, "Network address" },
  { "port",    'p', "NUMBER", 0, "Network port" },
  { "debug",   'd', 0,        0, "Print debug messages" },
  { 0 }
};

struct args
{
  char** args;
  size_t arg_count;
  char*  stdin_path;
  char*  stdout_path;
  char*  address;
  int    port;
  bool   reverse;
  bool   debug;
};

struct args args =
{
  .args        = NULL,
  .arg_count   = 0,
  .stdin_path  = NULL,
  .stdout_path = NULL,
  .address     = NULL,
  .port        = -1,
  .reverse     = false,
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
      args->port = arg ? atoi(arg) : -1;
      break;

    case 'r':
      args->reverse = true;
      break;

    case 'd':
      args->debug = true;
      break;

    case ARGP_KEY_ARG:
      args->args = realloc(args->args, sizeof(char*) * (state->arg_num + 1));

      if(!args->args) return ENOMEM;

      args->args[state->arg_num] = arg;

      args->arg_count = state->arg_num + 1;
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
static int stdin_thread_read(char* buffer, size_t size)
{
  // 1. Both fifo and socket are connected, read from fifo
  if(stdin_fifo != -1 && sockfd != -1)
  {
    return buffer_read(stdin_fifo, buffer, size);
  }
  // 2. If only one of fifo or socket is connected, read from stdin
  else
  {
    return buffer_read(0, buffer, size);
  }
}

/*
 * The stdin thread writes to either fifo, socket or terminal
 */
static void stdin_thread_write(const char* buffer, size_t size)
{
  // If both fifo and socket are connected, write to socket and terminal
  if(stdout_fifo != -1 && sockfd != -1)
  {
    socket_write(sockfd, buffer, size);

    buffer_write(1, buffer, size);
  }
  // If only output fifo is connected, write to fifo
  else if(stdout_fifo != -1)
  {
    buffer_write(stdout_fifo, buffer, size);
  }
  // If only socket is connected, write to socket
  else if(sockfd != -1)
  {
    socket_write(sockfd, buffer, size);
  }
  // If neither fifo nor socket is connected, write to stdout
  else
  {
    // This should be the same as printf
    buffer_write(1, buffer, size);
  }
} 

/*
 * The stdout thread takes input from either fifo, socket
 * If neither fifo nor socket is connected, nothing is done
 */
static int stdout_thread_read(char* buffer, size_t size)
{
  if(sockfd != -1)
  {
    return socket_read(sockfd, buffer, size);
  }
  else if(stdin_fifo != -1)
  {
    return buffer_read(stdin_fifo, buffer, size);
  }
  else return -1;
}

/*
 * The stdout thread writes to either fifo or terminal
 */
static void stdout_thread_write(const char* buffer, size_t size)
{
  // If both fifo and socket are connected, write to fifo
  if(stdout_fifo != -1 && sockfd != -1)
  {
    buffer_write(stdout_fifo, buffer, size);
  }
  // If either both or only one of fifo and socket are connected,
  // write to terminal
  if(stdout_fifo != -1 || sockfd != -1)
  {
    buffer_write(1, buffer, size);
  }
} 

/*
 *
 */
void* stdout_routine(void* arg)
{
  // No need for a recieving routine if neither fifo nor socket is connected
  if(stdin_fifo == -1 && sockfd == -1) return NULL;

  stdout_running = true;

  info_print("stdout routine...");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(stdout_thread_read(buffer, sizeof(buffer) - 1) != -1)
  {
    stdout_thread_write(buffer, sizeof(buffer) - 1);

    memset(buffer, '\0', sizeof(buffer));
  }

  info_print("killing stdin routine...");

  pthread_kill(stdin_thread, SIGUSR1);

  info_print("stop stdout routine");

  stdout_running = false;

  return NULL;
}

/*
 *
 */
void* stdin_routine(void* arg)
{
  stdin_running = true;

  info_print("stdin routine...");

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  while(stdin_thread_read(buffer, sizeof(buffer) - 1) != -1)
  {
    stdin_thread_write(buffer, sizeof(buffer) - 1);

    memset(buffer, '\0', sizeof(buffer));
  }

  info_print("killing stdout routine...");

  pthread_kill(stdout_thread, SIGUSR1);

  info_print("stop stdin routine");

  stdin_running = false;

  return NULL;
}

/*
 * Keyboard interrupt - close the program (the threads)
 */
void sigint_handler(int signum)
{
  if(args.debug) info_print("Keyboard interrupt");

  if(stdin_running)  pthread_kill(stdin_thread, SIGUSR1);

  if(stdout_running) pthread_kill(stdout_thread, SIGUSR1);
}

/*
 * Broken pipe - close the program (the threads)
 */
void sigpipe_handler(int signum)
{
  if(args.debug) error_print("Pipe has been broken");

  if(stdin_running)  pthread_kill(stdin_thread, SIGUSR1);

  if(stdout_running) pthread_kill(stdout_thread, SIGUSR1);
}

/*
 *
 */
void sigint_handler_setup(void)
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
void sigpipe_handler_setup(void)
{
  struct sigaction sig_action;

  sig_action.sa_handler = sigpipe_handler;
  sig_action.sa_flags = 0;
  sigemptyset(&sig_action.sa_mask);

  sigaction(SIGPIPE, &sig_action, NULL);
}

void sigusr1_handler(int signum) {}

/*
 *
 */
void sigusr1_handler_setup(void)
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
void signals_handler_setup(void)
{
  sigpipe_handler_setup();
  
  sigint_handler_setup();

  sigusr1_handler_setup();
}

/*
 *
 */
static void socket_create(void)
{
  // 1. Try to connect to a server using address and port
  sockfd = client_socket_create(args.address, args.port, args.debug);

  if(sockfd != -1) return;

  // 2. If no server was running, create a new server
  servfd = server_socket_create(args.address, args.port, args.debug);

  if(servfd == -1) return;

  // 3. Accept client connecting to server
  sockfd = socket_accept(servfd, args.address, args.port, args.debug);
}

static struct argp argp = { options, opt_parse, args_doc, doc };

/*
 * This is the main function
 */
int main(int argc, char* argv[])
{
  argp_parse(&argp, argc, argv, 0, 0, &args);

  signals_handler_setup();

  // If either an address or a port has been inputted,
  // the program should connect to a socket
  if(args.address || args.port != -1)
  {
    if(!args.address) args.address = "127.0.0.1";

    if(args.port == -1)
    {
      error_print("No port is specified");
    }
    else
    {
      socket_create();
    }
  }

  if(args.stdout_path && args.stdin_path && fifo_reverse)
  {
    stdout_fifo_open(&stdout_fifo, args.stdout_path, args.debug);

    stdin_fifo_open(&stdin_fifo, args.stdin_path, args.debug);
  }
  else
  {
    if(args.stdin_path)
    {
      stdin_fifo_open(&stdin_fifo, args.stdin_path, args.debug);
    }
    if(args.stdout_path)
    {
      stdout_fifo_open(&stdout_fifo, args.stdout_path, args.debug);
    }
  }

  stdin_stdout_thread_start(&stdin_thread, &stdin_routine, &stdout_thread, &stdout_routine, args.debug);

  stdout_fifo_close(&stdout_fifo, args.debug);

  stdin_fifo_close(&stdin_fifo, args.debug);

  socket_close(&sockfd, args.debug);

  socket_close(&servfd, args.debug);

  if(args.args) free(args.args);

  return 0;
}
