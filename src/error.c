#include "error.h"

void (*globalErrorCallback)(const char *);

const char *errorToString(Error err) {
  switch (err) {
  case SQLITE_SUCCESS:
    return "OK: Operation completed successfully";
  case SQLITE_ERROR:
    return "ERROR: SQLite error";
  default:
    return "ERROR: Unknown error.";
  }
}
