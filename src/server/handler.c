#include "handler.h"
#include "../utils/array.h"
#include "../utils/cellmap.h"
#include "../utils/content_type.h"
#include "../utils/hour_util.h"
#include "../utils/logger.h"
#include "../utils/str_replace.h"
#include "router.h"
#include <ctype.h>
#include <errno.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int callback(CellMap **cellmap, int argc, char **argv, char **azColName);
int get_wards(sqlite3 *db, char **list, void *callback);
int get_teachers(sqlite3 *db, char **list, void *callback);
int get_classrooms(sqlite3 *db, char **list, void *callback);
int get_date(sqlite3 *db, char **generated_date, char **effective_date,
             void *callback);
int get_select(sqlite3 *db, char **select);
int get_dates(DbCacheArray *db_cache, char **list, char *data);

int ward_list_callback(void *data, int argc, char **argv, char **az_col_name) {
  char newString[200];
  sprintf(newString, WARDS_LI_ITEM, argv[0]);
  appendstr((char **)data, newString);
  return 0;
}

int teacher_list_callback(void *data, int argc, char **argv,
                          char **az_col_name) {
  char newString[200];
  sprintf(newString, TEACHERS_LI_ITEM, argv[1], argv[0]);
  appendstr((char **)data, newString);
  return 0;
}

int classroom_list_callback(void *data, int argc, char **argv,
                            char **az_col_name) {
  char newString[200];
  sprintf(newString, CLASSROOM_LI_ITEM, argv[0]);
  appendstr((char **)data, newString);
  return 0;
}

int date_callback(void *data, int argc, char **argv, char **az_col_name) {
  char newString[100];
  sprintf(newString, "%s|%s", argv[0], argv[1]);
  appendstr((char **)data, newString);
  return 0;
}

// It is a great implementation of a counter don't judge me
Error update_count(char *path, int *new_count) {
  FILE *file_ptr = fopen(path, "r+");

  if (file_ptr == NULL) {
    print_debug("%s file doesn't exits", path);
    file_ptr = fopen(path, "w+");

    if (file_ptr == NULL) {
      print_error("Failed to create %s", path);
      return IO_ERROR;
    }

    fprintf(file_ptr, "%d", 0);
    fseek(file_ptr, 0, SEEK_SET);
  }

  int count;
  if (fscanf(file_ptr, "%d", &count) != 1) {
    print_error("Failed to read count value");
    fclose(file_ptr);
    return IO_ERROR;
  }

  *new_count = ++count;
  fseek(file_ptr, 0, SEEK_SET);
  fprintf(file_ptr, "%d", count);
  fclose(file_ptr);

  return IO_OK;
}

