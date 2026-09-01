#ifndef DATABASE_H
#define DATABASE_H

#include "scraper/list.h"
#include "utils/array.h"
#include "utils/error.h"
#include <sqlite3.h>

Error sqlite_result(sqlite3 *db, int rc, const char *successMessage);

Error create_database(sqlite3 *db);

Error add_ward(sqlite3 *db, Ward ward);
Error add_teacher(sqlite3 *db, Teacher teacher);
Error add_lesson(sqlite3 *db, Lesson lesson);
Error add_date(sqlite3 *db, char *effective_from, char *generated);

#endif // !DATABASE_H
