#include "../utils/error.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void respond_with_html(int client_socket, char *html);
char *read_file(const char *filename, long *file_size);
Error handle_client(int client_socket);

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

Error handle_client(int client_socket) {
  FILE *file;
  char buffer[2048];
  char *file_buffer;
  int read_size;
  long file_size;

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

  file_buffer = read_file("views/index.html", &file_size);
  if (file_buffer == NULL) {
    close(client_socket);
    return WEB_SERVER_ERROR;
  }

  respond_with_html(client_socket, file_buffer);

  free(file_buffer);

  return WEB_SERVER_OK;
}

void respond_with_html(int client_socket, char *html) {
  char response[2048];
  snprintf(response, sizeof(response),
           "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\nContent-Type: "
           "text/html;charset=UTF-8\r\n\r\n%s",
           strlen(html), html);
  write(client_socket, response, strlen(response));
}
