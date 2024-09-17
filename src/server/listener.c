#include "../utils/error.h"
#include "../utils/logger.h"
#include "handler.h"
#include <arpa/inet.h>
#include <sqlite3.h>
#include <stdio.h>
#include <unistd.h>

Error server(sqlite3 *db) {
  Error err;
  int socket_desc;
  struct sockaddr_in server;

  // Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1) {
    perror("Could not create socket");
    return WEB_SERVER_ERROR;
  }

  int opt = 1;
  if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    return WEB_SERVER_ERROR;
  }

  // Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(3000);

  // Bind
  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("Bind failed\n");
    return WEB_SERVER_ERROR;
  }

  // Listen
  if (listen(socket_desc, 3) < 0) {
    perror("Listen Failed");
    return WEB_SERVER_ERROR;
  }

  print_info("Server listening on :%i", 3000);
  int c = sizeof(struct sockaddr_in);
  int new_socket;
  struct sockaddr_in client;
  while ((new_socket = accept(socket_desc, (struct sockaddr *)&client,
                              (socklen_t *)&c))) {
    if (new_socket < 0) {
      perror("Accept failed");
      continue;
    }

    print_info("Connection accepted from %s:%d", inet_ntoa(client.sin_addr),
               ntohs(client.sin_port));

    err = handle_client(new_socket, db);
    if (err != WEB_SERVER_OK) {
      continue;
    };

    close(new_socket);
  }

  if (new_socket < 0) {
    perror("accept failed");
  }

  close(socket_desc);

  return WEB_SERVER_OK;
}
