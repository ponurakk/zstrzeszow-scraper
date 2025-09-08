#pragma once
#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "../database.h"
#include "../utils/array.h"
#include "../utils/error.h"
#include "error.h"
#include <curl/curl.h>
#include <libxml2/libxml/xpath.h>

typedef struct Lesson Lesson;

Error get_generation_date(xmlXPathContextPtr context, char **generation_date);
Error get_effective_date(xmlXPathContextPtr context, char **effective_date);

Error get_timetable(LessonArray *lesson, xmlXPathContextPtr context,
                    char **effective_date);

#endif // !TIMETABLE_H
