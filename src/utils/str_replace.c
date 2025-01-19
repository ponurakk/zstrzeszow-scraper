#include "str_replace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int appendstr(char **string, const char *append) {
  // Handle NULL
  size_t len = (*string ? strlen(*string) : 0) + strlen(append) + 1;
  char *new_string = malloc(len);
  if (!new_string) {
    return -1; // Handle allocation failure
  }

  // Avoid NULL dereference
  sprintf(new_string, "%s%s", *string ? *string : "", append);
  free(*string);
  *string = new_string;
  return 0;
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

void str_replace(char **target, const char *strf, const char *strr) {
  if (!*target || !strf || !*strf) {
    return; // No work if input is NULL or find string is empty
  }

  size_t target_len = strlen(*target);
  size_t find_len = strlen(strf);
  size_t replace_len = strlen(strr);

  // Estimate buffer size for replacement
  size_t count = 0;
  for (const char *p = *target; (p = strstr(p, strf)); p += find_len) {
    count++;
  }

  // Calculate new length based on replacements
  size_t max_new_len = target_len + count * (replace_len - find_len);
  char *buffer = malloc(max_new_len + 1);
  if (!buffer) {
    return; // Handle allocation failure
  }

  char *ptr = *target;
  char *write_ptr = buffer;
  while (*ptr) {
    // Look for match
    if (strncmp(ptr, strf, find_len) == 0) {
      // Replace match with strr
      strcpy(write_ptr, strr);
      write_ptr += replace_len;
      ptr += find_len;
    } else {
      // Copy character
      *write_ptr++ = *ptr++;
    }
  }
  *write_ptr = '\0'; // Null-terminate new string

  free(*target);    // Free old string
  *target = buffer; // Update to new buffer
}
