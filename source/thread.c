#include "thread.h"

/*
 * Create stdin and stdout threads
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create stdin thread
 * - 2 | Failed to create stdout thread
 */
int stdin_stdout_thread_create(pthread_t* stdinThread, void *(*stdin_routine) (void *), pthread_t* stdoutThread, void *(*stdout_routine) (void *), bool debug)
{
  if(pthread_create(stdinThread, NULL, stdin_routine, NULL) != 0)
  {
    if(debug) error_print("Failed to create stdin thread");

    return 1;
  }
  if(pthread_create(stdoutThread, NULL, stdout_routine, NULL) != 0)
  {
    if(debug) error_print("Failed to create stdout thread");

    // Interrupt stdin thread
    pthread_kill(*stdinThread, SIGUSR1);

    return 2;
  }
  return 0;
}

/*
 * Join stdin and stdout threads
 */
void stdin_stdout_thread_join(pthread_t stdinThread, pthread_t stdoutThread, bool debug)
{
  if(pthread_join(stdinThread, NULL) != 0)
  {
    if(debug) error_print("Failed to join stdin thread");
  }
  if(pthread_join(stdoutThread, NULL) != 0)
  {
    if(debug) error_print("Failed to join stdout thread");
  }
}

/*
 * Start stdin and stdout thread
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create stdin and stdout threads
 */
int stdin_stdout_thread_start(pthread_t* stdinThread, void *(*stdin_routine) (void *), pthread_t* stdoutThread, void *(*stdout_routine) (void *), bool debug)
{
  if(stdin_stdout_thread_create(stdinThread, stdin_routine, stdoutThread, stdout_routine, debug) != 0) return 1;
  
  stdin_stdout_thread_join(*stdinThread, *stdoutThread, debug);

  return 0;
}
