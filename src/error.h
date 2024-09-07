#ifndef ERROR_H
#define ERROR_H

typedef enum {
  SQLITE_SUCCESS = 0,
  SQLITE_ERROR = 1,
} Error;

const char *errorToString(Error err);

#endif // !ERROR_H
