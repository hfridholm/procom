#ifndef THREAD_H
#define THREAD_H

#include "debug.h"

#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

extern int stdin_stdout_thread_create(pthread_t* stdinThread, void *(*stdin_routine) (void *), pthread_t* stdoutThread, void *(*stdout_routine) (void *), bool debug);

extern void stdin_stdout_thread_join(pthread_t stdinThread, pthread_t stdoutThread, bool debug);

extern int stdin_stdout_thread_start(pthread_t* stdinThread, void *(*stdin_routine) (void *), pthread_t* stdoutThread, void *(*stdout_routine) (void *), bool debug);

#endif // THREAD_H
