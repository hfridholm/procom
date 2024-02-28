#ifndef FIFO_H
#define FIFO_H

#include "debug.h"

#include <stddef.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

extern int buffer_read(int fd, char* buffer, size_t size);

extern int buffer_write(int fd, const char* buffer, size_t size);

extern int stdin_stdout_fifo_close(int* stdinFIFO, int* stdoutFIFO);

extern int stdin_stdout_fifo_open(int* stdinFIFO, const char stdinFIFOname[], int* stdoutFIFO, const char stdoutFIFOname[], bool reversed);

extern int stdout_stdin_fifo_open(int* stdoutFIFO, const char stdoutFIFOname[], int* stdinFIFO, const char stdinFIFOname[], bool reversed);

#endif // FIFO_H
