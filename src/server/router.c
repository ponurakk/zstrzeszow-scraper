#include "router.h"
#include "../utils/error.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *path_to_file(char *path) {
  print_info("PATH: '%s', %i, %i", path, strcmp(path, "/style.css"),
             strcmp(path, "/") == 0);

  if (strcmp(path, "/") == 0) {
    return "views/src/index.html";
  } else if (strcmp(path, "/style.css") == 0) {
    return "views/src/assets/style.css";
  }

  return "views/src/404.html";
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
      if (strcmp(split, "o") == 0) {
        *templ = WARD;
        file_path = "views/src/index.html";
        printf("ward %s\n", number);
      } else if (strcmp(split, "n") == 0) {
        *templ = TEACHER;
        file_path = "views/src/index.html";
        printf("teacher %s\n", number);
      } else if (strcmp(split, "s") == 0) {
        *templ = CLASSROOM;
        file_path = "views/src/index.html";
        printf("classroom %s\n", number);
      } else {
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
