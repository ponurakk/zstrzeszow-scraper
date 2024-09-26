#ifndef HANDLER_H
#define HANDLER_H
#include "../utils/error.h"
#include <arpa/inet.h>
#include <sqlite3.h>

void respond_http(int client_socket, char **html, long file_size);
char *read_file(const char *filename, long *file_size);
Error handle_client(int client_socket, sqlite3 *db, struct sockaddr_in client);

#endif // !HANDLER_H
