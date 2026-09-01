#include "array.h"

#define DEFINE_ARRAY_IMPLEMENTATION(type)                                      \
  void type##ArrayInit(type##Array *array, size_t size) {                      \
    array->array = malloc(size * sizeof(type));                                \
    array->count = 0;                                                          \
    array->size = size;                                                        \
  }                                                                            \
                                                                               \
  int type##ArrayPush(type##Array *array, type element) {                      \
    if (array->count == array->size) {                                         \
      array->size *= 2;                                                        \
      array->array = realloc(array->array, array->size * sizeof(type));        \
    }                                                                          \
    array->array[array->count++] = element;                                    \
    return array->count - 1;                                                   \
  }                                                                            \
                                                                               \
  void type##ArrayPop(type##Array *array) {                                    \
    array->array[array->count-- - 1] = array->array[array->size];              \
  }                                                                            \
                                                                               \
  void type##ArrayFree(type##Array *array) {                                   \
    free(array->array);                                                        \
    array->array = NULL;                                                       \
    array->count = array->size = 0;                                            \
  }

DEFINE_ARRAY_IMPLEMENTATION(int)
DEFINE_ARRAY_IMPLEMENTATION(Lesson)
DEFINE_ARRAY_IMPLEMENTATION(DbCache)

void free_lesson(Lesson *lesson) {
  if (lesson->class_id) {
    free(lesson->class_id);
  }
  if (lesson->teacher_id) {
    free(lesson->teacher_id);
  }
  if (lesson->hours) {
    free(lesson->hours);
  }
  if (lesson->lesson_name) {
    free(lesson->lesson_name);
  }
  if (lesson->classroom) {
    free(lesson->classroom);
  }
}

void free_lesson_array(LessonArray *array) {
  for (size_t i = 0; i < array->count; i++) {
    free_lesson(&array->array[i]);
  }
  free(array->array); // Free the array of lessons
}
