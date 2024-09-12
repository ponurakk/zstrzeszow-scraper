#ifndef ERROR_H
#define ERROR_H

typedef enum {
  SQLITE_SUCCESS = 0,
  SQLITE_ERROR = 1,
  TIMETABLE_OK = 2,
  TIMETABLE_ERROR = 3,
  SCRAPER_OK = 4,
  SCRAPER_ERROR = 5,
} Error;

const char *error_to_string(Error err);

#endif // !ERROR_H
