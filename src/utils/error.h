#ifndef ERROR_H
#define ERROR_H

typedef enum {
  SQLITE_SUCCESS = 0,
  SQLITE_ERROR = 1,
  TIMETABLE_OK = 2,
  TIMETABLE_ERROR = 3,
  SCRAPER_OK = 4,
  SCRAPER_ERROR = 5,
  WEB_SERVER_OK = 6,
  WEB_SERVER_ERROR = 7,
  HASHMAP_OPERATION_OK = 8,
  HASHMAP_NULL = 9,
  IO_OK = 10,
  IO_ERROR = 11,
} Error;

const char *error_to_string(Error err);

#endif // !ERROR_H
