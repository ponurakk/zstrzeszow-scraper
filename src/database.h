#ifndef DATABASE_H
#define DATABASE_H

#include "utils/array.h"
#include "utils/error.h"
#include <sqlite3.h>

typedef struct {
  // o1
  char *id;
  // 1AT
  char *full;
  // 1
  int class_number;
  // AT
  char *tag;
} Ward;

typedef struct {
  // n1
  char *id;
  // Ni
  char *initials;
  // N.Surname
  char *name;
} Teacher;

Error sqlite_result(sqlite3 *db, int rc, const char *successMessage);

Error create_database(sqlite3 *db);

Error add_ward(sqlite3 *db, Ward ward);
Error add_teacher(sqlite3 *db, Teacher teacher);
Error add_lesson(sqlite3 *db, Lesson lesson);
Error add_date(sqlite3 *db, char *effective_from, char *generated);

#endif // !DATABASE_H
