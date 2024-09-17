#include "str_replace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *appendstr(char *string, const char *append) {
  char *newString = malloc(strlen(string) + strlen(append) + 1);

  sprintf(newString, "%s%s", string, append);
  free(string);
  return newString;
}

char *strtokk(char *string, const char *strf) {
  static char *ptr;
  static char *ptr2;

  if (!*strf)
    return string;
  if (string)
    ptr = string;
  else {
    if (!ptr2)
      return ptr2;
    ptr = ptr2 + strlen(strf);
  }

  if (ptr) {
    ptr2 = strstr(ptr, strf);
    if (ptr2)
      memset(ptr2, 0, strlen(strf));
  }
  return ptr;
}

char *str_replace(const char *cadena, const char *strf, const char *strr) {
  char *string;
  char *ptr;
  char *strrep;

  string = (char *)malloc(strlen(cadena) + 1);
  sprintf(string, "%s", cadena);
  if (!*strf)
    return string;
  ptr = strtokk(string, strf);
  strrep = malloc(strlen(ptr) + 1);
  memset(strrep, 0, strlen(ptr) + 1);
  while (ptr) {
    strrep = appendstr(strrep, ptr);
    ptr = strtokk(NULL, strf);
    if (ptr)
      strrep = appendstr(strrep, strr);
  }
  free(string);
  return strrep;
}
