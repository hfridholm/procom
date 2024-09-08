/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-09-08
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

extern int debug_print(FILE* stream, const char* title, const char* format, ...);

extern int error_print(const char* format, ...);

extern int info_print(const char* format, ...);

extern int format_string(char* buffer, const char* format, ...);

#endif // DEBUG_H
