#include "../debug.h"

int error_print(const char* format, ...)
{
  return fprintf(stderr, "[ ERROR ]: %s\n", format);
}

int info_print(const char* format, ...)
{
  return fprintf(stdout, "[ INFO ]: %s\n", format);
}
