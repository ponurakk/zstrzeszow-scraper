#ifndef HANDLER_H
#define HANDLER_H
#include "../utils/error.h"

void respond_with_html(int client_socket, char *html);
char *read_file(const char *filename, long *file_size);
Error handle_client(int client_socket);

#endif // !HANDLER_H
