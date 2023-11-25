#include "../debug.h"

char* time_format_string(char* buffer, struct timezone* timezone)
{
  struct timeval timeval;
  if(gettimeofday(&timeval, timezone) == -1) return NULL;

  struct tm* timeInfo = localtime(&timeval.tv_sec);

  strftime(buffer, 10, "%H:%M:%S", timeInfo);

  sprintf(buffer + 8, ".%03ld", timeval.tv_usec / 1000);

  return buffer;
}

bool format_specifier_arg_append(char* buffer, const char* specifier, va_list args)
{
  if(!strncmp(specifier, "d", 1))
  {
    int arg = va_arg(args, int);

    sprintf(buffer, "%d", arg);
  }
  else if(!strncmp(specifier, "ld", 2))
  {
    long int arg = va_arg(args, long int);

    sprintf(buffer, "%ld", arg);
  }
  else if(!strncmp(specifier, "lld", 2))
  {
    long long int arg = va_arg(args, long long int);

    sprintf(buffer, "%lld", arg);
  }
  else if(!strncmp(specifier, "c", 1))
  {
    // ‘char’ is promoted to ‘int’ when passed through ‘...’
    int arg = va_arg(args, int);

    sprintf(buffer, "%c", arg);
  }
  else if(!strncmp(specifier, "f", 1))
  {
    // ‘float’ is promoted to ‘double’ when passed through ‘...’
    double arg = va_arg(args, double);

    sprintf(buffer, "%lf", arg);
  }
  else if(!strncmp(specifier, "s", 1))
  {
    const char* arg = va_arg(args, const char*);

    sprintf(buffer, "%s", arg);
  }
  else return false;

  return true;
}

int format_arg_append(char* buffer, const char* format, int* fIndex, va_list args)
{
  char specifier[strlen(format)];
  memset(specifier, '\0', sizeof(specifier));

  for(int sIndex = 0; (*fIndex)++ < strlen(format); sIndex++)
  {
    specifier[sIndex] = format[*fIndex];

    if(format_specifier_arg_append(buffer, specifier, args)) return 0;
  }
  return 0; // Implement actual status codes
}

int format_args_string(char* buffer, const char* format, va_list args)
{
  int bIndex = 0;

  for(int fIndex = 0; fIndex < strlen(format); fIndex++)
  {
    if(format[fIndex] == '%')
    {
      format_arg_append(buffer + bIndex, format, &fIndex, args);

      bIndex = strlen(buffer);
    }
    else buffer[bIndex++] = format[fIndex];
  }
  return 0; // Implement actual status codes
}

int debug_args_print(FILE* stream, const char* title, const char* format, va_list args)
{
  char timeString[32];
  memset(timeString, '\0', sizeof(timeString));

  time_format_string(timeString, NULL);

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  format_args_string(buffer, format, args);

  return fprintf(stream, "[%s] [ %s ]: %s\n", timeString, title, buffer);
}

int format_string(char* buffer, const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = format_args_string(buffer, format, args);

  va_end(args);

  return status;
}

int debug_print(FILE* stream, const char* title, const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = debug_args_print(stream, title, format, args);

  va_end(args);

  return status;
}

int error_print(const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = debug_args_print(stderr, "ERROR", format, args);

  va_end(args);

  return status;
}

int info_print(const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = debug_args_print(stdout, "INFO", format, args);

  va_end(args);

  return status;
}