Error handle_client(int client_socket, DbCacheArray *db_cache,
                    struct sockaddr_in client) {
  char buffer[2048];
  int read_size;

  memset(buffer, 0, sizeof(buffer));

  read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
  if (read_size < 0) {
    print_error("Recv failed: %s", strerror(errno));
    close(client_socket);
    return WEB_SERVER_ERROR;
  }

  char *method = strtok(buffer, " ");
  char *path = strtok(NULL, " ");
  strtok(NULL, " ");

  char *query_string = NULL;
  char *path_only = strtok(path, "?");
  query_string = strtok(NULL, "?");
  char *key, *value;

  int shortened = 0;
  sqlite3 *db = NULL;

  if (query_string != NULL) {
    char *token;
    char *outer_saveptr = NULL;
    char *inner_saveptr = NULL;
    token = strtok_r(query_string, "&", &outer_saveptr);
    while (token != NULL) {
      key = strtok_r(token, "=", &inner_saveptr);
      value = strtok_r(NULL, "=", &inner_saveptr);

      if (strcmp(key, "short") == 0) {
        shortened = atoi(value);
      }

      if (strcmp(key, "date") == 0) {
        for (int i = 0; i < db_cache->count; ++i) {
          if (strcmp(value, db_cache->array[i].date) == 0) {
            db = db_cache->array[i].db;
          }
        }
      }

      token = strtok_r(NULL, "&", &outer_saveptr);
    };
  }

  if (db == NULL) {
    if (db_cache->count <= 0) {
      print_error("Failed to select database");
      close(client_socket);
      return WEB_SERVER_ERROR;
    }
    db = db_cache->array[db_cache->count - 1].db;
  }

  print_info("Connection accepted from %s:%d %s %s", inet_ntoa(client.sin_addr),
             ntohs(client.sin_port), method, path);

  if (path == NULL) {
    print_error("Path is null");
    close(client_socket);
    return WEB_SERVER_ERROR;
  }

  char *file_buffer;
  long file_size;
  Template templ;
  char *id = NULL;

  get_template(path, &file_buffer, &file_size, &templ, &id);
  char *id_decoded = NULL;
  if (id != NULL) {
    id_decoded = malloc(strlen(id) + 1);
    urldecode2(id_decoded, id);
  }

  int count = 0;

  if (templ != NONE) {
    update_count("./count.txt", &count);
  }

  if (templ == WARD || templ == TEACHER || templ == CLASSROOM) {
    char *res = NULL;
    fetch_table(db, &res, templ, id_decoded, shortened);

    char *wards_list = NULL;
    get_wards(db, &wards_list, ward_list_callback);

    char *teachers_list = NULL;
    get_teachers(db, &teachers_list, teacher_list_callback);

    char *classrooms_list = NULL;
    get_classrooms(db, &classrooms_list, classroom_list_callback);

    char *generated_date = NULL;
    char *effective_date = NULL;
    get_date(db, &generated_date, &effective_date, date_callback);

    char *dates = "\0";
    get_dates(db_cache, &dates, DATES_LI);

    ReplacePair replacements[] = {
        {"%res%", res ? res : ""},
        {"%title%", res ? id_decoded : "Not Found"},
        {"%dates%", dates},
        {"%effective%", effective_date},
        {"%generated%", generated_date},
        {"%wards%", wards_list},
        {"%teachers%", teachers_list},
        {"%classrooms%", classrooms_list},
    };

    str_replace_multiple(&file_buffer, replacements,
                         sizeof(replacements) / sizeof(ReplacePair));

    free(wards_list);
    free(teachers_list);
    free(classrooms_list);
    free(generated_date);
    free(effective_date);
    free(dates);
    free(res);
  } else if (templ == INDEX) {
    char *select = "\0";
    get_select(db, &select);

    char str[24];
    sprintf(str, "%d", count);

    char *dates = "\0";
    get_dates(db_cache, &dates, DATES_DUMP_LI);

    ReplacePair replacements[] = {
        {"%select%", select},
        {"%count%", str},
        {"%dates%", dates},
    };

    str_replace_multiple(&file_buffer, replacements,
                         sizeof(replacements) / sizeof(ReplacePair));

    free(select);
    free(dates);
  } else if (templ == DATABASE) {
    struct stat file_stat;

    char filename[20];
    sprintf(filename, "./backup/%s.db", id_decoded);

    if (stat(filename, &file_stat) < 0) {
      const char *error_message = "Database dump not found!";
      send(client_socket, error_message, strlen(error_message), 0);
      return WEB_SERVER_OK;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
      print_error("File not found: %s", strerror(errno));
      return WEB_SERVER_OK;
    }

    char header[128];
    sprintf(header,
            "HTTP/1.1 200 OK\r\nContent-Type: "
            "application/x-sqlite3\r\nContent-Disposition: attachment; "
            "filename=\"%s.db\"\r\n\r\n",
            id_decoded);
    write(client_socket, header, strlen(header));

    char buffer[1024];
    size_t bytes_read;
    ssize_t bytes_written;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
      bytes_written = write(client_socket, buffer, bytes_read);
      if (bytes_written == -1) {
        print_error("Error writing to client socket: %s", strerror(errno));
        break;
      }
    }

    fclose(file);

    free(file_buffer);
    free(id_decoded);
    if (id != NULL) {
      free(id);
    }
    return WEB_SERVER_OK;
  }

  respond_http(client_socket, &file_buffer, strlen(file_buffer),
               get_content_type(path_to_file(path)));

  free(file_buffer);
  free(id_decoded);
  if (id != NULL) {
    free(id);
  }
  return WEB_SERVER_OK;
}

