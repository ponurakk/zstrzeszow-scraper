#include "error.h"
#include <sqlite3.h>
#include <stdio.h>

Error sqliteResult(sqlite3 *db, int rc, const char *successMessage) {
  if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW) {
    if (successMessage != NULL) {
      fprintf(stderr, "INFO: %s\n", successMessage);
    }
    return SQLITE_SUCCESS;
  } else {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
  }

  return SQLITE_ERROR;
}

Error createWardsTable(sqlite3 *db) {
  const char *createTable = "CREATE TABLE IF NOT EXISTS \"wards\" ("
                            "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "\"class_id\" VARCHAR NOT NULL,"
                            "\"full\" VARCHAR NOT NULL,"
                            "\"class_number\" INTEGER NOT NULL,"
                            "\"tag\" VARCHAR NOT NULL"
                            ");";

  int rc = sqlite3_exec(db, createTable, 0, 0, 0);

  if (sqliteResult(db, rc, "Created wards table") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  return SQLITE_SUCCESS;
}

Error createTeachersTable(sqlite3 *db) {
  const char *createTable = "CREATE TABLE IF NOT EXISTS \"teachers\" ("
                            "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "\"teacher_id\" VARCHAR NOT NULL,"
                            "\"name\" VARCHAR NOT NULL,"
                            "\"initials\" VARCHAR NOT NULL"
                            ");";

  int rc = sqlite3_exec(db, createTable, 0, 0, 0);

  if (sqliteResult(db, rc, "Created teachers table") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  return SQLITE_SUCCESS;
}

Error createDatabase(sqlite3 *db) {
  int rc = sqlite3_open("./sqlite.db", &db);

  if (sqliteResult(db, rc, "Opened database successfully") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  if (createWardsTable(db) != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  if (createTeachersTable(db) != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  sqlite3_close(db);
  return SQLITE_SUCCESS;
}
