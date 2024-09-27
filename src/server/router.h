#ifndef ROUTER_H
#define ROUTER_H

#include "../utils/error.h"

#define INDEX_FILE "views/src/index.html"
#define PLAN_FILE "views/src/plan.html"
#define STYLES_FILE "views/src/assets/styles.css"
#define NOT_FOUND_FILE "views/src/404.html"

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
