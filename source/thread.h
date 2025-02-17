/*
 * thread.h - verbose thread handling functions
 *
 * These functions make use of the debug.h library
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-02-17
 *
 *
 * In main compilation unit; define THREAD_IMPLEMENT
 *
 *
 * int stdin_stdout_thread_start(pthread_t* stdin_thread, void *(*stdin_routine) (void *), pthread_t* stdout_thread, void *(*stdout_routine) (void *), bool debug)
 */

#ifndef THREAD_H
#define THREAD_H

extern int stdin_stdout_thread_start(pthread_t* stdin_thread, void *(*stdin_routine) (void *), pthread_t* stdout_thread, void *(*stdout_routine) (void *), bool debug);

#endif // THREAD_H

/*
 * This header library file uses _IMPLEMENT guards
 *
 * If THREAD_IMPLEMENT is defined, the definitions will be included
 */

#ifdef THREAD_IMPLEMENT

#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#include "debug.h"

/*
 * Create stdin and stdout threads
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Failed to create stdin thread
 * - 2 | Failed to create stdout thread
 */
static inline int stdin_stdout_thread_create(pthread_t* stdin_thread, void *(*stdin_routine) (void *), pthread_t* stdout_thread, void *(*stdout_routine) (void *), bool debug)
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
static inline void stdin_stdout_thread_join(pthread_t stdin_thread, pthread_t stdout_thread, bool debug)
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

#endif // THREAD_IMPLEMENT