void respond_http(int client_socket, char **html, long file_size,
                  char *content_type) {
  char response_header[256];
  snprintf(response_header, sizeof(response_header),
           "HTTP/1.1 200 OK\r\nContent-Type: %s; "
           "charset=UTF-8\r\nContent-Length: %lu\r\n\r\n",
           content_type, file_size);
  write(client_socket, response_header, strlen(response_header));

  write(client_socket, *html, file_size);
}

typedef struct StringCache_t {
  char *str;
  size_t size;
  size_t used;
} StringCache;

void append_str(StringCache *cache, const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Calculate the needed size
  size_t required_size = vsnprintf(NULL, 0, format, args) + 1;
  va_end(args);

  if (cache->used + required_size > cache->size) {
    // Increase the size of the buffer
    cache->size = (cache->size + required_size) * 2;
    cache->str = realloc(cache->str, cache->size);
    if (cache->str == NULL) {
      print_error("Failed to reallocate memory: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  va_start(args, format);
  vsnprintf(cache->str + cache->used, required_size, format, args);
  va_end(args);

  cache->used += required_size - 1;
}

Error fetch_table(sqlite3 *db, char **response, Template templ, char *id,
                  int shortened) {
  char *sql;
  switch (templ) {
  case WARD:
    sql = "SELECT \"order\", hours, lesson_name, teacher_id, "
          "classroom, weekday, class_id FROM timetable WHERE class_id = ? "
          "ORDER BY \"order\" ASC, weekday ASC";
    break;
  case TEACHER:
    sql = "SELECT \"order\", hours, lesson_name, teacher_id, "
          "classroom, weekday, class_id FROM timetable WHERE teacher_id = ? "
          "ORDER BY \"order\" ASC, weekday ASC";
    break;
  case CLASSROOM:
    sql = "SELECT \"order\", hours, lesson_name, teacher_id, "
          "classroom, weekday, class_id FROM timetable WHERE classroom = ? "
          "ORDER BY \"order\" ASC, weekday ASC";
    break;
  default:
    return WEB_SERVER_ERROR;
  }
  CellMap *cellmap = cellmap_init();

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    print_error("Failed to select data");
    sqlite3_close(db);
    return 1;
  }

  if (templ == WARD) {
    for (int i = 0; i < 2; ++i) {
      id[i] = toupper(id[i]);
    }
  }

  sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    int argc = sqlite3_column_count(stmt);
    char *argv[argc];
    char *azColName[argc];

    // Collect column names and values
    for (int i = 0; i < argc; i++) {
      argv[i] = (char *)sqlite3_column_text(stmt, i);
      azColName[i] = (char *)sqlite3_column_name(stmt, i);
    }

    // Invoke the callback with current row
    callback(&cellmap, argc, argv, azColName);
  }

  if (rc != SQLITE_DONE) {
    print_error("Failed to execute statement");
  }

  sqlite3_finalize(stmt);

  int item_count = cellmap->len;
  if (item_count == 0) {
    return WEB_SERVER_ERROR;
  }

  Item *items = malloc(item_count * sizeof(Item));

  cellmap_collect(cellmap, items, &item_count);
  qsort(items, item_count, sizeof(Item), compare_cells);

  StringCache cache = {.str = malloc(1024), .size = 1024, .used = 0};
  cache.str[0] = '\0';

  for (int i = 1; i < items[0].key.x; ++i) {
    // Add time cells
    if (shortened == 0) {
      append_str(&cache, HOUR_ROW, i, order_to_hour(i));
    } else {
      append_str(&cache, HOUR_ROW, i, order_to_hour_shortened(i));
    }

    // Append missing cells at start
    for (int j = 0; j < 5; ++j) {
      append_str(&cache, EMPTY_TD);
    }
    append_str(&cache, TR_CLOSE);
  }

  for (int i = 0; i < item_count; ++i) {
    int item_x;
    if (i - 1 < 0) {
      item_x = 99;
    } else {
      item_x = items[i - 1].key.x;
    }

    // Start new line
    if (item_x != items[i].key.x) {
      append_str(&cache, TR_OPEN);
    }

    LessonArray cell_array = items[i].val;
    // Add time cells
    if (item_x != items[i].key.x) {
      if (shortened == 0) {
        append_str(&cache, HOUR_CELL, cell_array.array[0].order,
                   order_to_hour(cell_array.array[0].order));
      } else {
        append_str(&cache, HOUR_CELL, cell_array.array[0].order,
                   order_to_hour_shortened(cell_array.array[0].order));
      }
      for (int j = 0; j < items[i].key.y; ++j) {
        append_str(&cache, EMPTY_TD);
      }
    }

    append_str(&cache, TD_OPEN);
    for (int j = 0; j < cell_array.count; ++j) {
      switch (templ) {
      case WARD:
        append_str(&cache, WARD_PLAN_CELL, cell_array.array[j].lesson_name,
                   cell_array.array[j].teacher_id,
                   cell_array.array[j].classroom);
        break;
      case TEACHER:
        append_str(&cache, TEACHER_PLAN_CELL, cell_array.array[j].lesson_name,
                   cell_array.array[j].classroom, cell_array.array[j].class_id);
        break;
      case CLASSROOM:
        append_str(&cache, CLASSROOM_PLAN_CELL, cell_array.array[j].lesson_name,
                   cell_array.array[j].teacher_id,
                   cell_array.array[j].class_id);
        break;
      default:
        append_str(&cache, WARD_PLAN_CELL, cell_array.array[j].lesson_name,
                   cell_array.array[j].teacher_id,
                   cell_array.array[j].classroom);
        break;
      }
    }
    append_str(&cache, TD_CLOSE);

    // Fill missing cells
    if (i + 1 < item_count && items[i].key.y + 1 != items[i + 1].key.y &&
        items[i].key.y != 4 && items[i + 1].key.y <= 4) {
      for (int j = items[i].key.y; j < items[i + 1].key.y - 1; ++j) {
        append_str(&cache, EMPTY_TD);
      }
    }

    // Add empty/closing tags at the end
    // NOTE: items[i + 1] results in invalid read but i really don't care enough
    // to fix it. If you want you can
    if (items[i].key.x != items[i + 1].key.x) {
      // Fill missing cells
      for (int j = items[i].key.y; j < 4; ++j) {
        append_str(&cache, EMPTY_TD);
      }
      append_str(&cache, TR_CLOSE);
    }
  }

  *response = strdup(cache.str);
  free(cache.str);
  free(items);
  cellmap_free(cellmap);
  return WEB_SERVER_OK;
}

int callback(CellMap **cellmap, int argc, char **argv, char **az_col_name) {
  Error err;

  Lesson lesson = {
      .order = atoi(argv[0]),
      .hours = argv[1] ? strdup(argv[1]) : NULL,
      .lesson_name = argv[2] ? strdup(argv[2]) : NULL,
      .teacher_id = argv[3] ? strdup(argv[3]) : NULL,
      .classroom = argv[4] ? strdup(argv[4]) : NULL,
      .weekday = atoi(argv[5]),
      .class_id = argv[6] ? strdup(argv[6]) : NULL,
  };

  Cell cell = {.x = lesson.order, .y = lesson.weekday};

  LessonArray out;

  err = cellmap_get(*cellmap, cell, &out);
  if (err != HASHMAP_OPERATION_OK) {
    LessonArray new;
    arrayInit(&new, 8);
    arrayPush(&new, lesson);
    cellmap_set(*cellmap, cell, new);
  } else {
    cellmap_insert_or_push(*cellmap, cell, out, lesson);
  }

  return 0;
}

void urldecode2(char *dst, const char *src) {
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

int get_wards(sqlite3 *db, char **list, void *callback) {
  int rc = sqlite3_exec(db, "SELECT \"full\" FROM wards", callback, list, 0);
  if (rc != SQLITE_OK) {
    print_error("Failed to select data");
    sqlite3_close(db);
    return 1;
  }

  return 0;
}

int get_teachers(sqlite3 *db, char **list, void *callback) {
  int rc = sqlite3_exec(db, "SELECT \"name\", \"initials\" FROM teachers",
                        callback, list, 0);
  if (rc != SQLITE_OK) {
    print_error("Failed to select data");
    sqlite3_close(db);
    return 1;
  }

  return 0;
}

int get_classrooms(sqlite3 *db, char **list, void *callback) {
  int rc = sqlite3_exec(
      db,
      "SELECT DISTINCT classroom FROM timetable WHERE "
      "classroom != \"\" ORDER BY classroom + 0 ASC, classroom ASC",
      callback, list, 0);
  if (rc != SQLITE_OK) {
    print_error("Failed to select data");
    sqlite3_close(db);
    return 1;
  }

  return 0;
}

int get_date(sqlite3 *db, char **generated_date, char **effective_date,
             void *callback) {
  char *dates = NULL;
  int rc =
      sqlite3_exec(db, "SELECT generated, effective_from FROM date LIMIT 1",
                   callback, &dates, 0);
  if (rc != SQLITE_OK) {
    print_error("Failed to select data");
    sqlite3_close(db);
    return 1;
  }

  *generated_date = strdup(strtok(dates, "|"));
  *effective_date = strdup(strtok(NULL, "|"));

  free(dates);
  return 0;
}

int select_ward_list_callback(void *data, int argc, char **argv,
                              char **az_col_name) {
  if (argv[0] == NULL) {
    return 1;
  }

  char newString[100];
  sprintf(newString, WARDS_OPTION_ITEM, argv[0]);
  appendstr((char **)data, newString);
  return 0;
}

int select_teacher_list_callback(void *data, int argc, char **argv,
                                 char **az_col_name) {
  if (argv[0] == NULL || argv[1] == NULL) {
    return 1;
  }
  char newString[100];
  snprintf(newString, sizeof(newString), TEACHERS_OPTION_ITEM, argv[1],
           argv[0]);
  appendstr((char **)data, newString);
  return 0;
}

int select_classroom_list_callback(void *data, int argc, char **argv,
                                   char **az_col_name) {
  if (argv[0] == NULL) {
    return 1;
  }
  char **arr = (char **)data;
  char newString[100];
  snprintf(newString, sizeof(newString), CLASSROOM_OPTION_ITEM, argv[0]);
  appendstr((char **)data, newString);
  return 0;
}

int get_select(sqlite3 *db, char **select) {
  StringCache res = {.str = malloc(1024), .size = 1024, .used = 0};
  res.str[0] = '\0';

  char *wards_list = NULL;
  get_wards(db, &wards_list, select_ward_list_callback);

  char *teachers_list = NULL;
  get_teachers(db, &teachers_list, select_teacher_list_callback);

  char *classrooms_list = NULL;
  get_classrooms(db, &classrooms_list, select_classroom_list_callback);

  append_str(&res, "<optgroup label=\"Odziały\" class=\"text-gray-400\">");
  append_str(&res, wards_list);
  append_str(&res, "</optgroup>");
  append_str(&res, "<optgroup label=\"Nauczyciele\" class=\"text-gray-400\">");
  append_str(&res, teachers_list);
  append_str(&res, "</optgroup>");
  append_str(&res, "<optgroup label=\"Sale\" class=\"text-gray-400\">");
  append_str(&res, classrooms_list);
  append_str(&res, "</optgroup>");

  *select = strdup(res.str);

  free(wards_list);
  free(teachers_list);
  free(classrooms_list);
  free(res.str);
  return 0;
}

int get_dates(DbCacheArray *db_cache, char **list, char *data) {
  StringCache cache = {.str = malloc(1024), .size = 1024, .used = 0};
  cache.str[0] = '\0';

  for (int i = 0; i < db_cache->count; ++i) {
    append_str(&cache, data, db_cache->array[i].date);
  }

  *list = strdup(cache.str);

  free(cache.str);
  return 0;
}
