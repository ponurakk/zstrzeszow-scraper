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
  snprintf(new_string, len, "%s%s", *string ? *string : "", append);
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
  char *buffer = malloc(max_new_len + 1); // Allocate new buffer
  if (!buffer) {
    return; // Handle allocation failure
  }

  const char *ptr = *target;
  char *write_ptr = buffer;

  while (*ptr) {
    // Look for match
    if (strncmp(ptr, strf, find_len) == 0) {
      // Replace match with strr
      memcpy(write_ptr, strr, replace_len);
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

void str_replace_multiple(char **target, ReplacePair *replacements,
                          size_t num_replacements) {
  if (!target || !*target || !replacements || num_replacements == 0) {
    return; // No work if input is NULL or no replacements
  }

  const char *src = *target;
  size_t target_len = strlen(*target);

  // Temporary buffer to calculate the size of the final string
  size_t new_length = target_len;
  size_t *find_lengths = malloc(num_replacements * sizeof(size_t));
  size_t *replace_lengths = malloc(num_replacements * sizeof(size_t));

  for (size_t i = 0; i < num_replacements; ++i) {
    find_lengths[i] = strlen(replacements[i].find);
    replace_lengths[i] = strlen(replacements[i].replace);
  }

  // First pass: Calculate final length
  for (size_t i = 0; i < num_replacements; ++i) {
    const char *p = src;
    while ((p = strstr(p, replacements[i].find))) {
      new_length += replace_lengths[i] - find_lengths[i];
      p += find_lengths[i];
    }
  }

  // Allocate new buffer for the modified string
  char *buffer = malloc(new_length + 1);
  if (!buffer) {
    free(find_lengths);
    free(replace_lengths);
    return; // Allocation failure
  }

  // Second pass: Apply replacements
  char *write_ptr = buffer;
  while (*src) {
    int replaced = 0;
    for (size_t i = 0; i < num_replacements; ++i) {
      if (strncmp(src, replacements[i].find, find_lengths[i]) == 0) {
        // Copy replacement string
        memcpy(write_ptr, replacements[i].replace, replace_lengths[i]);
        write_ptr += replace_lengths[i];
        src += find_lengths[i];
        replaced = 1;
        break;
      }
    }
    if (!replaced) {
      *write_ptr++ = *src++;
    }
  }

  *write_ptr = '\0';

  // Cleanup and update target
  free(*target);
  free(find_lengths);
  free(replace_lengths);
  *target = buffer;
}
