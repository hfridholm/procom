#ifndef THREAD_H
#define THREAD_H

#include "debug.h"

#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

extern int  stdin_stdout_thread_start(pthread_t* stdin_thread, void *(*stdin_routine) (void *), pthread_t* stdout_thread, void *(*stdout_routine) (void *), bool debug);

#endif // THREAD_H
