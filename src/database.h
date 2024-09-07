#ifndef DATABASE_H
#define DATABASE_H

#include "error.h"
#include <sqlite3.h>

Error sqliteResult(sqlite3 *db, int rc, const char *successMessage);

Error createDatabase(sqlite3 *db);

#endif // !DATABASE_H
