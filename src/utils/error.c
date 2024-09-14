#include "error.h"

void (*globalErrorCallback)(const char *);

const char *error_to_string(Error err) {
  switch (err) {
  case SQLITE_SUCCESS:
    return "OK: Operation completed successfully";
  case SQLITE_ERROR:
    return "ERROR: SQLite error";
  case TIMETABLE_OK:
    return "OK: Successfully parsed timetable";
  case TIMETABLE_ERROR:
    return "ERROR: Failed parsing timetable";
  case SCRAPER_OK:
    return "OK: Successfully scraped website";
  case SCRAPER_ERROR:
    return "ERROR: Failed scraping website";
  case WEB_SERVER_OK:
    return "OK: Successfull operation";
  case WEB_SERVER_ERROR:
    return "ERROR: Server error";
  case HASHMAP_OPERATION_OK:
    return "OK: Server error";
  case HASHMAP_NULL:
    return "ERROR: Key does not exist";
  default:
    return "ERROR: Unknown error.";
  }
}
