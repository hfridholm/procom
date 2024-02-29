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

  int status;

  while((status = socket_read(sockfd, buffer, sizeof(buffer))) > 0)
  {
    fputs(buffer, stdout);

    memset(buffer, '\0', sizeof(buffer));
  }
  info_print("Stopped socket -> stdout");

  // If stdin thread has interrupted stdout thread
  if(status == -1 && errno == EINTR)
  {
    info_print("stdout routine interrupted"); 
  }

  // Interrupt stdin thread
  pthread_kill(stdinThread, SIGUSR1);

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
// - close sock and server sockets
// - exit the program
void sigint_handler(int signum)
{
  info_print("Keyboard interrupt");

  socket_close(&sockfd);
  socket_close(&serverfd);

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

/*
 * Accept socket client
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to accept client socket
 * - 2 | Failed to start stdin and stdout threads
 */
int server_process_step2(const char address[], int port)
{
  sockfd = socket_accept(serverfd, address, port);

  if(sockfd == -1) return 1;

  int status = stdin_stdout_thread_start(&stdinThread, &stdoutThread);

  socket_close(&sockfd);

  return (status != 0) ? 2 : 0;
}

/*
 * Create server socket
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create server socket
 * - 2 | Failed server_process_step2
 */
int server_process(const char address[], int port)
{
  serverfd = server_socket_create(address, port, 1);

  if(serverfd == -1) return 1;

  int status = server_process_step2(address, port);

  socket_close(&serverfd);

  return (status != 0) ? 2 : 0;
}

/*
 * Create client socket
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create client socket
 * - 2 | Failed to start stdin and stdout threads
 */
int client_process(const char address[], int port)
{
  sockfd = client_socket_create(address, port);

  if(sockfd == -1) return 1;

  int status = stdin_stdout_thread_start(&stdinThread, &stdoutThread);

  socket_close(&sockfd);

  return (status != 0) ? 2 : 0;
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
