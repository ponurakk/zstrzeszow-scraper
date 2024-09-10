#pragma once
#ifndef ARRAY_H
#define ARRAY_H

typedef struct Lesson {
  char *class_id;
  char *teacher_id;
  int order;
  char *hours;
  char *lesson_name;
  char *classroom;
  int weekday;
} Lesson;

#include <stdlib.h>

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

#define arrayInit(array, size)                                                 \
  _Generic((array), intArray *: intArrayInit, LessonArray *: LessonArrayInit)( \
      array, size)

#define arrayPush(array, element)                                              \
  _Generic((array), intArray *: intArrayPush, LessonArray *: LessonArrayPush)( \
      array, element)

#define arrayPop(array)                                                        \
  _Generic((array), intArray *: intArrayPop, LessonArray *: LessonArrayPop)(   \
      array)

#define arrayFree(array)                                                       \
  _Generic((array), intArray *: intArrayFree, LessonArray *: LessonArrayFree)( \
      array)

#endif // ARRAY_H
