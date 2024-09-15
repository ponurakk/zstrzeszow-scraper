#ifndef LOGGER_H
#define LOGGER_H

void print_debug(const char *format, ...);
void print_info(const char *format, ...);
void print_warning(const char *format, ...);
void print_error(const char *format, ...);
void print_critical(const char *format, ...);

char *format_debug(char *txt);
char *format_info(char *txt);
char *format_warning(char *txt);
char *format_error(char *txt);
char *format_critical(char *txt);

#endif // !LOGGER_H
