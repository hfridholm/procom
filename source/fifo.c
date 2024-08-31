/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-08-31
 */

#include "fifo.h"

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to open stdin FIFO
 */
int stdin_fifo_open(int* fifo, const char* path, bool debug)
{
  if(debug) info_print("Opening stdin FIFO (%s)", path);

  if((*fifo = open(path, O_RDONLY)) == -1)
  {
    if(debug) error_print("Failed to open stdin FIFO (%s)", path);
    
    return 1;
  }

  if(debug) info_print("Opened stdin FIFO (%s)", path);

  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to open stdout FIFO
 */
int stdout_fifo_open(int* fifo, const char* path, bool debug)
{
  if(debug) info_print("Opening stdout FIFO (%s)", path);

  if((*fifo = open(path, O_WRONLY)) == -1)
  {
    if(debug) error_print("Failed to open stdout FIFO (%s)", path);

    return 1;
  }
  
  if(debug) info_print("Opened stdout FIFO (%s)", path);

  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to close stdin FIFO
 */
int stdin_fifo_close(int* fifo, bool debug)
{
  // No need to close an already closed FIFO
  if(*fifo == -1) return 0;

  if(debug) info_print("Closing stdin FIFO (%d)", *fifo);

  if(close(*fifo) == -1)
  {
    if(debug) error_print("Failed to close stdin FIFO: %s", strerror(errno));

    return 1;
  }
  *fifo = -1;

  if(debug) info_print("Closed stdin FIFO");

  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to close stdout FIFO
 */
int stdout_fifo_close(int* fifo, bool debug)
{
  // No need to close an already closed FIFO
  if(*fifo == -1) return 0;

  if(debug) info_print("Closing stdout FIFO (%d)", *fifo);

  if(close(*fifo) == -1)
  {
    if(debug) error_print("Failed to close stdout FIFO: %s", strerror(errno));

    return 1;
  }
  *fifo = -1;

  if(debug) info_print("Closed stdout FIFO");

  return 0;
}

/*
 * PARAMS
 * - bool reverse
 *   - true  | First open stdin then stdout
 *   - false | First open stdout then stdin
 *
 * RETURN
 * [IMPORTANT] Same as stdin_stdout_fifo_open
 *
 * This is a very nice programming concept
 * I have never seen it being used before
 */
int stdout_stdin_fifo_open(int* stdout_fifo, const char* stdout_path, int* stdin_fifo, const char* stdin_path, bool reverse, bool debug)
{
  if(reverse) return stdin_stdout_fifo_open(stdin_fifo, stdin_path, stdout_fifo, stdout_path, !reverse, debug);

  int status = 0b00;

  if(stdout_path && stdout_fifo_open(stdout_fifo, stdout_path, debug) != 0)
  {
    status |= (0b01 << 1);
  }

  if(stdin_path && stdin_fifo_open(stdin_fifo, stdin_path, debug) != 0)
  {
    status |= (0b01 << 0);
  }

  return status;
}

/*
 * PARAMS
 * - bool reverse
 *   - true  | First open stdout then stdin
 *   - false | First open stdin then stdout
 *
 * RETURN (int status)
 * - 01 | Failed to open stdin FIFO
 * - 10 | Failed to open stdout FIFO
 */
int stdin_stdout_fifo_open(int* stdin_fifo, const char* stdin_path, int* stdout_fifo, const char* stdout_path, bool reverse, bool debug)
{
  if(reverse) return stdout_stdin_fifo_open(stdout_fifo, stdout_path, stdin_fifo, stdin_path, !reverse, debug);

  int status = 0b00;

  if(stdin_path && stdin_fifo_open(stdin_fifo, stdin_path, debug) != 0)
  {
    status |= (0b01 << 0);
  }

  if(stdout_path && stdout_fifo_open(stdout_fifo, stdout_path, debug) != 0)
  {
    status |= (0b01 << 1);
  }

  return status;
}

/*
 * RETURN (int status)
 * - 01 | Failed to close stdin FIFO
 * - 10 | Failed to close stdout FIFO
 *
 * Designed to close stdout even if stdin fails
 */
int stdin_stdout_fifo_close(int* stdin_fifo, int* stdout_fifo, bool debug)
{
  int status = 0;

  if(stdin_fifo_close(stdin_fifo, debug) != 0)
  {
    status |= (0b01 << 0);
  }

  if(stdout_fifo_close(stdout_fifo, debug) != 0)
  {
    status |= (0b01 << 1);
  }

  return status;
}

/*
 * RETURN (ssize_t)
 * - >0 | Success! The length of the read buffer
 * -  0 | End of File
 * - -1 | Failed to read buffer
 */
ssize_t buffer_read(int fd, char* buffer, size_t size)
{
  char symbol = '\0';
  ssize_t index;

  for(index = 0; index < size && symbol != '\n'; index++)
  {
    if(errno != 0) return -1;

    ssize_t status = read(fd, &symbol, 1);

    if(status == -1 || errno != 0) return -1; // ERROR

    buffer[index] = symbol;

    if(status == 0) return 0; // END OF FILE
  }
  return index;
}

/*
 * RETURN (ssize_t)
 * - >0 | Success! The length of the written buffer
 * -  0 | End of File? (I think)
 * - -1 | Failed to write to buffer
 */
ssize_t buffer_write(int fd, const char* buffer, size_t size)
{
  ssize_t index;
  char symbol;

  for(index = 0; index < size; index++)
  {
    symbol = buffer[index];

    if(errno != 0) return -1;

    ssize_t status = write(fd, &symbol, 1);

    if(status == -1 || errno != 0) return -1;

    if(status == 0) return 0;

    if(symbol == '\0' || symbol == '\n') break;
  }
  return index;
}
