#include "../debug.h"

/*
 * Format string of current time in timezone with hours, minuts, seconds and ms
 *
 * PARAMS
 * - char* buffer              | The buffer to store the format string at
 * - struct timezone* timezone | The timezone
 *
 * RETURN
 * - SUCCESS | Pointer at the time format string
 * - ERROR   | NULL
 */
char* time_format_string(char* buffer, struct timezone* timezone)
{
  struct timeval timeval;
  if(gettimeofday(&timeval, timezone) == -1) return NULL;

  struct tm* timeInfo = localtime(&timeval.tv_sec);

  strftime(buffer, 10, "%H:%M:%S", timeInfo);

  sprintf(buffer + 8, ".%03ld", timeval.tv_usec / 1000);

  return buffer;
}

/*
 * Parse va_list argument and print it to a buffer
 *
 * PARAMS
 * - char* buffer          | The buffer to store the printed argument at
 * - const char* specifier | The argument format specifier
 * - va_list args          | The va_list argument list
 *
 * RETURN
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
bool format_specifier_arg_append(char* buffer, const char* specifier, va_list args)
{
  if(!strncmp(specifier, "d", 1))
  {
    int arg = va_arg(args, int);

    return sprintf(buffer, "%d", arg);
  }
  else if(!strncmp(specifier, "ld", 2))
  {
    long int arg = va_arg(args, long int);

    return sprintf(buffer, "%ld", arg);
  }
  else if(!strncmp(specifier, "lld", 2))
  {
    long long int arg = va_arg(args, long long int);

    return sprintf(buffer, "%lld", arg);
  }
  else if(!strncmp(specifier, "c", 1))
  {
    // ‘char’ is promoted to ‘int’ when passed through ‘...’
    int arg = va_arg(args, int);

    return sprintf(buffer, "%c", arg);
  }
  else if(!strncmp(specifier, "f", 1))
  {
    // ‘float’ is promoted to ‘double’ when passed through ‘...’
    double arg = va_arg(args, double);

    return sprintf(buffer, "%lf", arg);
  }
  else if(!strncmp(specifier, "s", 1))
  {
    const char* arg = va_arg(args, const char*);

    return sprintf(buffer, "%s", arg);
  }
  else return -1; // Specifier does not exist
}

/*
 * Part of function format_args_string
 * Formats just a single format specifier argument from va_list
 *
 * RETURN
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
int format_arg_append(char* buffer, const char* format, int* fIndex, va_list args)
{
  char specifier[strlen(format)];
  memset(specifier, '\0', sizeof(specifier));

  for(int sIndex = 0; (*fIndex)++ < strlen(format); sIndex++)
  {
    specifier[sIndex] = format[*fIndex];

    int status = format_specifier_arg_append(buffer, specifier, args);

    // If the status is successful, return it
    if(status > 0) return status;
  }
  return -1; // Unsuccessful return
}

/*
 * sprintf, but with va_list as arguments
 *
 * RETURN (same as sprintf)
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
int format_args_string(char* buffer, const char* format, va_list args) {
  int bIndex = 0;

  for(int fIndex = 0; fIndex < strlen(format); fIndex++)
  {
    if(format[fIndex] == '%')
    {
      int status = format_arg_append(buffer + bIndex, format, &fIndex, args);

      // If failed to append format argument, return error
      if(status < 0) return -1;

      bIndex = strlen(buffer);
    }
    else buffer[bIndex++] = format[fIndex];
  }
  // Successful return - the number of printed characters (buffer length)
  return strlen(buffer);
}

/*
 * fprintf, but with va_list as arguments and time with title
 *
 * RETURN (same as fprintf)
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
int debug_args_print(FILE* stream, const char* title, const char* format, va_list args)
{
  char timeString[32];
  memset(timeString, '\0', sizeof(timeString));

  time_format_string(timeString, NULL);

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  int status = format_args_string(buffer, format, args);

  // If failed to create format string, return error
  if(status < 0) return -1;

  return fprintf(stream, "[%s] [ %s ]: %s\n", timeString, title, buffer);
}

/*
 * sprintf - my own implementation
 *
 * RETURN (same as sprintf)
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
int format_string(char* buffer, const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = format_args_string(buffer, format, args);

  va_end(args);

  return status;
}

/*
 * fprintf, but with time and title
 *
 * RETURN (same as fprintf)
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
int debug_print(FILE* stream, const char* title, const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = debug_args_print(stream, title, format, args);

  va_end(args);

  return status;
}

/*
 * fprintf to stderr, but with time and "ERROR" title
 *
 * RETURN (same as fprintf)
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
int error_print(const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = debug_args_print(stderr, "\e[1;31mERROR\e[0m", format, args);

  va_end(args);

  return status;
}

/*
 * fprintf to stdout, but with time and "INFO" title
 *
 * RETURN (same as fprintf)
 * - SUCCESS | The number of printed characters
 * - ERROR   | A negative value
 */
int info_print(const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = debug_args_print(stdout, "\e[1;37mINFO \e[0m", format, args);

  va_end(args);

  return status;
}
