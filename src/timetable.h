#pragma once
#include "array.h"
#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "database.h"
#include "error.h"
#include "list.h"
#include <curl/curl.h>

typedef struct Lesson Lesson;

Error getTimetable(LessonArray *lesson, int i, Ward *ward, CURL *curl_handle);

#endif // !TIMETABLE_H
