#ifndef ARRAY_H
#define ARRAY_H

#include <sqlite3.h>
#include <stdlib.h>

typedef struct Lesson {
  char *class_id;
  char *teacher_id;
  int order;
  char *hours;
  char *lesson_name;
  char *classroom;
  int weekday;
} Lesson;

void free_lesson(Lesson *lesson);

typedef struct DbCache_t {
  char *date;
  sqlite3 *db;
} DbCache;

#define DEFINE_ARRAY(type)                                                     \
  typedef struct {                                                             \
    type *array;                                                               \
    size_t count;                                                              \
    size_t size;                                                               \
  } type##Array;                                                               \
                                                                               \
  void type##ArrayInit(type##Array *array, size_t size);                       \
  int type##ArrayPush(type##Array *array, type element);                       \
  void type##ArrayPop(type##Array *array);                                     \
  void type##ArrayFree(type##Array *array);

DEFINE_ARRAY(int)
DEFINE_ARRAY(Lesson)
DEFINE_ARRAY(DbCache)

#define arrayInit(array, size)                                                 \
  _Generic((array),                                                            \
      intArray *: intArrayInit,                                                \
      LessonArray *: LessonArrayInit,                                          \
      DbCacheArray *: DbCacheArrayInit)(array, size)

#define arrayPush(array, element)                                              \
  _Generic((array),                                                            \
      intArray *: intArrayPush,                                                \
      LessonArray *: LessonArrayPush,                                          \
      DbCacheArray *: DbCacheArrayPush)(array, element)

#define arrayPop(array)                                                        \
  _Generic((array),                                                            \
      intArray *: intArrayPop,                                                 \
      LessonArray *: LessonArrayPop,                                           \
      DbCacheArray *: DbCacheArrayPop)(array)

#define arrayFree(array)                                                       \
  _Generic((array),                                                            \
      intArray *: intArrayFree,                                                \
      LessonArray *: LessonArrayFree,                                          \
      DbCacheArray *: DbCacheArrayFree)(array)

void free_lesson_array(LessonArray *array);
#endif // ARRAY_H
