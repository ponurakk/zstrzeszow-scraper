#include <stdio.h>
#include <string.h>

char *path_to_file(char *path) {
  // Debug print statements
  printf("PATH: '%s', %i, %i\n", path, strcmp(path, "/style.css"),
         strcmp(path, "/") == 0);

  // Use strcmp to compare strings
  if (strcmp(path, "/") == 0) {
    return "views/index.html";
  } else if (strcmp(path, "/style.css") == 0) {
    return "views/assets/style.css";
  }

  return "views/index.html";
}
