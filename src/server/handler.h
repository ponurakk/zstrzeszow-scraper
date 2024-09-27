#ifndef HANDLER_H
#define HANDLER_H
#include "../utils/error.h"
#include "router.h"
#include <arpa/inet.h>
#include <sqlite3.h>

#define WARDS_LI_ITEM "<li><a class=\"uk-link\" href=\"/o/%1$s\">%1$s</a></li>"
#define TEACHERS_LI_ITEM "<li><a class=\"uk-link\" href=\"/n/%s\">%s</a></li>"
#define CLASSROOM_LI_ITEM                                                      \
  "<li><a class=\"uk-link\" href=\"/s/%1$s\">%1$s</a></li>"

#define HOUR_ROW                                                               \
  "<tr class=\"border-b border-gray\"> <td class=\"py-4 px-6\">%i</td><td "    \
  "class=\"py-4 px-6\">%s</td>"
#define HOUR_CELL                                                              \
  "<td class=\"py-4 px-6\">%i</td><td class=\"py-4 px-6\">%s</td>"
#define EMPTY_TD "<td class=\"py-4 px-6\"></td>"
#define CELL                                                                   \
  "<span>%1$s <a href=\"/n/%2$s\" class=\"uk-link\">%2$s</a> <a "              \
  "href=\"/s/%3$s\" class=\"uk-link\">%3$s</a></span><br/>"

#define TR_OPEN "<tr class=\"border-b border-gray\">"
#define TR_CLOSE "</tr>"

#define TD_OPEN "<td class=\"py-4 px-6\">"
#define TD_CLOSE "</td>"

void urldecode2(char *dst, const char *src);
void respond_http(int client_socket, char **html, long file_size);
char *read_file(const char *filename, long *file_size);
Error fetch_table(sqlite3 *db, char **res, Template templ, char *number);
Error handle_client(int client_socket, sqlite3 *db, struct sockaddr_in client);

#endif // !HANDLER_H
