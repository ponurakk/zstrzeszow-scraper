#ifndef LISTENER_H
#define LISTENER_H
#include "../utils/array.h"
#include "../utils/error.h"
#include <sqlite3.h>

Error server(DbCacheArray *db);

#endif // !LISTENER_H
