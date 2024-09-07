#ifndef DATABASE_H
#define DATABASE_H

#include "error.h"
#include "list.h"
#include <sqlite3.h>

Error sqliteResult(sqlite3 *db, int rc, const char *successMessage);

Error createDatabase(sqlite3 *db);

Error addWard(sqlite3 *db, Ward ward);
Error addTeacher(sqlite3 *db, Teacher teacher);

#endif // !DATABASE_H
