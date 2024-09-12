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
  default:
    return "ERROR: Unknown error.";
  }
}
