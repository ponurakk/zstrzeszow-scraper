#pragma once
#include "../utils/array.h"
#include "../utils/error.h"
#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "../database.h"
#include "error.h"
#include "list.h"
#include <curl/curl.h>

typedef struct Lesson Lesson;

Error get_timetable(LessonArray *lesson, int i, char *timetable_url, Ward *ward,
                    CURL *curl_handle, char *generation_date);

#endif // !TIMETABLE_H
