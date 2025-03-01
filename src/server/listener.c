#include "listener.h"
#include "../utils/array.h"
#include "../utils/error.h"
#include "../utils/logger.h"
#include "handler.h"
#include <arpa/inet.h>
#include <errno.h>
#include <sqlite3.h>
#include <string.h>
#include <unistd.h>

Error server(DbCacheArray *db) {
  Error err;
  int socket_desc;
  struct sockaddr_in server;

  // Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1) {
    print_error("Could not create socket: %s", strerror(errno));
    return WEB_SERVER_ERROR;
  }

  int opt = 1;
  if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    print_error("setsockopt(SO_REUSEADDR) failed: %s", strerror(errno));
    return WEB_SERVER_ERROR;
  }

  // Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(LISTEN_PORT);

  // Bind
  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
    print_error("Bind failed: %s", strerror(errno));
    return WEB_SERVER_ERROR;
  }

  // Listen
  if (listen(socket_desc, 3) < 0) {
    print_error("Listen Failed: %s", strerror(errno));
    return WEB_SERVER_ERROR;
  }

  print_info("Server listening on :%i", LISTEN_PORT);
  int c = sizeof(struct sockaddr_in);
  int new_socket;
  struct sockaddr_in client;
  while ((new_socket = accept(socket_desc, (struct sockaddr *)&client,
                              (socklen_t *)&c))) {
    if (new_socket < 0) {
      print_error("Accept failed: %s", strerror(errno));
      continue;
    }

    err = handle_client(new_socket, db, client);
    if (err != WEB_SERVER_OK) {
      close(new_socket);
      break;
    };

    close(new_socket);
  }

  if (new_socket < 0) {
    print_error("Accept failed: %s", strerror(errno));
  }

  close(socket_desc);

  return WEB_SERVER_OK;
}
