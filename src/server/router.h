#ifndef ROUTER_H
#define ROUTER_H

#include "../utils/error.h"

char *path_to_file(char *path);
Error get_template(char *file_path, char **file_buffer, long *file_size);

#endif // !ROUTER_H
