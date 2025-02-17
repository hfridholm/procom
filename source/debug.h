/*
 * debug.h - functions for outputing debug messages
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-12-04
 *
 *
 * In main compilation unit; define DEBUG_IMPLEMENT
 *
 *
 * These are the available funtions:
 *
 * int debug_print(FILE* stream, const char* title, const char* format, ...)
 *
 * int error_print(const char* format, ...)
 *
 * int info_print(const char* format, ...)
 *
 * int debug_file_open(const char* filepath)
 *
 * void debug_file_close(void)
 *
 *
 * Uses va_list for argument parsing, like in getstr.c
 */

/*
 * From here on, until DEBUG_IMPLEMENT,
 * it is like a normal header file with declarations
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

extern int debug_print(FILE* stream, const char* title, const char* format, ...);

extern int error_print(const char* format, ...);

extern int info_print(const char* format, ...);


extern int debug_file_open(const char* filepath);

extern void debug_file_close(void);

extern FILE* debug_file;

#endif // DEBUG_H

/*
 * This header library file uses _IMPLEMENT guards
 *
 * If DEBUG_IMPLEMENT is defined, the definitions will be included
 */

#ifdef DEBUG_IMPLEMENT

#include <stdarg.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

FILE* debug_file = NULL;

/*
 * The debug format must include:
 * - %s for the time string
 * - %s for the title
 * - %s for the message
 * - \n for new-line
 */
#define DEBUG_FORMAT "[%s] [ %s ]: %s\n"

/*
 * Format string of current time
 *
 * PARAMS
 * - char* buffer | Buffer to store time string
 *
 * RETURN (char* buffer)
 * - NULL | Failed to get time of day
 */
static inline char* dbg_timestr_create(char* buffer)
{
  struct timeval timeval;
  if(gettimeofday(&timeval, NULL) == -1) return NULL;

  struct tm* timeinfo = localtime(&timeval.tv_sec);

  strftime(buffer, 10, "%H:%M:%S", timeinfo);

  sprintf(buffer + 8, ".%02ld", timeval.tv_usec / 10000);

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
 * RETURN (int amount, same as sprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
static inline int dbg_specifier_append(char* buffer, const char* specifier, va_list args)
{
  if(strncmp(specifier, "d", 1) == 0)
  {
    int arg = va_arg(args, int);

    return sprintf(buffer, "%d", arg);
  }
  else if(strncmp(specifier, "ld", 2) == 0)
  {
    long int arg = va_arg(args, long int);

    return sprintf(buffer, "%ld", arg);
  }
  else if(strncmp(specifier, "lld", 2) == 0)
  {
    long long int arg = va_arg(args, long long int);

    return sprintf(buffer, "%lld", arg);
  }
  else if(strncmp(specifier, "c", 1) == 0)
  {
    // ‘char’ is promoted to ‘int’ when passed through ‘...’
    int arg = va_arg(args, int);

    return sprintf(buffer, "%c", arg);
  }
  else if(strncmp(specifier, "f", 1) == 0)
  {
    // ‘float’ is promoted to ‘double’ when passed through ‘...’
    double arg = va_arg(args, double);

    return sprintf(buffer, "%lf", arg);
  }
  else if(strncmp(specifier, "s", 1) == 0)
  {
    const char* arg = va_arg(args, const char*);

    return sprintf(buffer, "%s", arg);
  }
  else return -1; // Specifier does not exist
}

/*
 * Formats just a single format specifier argument from va_list
 *
 * RETURN (int amount, same as sprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
static inline int dbg_arg_append(char* buffer, const char* format, int f_length, int* f_index, va_list args)
{
  char specifier[f_length + 1];

  for(int s_index = 0; (*f_index)++ < f_length; s_index++)
  {
    specifier[s_index] = format[*f_index];
    specifier[s_index + 1] = '\0';

    int amount = dbg_specifier_append(buffer, specifier, args);

    // If a valid format specifier has been found and parsed,
    // return the status of the appended specifier
    if(amount > 0) return amount;
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
static inline int dbg_string_create(char* string, const char* format, va_list args)
{
  const size_t f_length = strlen(format);

  int s_index = 0;

  for(int f_index = 0; f_index < f_length; f_index++)
  {
    if(format[f_index] == '%')
    {
      int amount = dbg_arg_append(string + s_index, format, f_length, &f_index, args);

      if(amount < 0) return -1;

      s_index += amount;
    }
    else string[s_index++] = format[f_index];
  }

  string[s_index] = '\0';

  return s_index;
}

/*
 * Print custom debug message, taking in va_list
 *
 * RETURN (same as fprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
static inline int dbg_valist_print(FILE* stream, const char* title, const char* format, va_list args)
{
  char timestr[32];

  if(dbg_timestr_create(timestr) == NULL)
  {
    return -1;
  }

  char string[1024];

  if(dbg_string_create(string, format, args) < 0)
  {
    return -1;
  }

  return fprintf(stream, DEBUG_FORMAT, timestr, title, string);
}

/*
 * Print own debug message to specified stream
 *
 * RETURN (same as fprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
int debug_print(FILE* stream, const char* title, const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int amount = dbg_valist_print(stream, title, format, args);

  fflush(stream);

  va_end(args);

  return amount;
}

/*
 * Print debug error message to stderr
 *
 * RETURN (same as fprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
int error_print(const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int amount;

  if(debug_file)
  {
    amount = dbg_valist_print(debug_file, "ERROR", format, args);

    fflush(debug_file);
  }
  else
  {
    amount = dbg_valist_print(stderr, "\e[1;37mERROR\e[0m", format, args);

    fflush(stderr);
  }

  va_end(args);

  return amount;
}

/*
 * Print debug info message to stdout
 *
 * RETURN (same as fprintf)
 * - >=0 | Number of printed characters
 * -  -1 | Format specifier does not exist, or sprintf error
 */
int info_print(const char* format, ...)
{
  va_list args;

  va_start(args, format);

  int amount;

  if(debug_file)
  {
    amount = dbg_valist_print(debug_file, "INFO", format, args);

    fflush(debug_file);
  }
  else
  {
    amount = dbg_valist_print(stdout, "\e[1;37mINFO \e[0m", format, args);

    fflush(stdout);
  }

  va_end(args);

  return amount;
}

/*
 * Open and start printing to debug file
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Failed to open file
 */
int debug_file_open(const char* filepath)
{
  FILE* stream = fopen(filepath, "a");

  if(!stream) return 1;

  if(debug_file) fclose(debug_file);

  debug_file = stream;

  return 0;
}

/*
 * Close the debug file
 */
void debug_file_close(void)
{
  if(debug_file) fclose(debug_file);

  debug_file = NULL;
}

#endif // DEBUG_IMPLEMENT

/*
 * Maybe:
 * - Add pthread mutex locks to make global "file" thread safe
 * - Create multiple debug files for [stderr, stdout]
 */
