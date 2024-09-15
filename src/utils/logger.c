#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

char *get_current_time(const char *format) {
  char buffer[100];
  time_t currentTime;
  struct tm localTime;

  // Get current time and convert it to local time
  time(&currentTime);
  localtime_r(&currentTime, &localTime);

  strftime(buffer, sizeof(buffer), format, &localTime);
  return strdup(buffer);
}

void __print_debug() { printf("[\033[1;34mDEBUG\033[1;0m]: "); }
void __print_info() { printf("[\033[1;32mINFO\033[1;0m]: "); }
void __print_warning() { printf("[\033[1;93mWARNING\033[1;0m]: "); }
void __print_error() { printf("[\033[1;31mERROR\033[1;0m]: "); }
void __print_critical() { printf("[\033[1;91mCRITICAL\033[1;0m]: "); }

void print_debug(const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("(%s) ", get_current_time("%H:%M:%S"));
  __print_debug();
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

void print_info(const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("(%s) ", get_current_time("%H:%M:%S"));
  __print_info();
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

void print_warning(const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("(%s) ", get_current_time("%H:%M:%S"));
  __print_warning();
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

void print_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("(%s) ", get_current_time("%H:%M:%S"));
  __print_error();
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

void print_critical(const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("(%s) ", get_current_time("%H:%M:%S"));
  __print_critical();
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

char *format_debug(char *txt) {
  char res[strlen(txt) + 30];
  sscanf(res, "\033[1;34m[DEBUG]\033[1;0m: %s\n", txt);
  return strdup(res);
}

char *format_info(char *txt) {
  char res[strlen(txt) + 30];
  sscanf(res, "\033[1;32m[INFO]\033[1;0m: %s\n", txt);
  return strdup(res);
}

char *format_warning(char *txt) {
  char res[strlen(txt) + 30];
  sscanf(res, "\033[1;93m[WARNING]\033[1;0m: %s\n", txt);
  return strdup(res);
}

char *format_error(char *txt) {
  char res[strlen(txt) + 30];
  sscanf(res, "\033[1;31m[ERROR]\033[1;0m: %s\n", txt);
  return strdup(res);
}

char *format_critical(char *txt) {
  char res[strlen(txt) + 30];
  sscanf(res, "\033[1;91m[CRITICAL]\033[1;0m: %s\n", txt);
  return strdup(res);
}
