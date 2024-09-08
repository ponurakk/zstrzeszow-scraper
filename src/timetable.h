#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "database.h"
#include "error.h"
#include "list.h"
#include <curl/curl.h>

typedef struct {
  char *class_id;
  char *teacher_id;
  int order;
  char *hours;
  char *lesson_name;
  char *classroom;
} Lesson;

Error getTimetable(Lesson *lesson, int i, Ward *ward, CURL *curl_handle);

#endif // !TIMETABLE_H
