#ifndef FIFO_H
#define FIFO_H

#include "debug.h"

#include <stddef.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

extern ssize_t buffer_read(int fd, char* buffer, size_t size);

extern ssize_t buffer_write(int fd, const char* buffer, size_t size);


extern int stdin_fifo_open(int* fifo, const char* path, bool debug);

extern int stdout_fifo_open(int* fifo, const char* path, bool debug);


extern int stdin_fifo_close(int* fifo, bool debug);

extern int stdout_fifo_close(int* fifo, bool debug);


extern int stdin_stdout_fifo_open(int* stdin_fifo, const char* stdin_path, int* stdout_fifo, const char* stdout_path, bool reverse, bool debug);

extern int stdout_stdin_fifo_open(int* stdout_fifo, const char* stdout_path, int* stdin_fifo, const char* stdin_path, bool reverse, bool debug);

#endif // FIFO_H
