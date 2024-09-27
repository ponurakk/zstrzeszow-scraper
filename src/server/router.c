#include "router.h"
#include "../utils/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *path_to_file(char *path) {
  if (strcmp(path, "/") == 0) {
    return INDEX_FILE;
  } else if (strcmp(path, "/styles.css") == 0) {
    return STYLES_FILE;
  }

  return NOT_FOUND_FILE;
}

char *read_file(const char *filename, long *file_size) {
  FILE *file = fopen(filename, "rb");
  char *file_buffer = NULL;

  if (file == NULL) {
    perror("Failed to open file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  *file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  file_buffer = (char *)malloc(*file_size + 1);
  if (file_buffer == NULL) {
    perror("Memory allocation failed");
    fclose(file);
    return NULL;
  }

  size_t bytes_read = fread(file_buffer, 1, *file_size, file);
  if (bytes_read != *file_size) {
    perror("Failed to read the entire file");
    free(file_buffer);
    fclose(file);
    return NULL;
  }

  file_buffer[*file_size] = '\0';

  fclose(file);
  return file_buffer;
}

Error get_template(char *path, char **file_buffer, long *file_size,
                   Template *templ, char **res) {
  char *file_path = path_to_file(path);
  char *split = strtok(path, "/");
  if (split != NULL) {
    char *number = strtok(NULL, "/");
    if (number != NULL) {
      *res = strdup(number);
      file_path = PLAN_FILE;
      if (strcmp(split, "o") == 0) {
        *templ = WARD;
      } else if (strcmp(split, "n") == 0) {
        *templ = TEACHER;
      } else if (strcmp(split, "s") == 0) {
        *templ = CLASSROOM;
      } else {
        file_path = NOT_FOUND_FILE;
        *templ = NONE;
      }
    } else {
      *templ = NONE;
    }
  } else {
    *templ = NONE;
  }

  *file_buffer = read_file(file_path, file_size);
  if (file_buffer == NULL) {
    return IO_ERROR;
  }

  return IO_OK;
}
