#ifndef LISTENER_H
#define LISTENER_H
#include "../utils/error.h"
#include <sqlite3.h>

Error server(sqlite3 *db);

#endif // !LISTENER_H
