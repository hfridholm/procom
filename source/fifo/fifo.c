#include "../fifo.h"

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to open stdin FIFO
 */
int stdin_fifo_open(int* stdinFIFO, const char stdinFIFOname[])
{
  info_print("Opening stdin FIFO");

  if((*stdinFIFO = open(stdinFIFOname, O_WRONLY)) == -1)
  {
    error_print("Failed to open stdin FIFO");
    
    return 1;
  }

  info_print("Opened stdin FIFO");

  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to open stdout FIFO
 */
int stdout_fifo_open(int* stdoutFIFO, const char stdoutFIFOname[])
{
  info_print("Opening stdout FIFO");

  if((*stdoutFIFO = open(stdoutFIFOname, O_RDONLY)) == -1)
  {
    error_print("Failed to open stdout FIFO");

    return 1;
  }
  
  info_print("Opened stdout FIFO");

  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to close stdin FIFO
 */
int stdin_fifo_close(int* stdinFIFO)
{
  // No need to close an already closed FIFO
  if(*stdinFIFO == -1) return 0;

  info_print("Closing stdin FIFO");

  if(close(*stdinFIFO) == -1)
  {
    error_print("Failed to close stdin FIFO: %s", strerror(errno));

    return 1;
  }
  *stdinFIFO = -1;

  info_print("Closed stdin FIFO");

  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to close stdout FIFO
 */
bool stdout_fifo_close(int* stdoutFIFO)
{
  // No need to close an already closed FIFO
  if(*stdoutFIFO == -1) return 0;

  info_print("Closing stdout FIFO");

  if(close(*stdoutFIFO) == -1)
  {
    error_print("Failed to close stdout FIFO: %s", strerror(errno));

    return false;
  }
  *stdoutFIFO = -1;

  info_print("Closed stdout FIFO");

  return true;
}

/*
 * PARAMS
 * - bool reversed
 *   - true  | First open stdin then stdout
 *   - false | First open stdout then stdin
 *
 * RETURN
 * [IMPORTANT] Same as stdin_stdout_fifo_open
 *
 * This is a very nice programming concept
 * I have never seen it being used before
 */
int stdout_stdin_fifo_open(int* stdoutFIFO, const char stdoutFIFOname[], int* stdinFIFO, const char stdinFIFOname[], bool reversed)
{
  if(reversed) return stdin_stdout_fifo_open(stdinFIFO, stdinFIFOname, stdoutFIFO, stdoutFIFOname, !reversed);

  if(stdout_fifo_open(stdoutFIFO, stdoutFIFOname) != 0) return 2;

  if(stdin_fifo_open(stdinFIFO, stdinFIFOname) != 0)
  {
    stdout_fifo_close(stdoutFIFO);

    return 1;
  }
  return 0;
}

/*
 * PARAMS
 * - bool reversed
 *   - true  | First open stdout then stdin
 *   - false | First open stdin then stdout
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to open stdin FIFO
 * - 2 | Failed to open stdout FIFO
 */
int stdin_stdout_fifo_open(int* stdinFIFO, const char stdinFIFOname[], int* stdoutFIFO, const char stdoutFIFOname[], bool reversed)
{
  if(reversed) return stdout_stdin_fifo_open(stdoutFIFO, stdoutFIFOname, stdinFIFO, stdinFIFOname, !reversed);

  if(stdin_fifo_open(stdinFIFO, stdinFIFOname) != 0) return 1;

  if(stdout_fifo_open(stdoutFIFO, stdoutFIFOname) != 0)
  {
    stdin_fifo_close(stdinFIFO);

    return 2;
  }
  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to close stdin FIFO
 * - 2 | Failed to close stdout FIFO
 *
 * Designed to close stdout even if stdin fails
 */
int stdin_stdout_fifo_close(int* stdinFIFO, int* stdoutFIFO)
{
  int status = 0;

  status = (stdin_fifo_close(stdinFIFO) == 0) ? status : 1;

  status = (stdout_fifo_close(stdoutFIFO) == 0) ? status : 2;

  return status;
}

/*
 * RETURN
 * - length | Success! The length of the read buffer
 * - -1     | Failed to read buffer
 */
int buffer_read(int fd, char* buffer, size_t size)
{
  char symbol = '\0';
  int index;

  for(index = 0; index < size && symbol != '\n'; index++)
  {
    int status = read(fd, &symbol, 1);

    if(status == -1) return -1; // ERROR

    buffer[index] = symbol;

    if(status == 0) break; // END OF FILE
  }
  return index;
}

/*
 * RETURN
 * - length | Success! The length of the written buffer
 * - -1     | Failed to write to buffer
 */
int buffer_write(int fd, const char* buffer, size_t size)
{
  int index;
  char symbol;

  for(index = 0; index < size; index++)
  {
    symbol = buffer[index];

    int status = write(fd, &symbol, 1);

    if(status == -1) return -1;
    if(status == 0) break;

    if(symbol == '\0' || symbol == '\n') break;
  }
  return index;
}
