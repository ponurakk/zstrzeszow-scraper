#include "../utils/error.h"
#include "router.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void respond_http(int client_socket, char *html, long file_size);
char *read_file(const char *filename, long *file_size);
Error handle_client(int client_socket);

Error handle_client(int client_socket) {
  char buffer[2048];
  int read_size;

  memset(buffer, 0, sizeof(buffer));

  read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
  if (read_size < 0) {
    perror("Recv failed");
    close(client_socket);
    return WEB_SERVER_ERROR;
  }

  char *method = strtok(buffer, " ");
  char *path = strtok(NULL, " ");
  strtok(NULL, " ");

  if (path == NULL) {
    printf("Path is null\n");
    close(client_socket);
    return WEB_SERVER_ERROR;
  }
  char *file_buffer;
  long file_size;
  get_html(path, &file_buffer, &file_size);

  respond_http(client_socket, file_buffer, file_size);

  return WEB_SERVER_OK;
}

void respond_http(int client_socket, char *html, long file_size) {
  char response[file_size + 100];
  snprintf(response, sizeof(response),
           "HTTP/1.1 200 OK\r\nContent-Length: %lu"
           "charset=UTF-8\r\n\r\n%s",
           strlen(html), html);
  write(client_socket, response, strlen(response));
}
