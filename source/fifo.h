/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-09-08
 */

#ifndef FIFO_H
#define FIFO_H

#include "debug.h"

#include <stddef.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

extern int stdin_stdout_fifo_open(int* stdin_fifo, const char* stdin_path, int* stdout_fifo, const char* stdout_path, bool reverse, bool debug);

extern int fifo_close(int* fifo, bool debug);


extern ssize_t buffer_read(int fd, char* buffer, size_t size);

extern ssize_t buffer_write(int fd, const char* buffer, size_t size);

#endif // FIFO_H
