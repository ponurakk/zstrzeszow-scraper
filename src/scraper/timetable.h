#pragma once
#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "../database.h"
#include "../utils/array.h"
#include "../utils/error.h"
#include "error.h"
#include "list.h"
#include <curl/curl.h>

typedef struct Lesson Lesson;

Error get_generation_date(xmlXPathContextPtr context, char *generation_date);
Error get_effective_date(xmlXPathContextPtr context, char *valid_date);

Error get_timetable(LessonArray *lesson, int i, char *timetable_url, Ward *ward,
                    CURL *curl_handle, char *generation_date,
                    char *effective_date);

#endif // !TIMETABLE_H
