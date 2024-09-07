/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-09-07
 */

#include "thread.h"

/*
 * Create stdin and stdout threads
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Failed to create stdin thread
 * - 2 | Failed to create stdout thread
 */
static int stdin_stdout_thread_create(pthread_t* stdin_thread, void *(*stdin_routine) (void *), pthread_t* stdout_thread, void *(*stdout_routine) (void *), bool debug)
{
  if(pthread_create(stdin_thread, NULL, stdin_routine, NULL) != 0)
  {
    if(debug) error_print("Failed to create stdin thread");

    return 1;
  }

  if(pthread_create(stdout_thread, NULL, stdout_routine, NULL) != 0)
  {
    if(debug) error_print("Failed to create stdout thread");

    // Interrupt stdin thread
    pthread_kill(*stdin_thread, SIGUSR1);

    return 2;
  }

  return 0;
}

/*
 * Join stdin and stdout threads
 */
static void stdin_stdout_thread_join(pthread_t stdin_thread, pthread_t stdout_thread, bool debug)
{
  if(pthread_join(stdin_thread, NULL) != 0)
  {
    if(debug) error_print("Failed to join stdin thread");
  }

  if(pthread_join(stdout_thread, NULL) != 0)
  {
    if(debug) error_print("Failed to join stdout thread");
  }
}

/*
 * Start stdin and stdout thread
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Failed to create stdin and stdout threads
 */
int stdin_stdout_thread_start(pthread_t* stdin_thread, void *(*stdin_routine) (void *), pthread_t* stdout_thread, void *(*stdout_routine) (void *), bool debug)
{
  if(stdin_stdout_thread_create(stdin_thread, stdin_routine, stdout_thread, stdout_routine, debug) != 0) return 1;
  
  stdin_stdout_thread_join(*stdin_thread, *stdout_thread, debug);

  return 0;
}
