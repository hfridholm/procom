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

extern int stdin_stdout_fifo_close(int* stdinFIFO, int* stdoutFIFO, bool debug);

extern int stdin_stdout_fifo_open(int* stdinFIFO, const char* stdinPathname, int* stdoutFIFO, const char* stdoutPathname, bool reversed, bool debug);

#endif // FIFO_H
