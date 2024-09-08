/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-09-08
 */

#include "debug.h"

/*
 * Format string of current time in timezone with hours, minuts, seconds and ms
 *
 * PARAMS
 * - char* buffer              | Buffer to store format string
 * - struct timezone* timezone | Timezone
 *
 * RETURN (char* buffer)
 * - NULL | Failed to get time of day
 */
static char* time_format_string(char* buffer, struct timezone* timezone)
{
  struct timeval timeval;
  if(gettimeofday(&timeval, timezone) == -1) return NULL;

  struct tm* timeinfo = localtime(&timeval.tv_sec);

  strftime(buffer, 10, "%H:%M:%S", timeinfo);

  sprintf(buffer + 8, ".%03ld", timeval.tv_usec / 1000);

  return buffer;
}

/*
 * Parse va_list argument and print it to a buffer
 *
 * PARAMS
 * - char* buffer          | Buffer to store printed argument
 * - const char* specifier | Argument format specifier
 * - va_list args          | va_list argument list
 *
 * RETURN (same as sprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
static int format_specifier_arg_append(char* buffer, const char* specifier, va_list args)
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
 * RETURN (same as sprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
static int format_arg_append(char* buffer, const char* format, int* f_index, va_list args)
{
  const size_t f_length = strlen(format);

  char specifier[f_length + 1];

  for(int s_index = 0; (*f_index)++ < f_length; s_index++)
  {
    specifier[s_index]     = format[*f_index];
    specifier[s_index + 1] = '\0';

    int status = format_specifier_arg_append(buffer, specifier, args);

    // If a valid format specifier has been found and parsed,
    // return the status of the appended specifier
    if(status > 0) return status;
  }

  return -1;
}

/*
 * sprintf, but with va_list as arguments
 *
 * RETURN (same as sprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
static int format_args_string(char* buffer, const char* format, va_list args)
{
  const size_t f_length = strlen(format);

  int b_index = 0;

  for(int f_index = 0; f_index < f_length; f_index++)
  {
    if(format[f_index] == '%')
    {
      int status = format_arg_append(buffer + b_index, format, &f_index, args);

      // If failed to append format argument, return error
      if(status < 0) return -1;

      b_index = strlen(buffer);
    }
    else buffer[b_index++] = format[f_index];
  }

  return strlen(buffer);
}

/*
 * fprintf, but with va_list as arguments and time with title
 *
 * RETURN (same as fprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
static int debug_args_print(FILE* stream, const char* title, const char* format, va_list args)
{
  char time_string[32];
  memset(time_string, '\0', sizeof(time_string));

  time_format_string(time_string, NULL);

  char buffer[1024];
  memset(buffer, '\0', sizeof(buffer));

  int status = format_args_string(buffer, format, args);

  // If failed to create format string, return error
  if(status < 0) return -1;

  return fprintf(stream, "[%s] [ %s ]: %s\n", time_string, title, buffer);
}

/*
 * sprintf - my own implementation
 *
 * RETURN (same as sprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
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
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
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
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
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
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
int info_print(const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int status = debug_args_print(stdout, "\e[1;37mINFO \e[0m", format, args);

  va_end(args);

  return status;
}
