#include "socket.h"

#include <stdio.h>

void error_print(char* format, ...)
{
  fprintf(stderr, "[ ERROR ]: %s\n", format);
}

void info_print(char* format, ...)
{
  fprintf(stdout, "[ INFO ]: %s\n", format);
}

int main(int argc, char* argv[])
{
  info_print("Started process");

  info_print("Stopped process");

  return 0;
}
