#ifndef ROUTER_H
#define ROUTER_H

#include "../utils/error.h"

typedef enum Template_t {
  NONE = 0,
  WARD = 1,
  TEACHER = 2,
  CLASSROOM = 3
} Template;

char *path_to_file(char *path);
Error get_template(char *path, char **file_buffer, long *file_size,
                   Template *templ, char **res);

#endif // !ROUTER_H
