#include "../fifo.h"

bool stdin_fifo_open(int* stdinFIFO, const char stdinFIFOname[])
{
  info_print("Opening stdin-fifo");

  *stdinFIFO = open(stdinFIFOname, O_WRONLY);

  if(*stdinFIFO == -1)
  {
    error_print("Could not open stdin-fifo");
    
    return false;
  }
  return true;
}

bool stdout_fifo_open(int* stdoutFIFO, const char stdoutFIFOname[])
{
  info_print("Opening stdout-fifo");

  *stdoutFIFO = open(stdoutFIFOname, O_RDONLY);
  
  if(*stdoutFIFO == -1)
  {
    error_print("Could not open stdout-fifo");

    return false;
  }
  return true;
}

bool stdin_fifo_close(int* stdinFIFO)
{
  info_print("Closing stdin-fifo");

  if(close(*stdinFIFO) == -1)
  {
    error_print("Could not close stdin-fifo: %s", strerror(errno));

    return false;
  }
  *stdinFIFO = -1;

  return true;
}

bool stdout_fifo_close(int* stdoutFIFO)
{
  info_print("Closing stdout-fifo");

  if(close(*stdoutFIFO) == -1)
  {
    error_print("Could not close stdout-fifo: %s", strerror(errno));

    return false;
  }
  *stdoutFIFO = -1;

  return true;
}

bool stdin_stdout_fifo_open(int* stdinFIFO, const char stdinFIFOname[], int* stdoutFIFO, const char stdoutFIFOname[], bool openOrder)
{
  if(openOrder)
  {
    if(!stdin_fifo_open(stdinFIFO, stdinFIFOname)) return false;

    if(!stdout_fifo_open(stdoutFIFO, stdoutFIFOname))
    {
      stdin_fifo_close(stdinFIFO);

      return false;
    }
  }
  else
  {
    if(!stdout_fifo_open(stdoutFIFO, stdoutFIFOname)) return false;

    if(!stdin_fifo_open(stdinFIFO, stdinFIFOname))
    {
      stdout_fifo_close(stdoutFIFO);

      return false;
    }
  }
  return true;
}

// Note: Close both
bool stdin_stdout_fifo_close(int* stdinFIFO, int* stdoutFIFO)
{
  if(!stdin_fifo_close(stdinFIFO)) return false;

  if(!stdout_fifo_close(stdoutFIFO)) return false;

  return true;
}

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
