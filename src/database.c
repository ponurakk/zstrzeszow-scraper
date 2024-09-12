#include "array.h"
#include "error.h"
#include "list.h"
#include <sqlite3.h>
#include <stdio.h>

Error sqlite_result(sqlite3 *db, int rc, const char *success_message) {
  if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW) {
    if (success_message != NULL) {
      fprintf(stderr, "INFO: %s\n", success_message);
    }
    return SQLITE_SUCCESS;
  } else {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
  }

  return SQLITE_ERROR;
}

Error create_wards_table(sqlite3 *db) {
  const char *create_table = "CREATE TABLE IF NOT EXISTS \"wards\" ("
                             "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "\"class_id\" VARCHAR NOT NULL,"
                             "\"full\" VARCHAR NOT NULL,"
                             "\"class_number\" INTEGER NOT NULL,"
                             "\"tag\" VARCHAR NOT NULL"
                             ");";

  int rc = sqlite3_exec(db, create_table, 0, 0, 0);

  if (sqlite_result(db, rc, "Created wards table") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  return SQLITE_SUCCESS;
}

Error create_teachers_table(sqlite3 *db) {
  const char *create_table = "CREATE TABLE IF NOT EXISTS \"teachers\" ("
                             "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "\"teacher_id\" VARCHAR NOT NULL,"
                             "\"name\" VARCHAR NOT NULL,"
                             "\"initials\" VARCHAR NOT NULL"
                             ");";

  int rc = sqlite3_exec(db, create_table, 0, 0, 0);

  if (sqlite_result(db, rc, "Created teachers table") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  return SQLITE_SUCCESS;
}

Error create_timetable_table(sqlite3 *db) {
  const char *create_table = "CREATE TABLE IF NOT EXISTS \"timetable\" ("
                             "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "\"class_id\" VARCHAR,"
                             "\"teacher_id\" VARCHAR,"
                             "\"order\" INTEGER NOT NULL,"
                             "\"hours\" VARCHAR NOT NULL,"
                             "\"lesson_name\" VARCHAR NOT NULL,"
                             "\"classroom\" VARCHAR,"
                             "\"weekday\" INTEGER NOT NULL"
                             ");";

  int rc = sqlite3_exec(db, create_table, 0, 0, 0);

  if (sqlite_result(db, rc, "Created timetable table") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  return SQLITE_SUCCESS;
}

Error create_database(sqlite3 *db) {
  if (create_wards_table(db) != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  if (create_teachers_table(db) != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  if (create_timetable_table(db) != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  return SQLITE_SUCCESS;
}

Error add_ward(sqlite3 *db, Ward ward) {
  const char *sql = "INSERT INTO wards(class_id, full, class_number, tag) "
                    "VALUES (?, ?, ?, ?)";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
    return SQLITE_ERROR;
  }

  sqlite3_bind_text(stmt, 1, ward.id, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, ward.full, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 3, ward.class_number);
  sqlite3_bind_text(stmt, 4, ward.tag, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return SQLITE_ERROR;
  }

  sqlite3_finalize(stmt);

  return SQLITE_SUCCESS;
}

Error add_teacher(sqlite3 *db, Teacher teacher) {
  const char *sql = "INSERT INTO teachers(teacher_id, name, initials) "
                    "VALUES (?, ?, ?)";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
    return SQLITE_ERROR;
  }

  sqlite3_bind_text(stmt, 1, teacher.id, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, teacher.name, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, teacher.initials, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return SQLITE_ERROR;
  }

  sqlite3_finalize(stmt);

  return SQLITE_SUCCESS;
}

Error add_lesson(sqlite3 *db, Lesson lesson) {
  const char *sql =
      "INSERT INTO timetable(class_id, teacher_id, \"order\", hours, "
      "lesson_name, classroom, weekday) "
      "VALUES (?,?,?,?,?,?,?)";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
    return SQLITE_ERROR;
  }

  sqlite3_bind_text(stmt, 1, lesson.class_id, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, lesson.teacher_id, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 3, lesson.order);
  sqlite3_bind_text(stmt, 4, lesson.hours, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 5, lesson.lesson_name, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 6, lesson.classroom, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 7, lesson.weekday);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "ERROR: SQL error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return SQLITE_ERROR;
  }

  sqlite3_finalize(stmt);

  return SQLITE_SUCCESS;
}
