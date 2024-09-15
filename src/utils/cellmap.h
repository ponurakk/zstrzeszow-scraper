#ifndef CELLMAP_H
#define CELLMAP_H
#include "array.h"
#include "error.h"

typedef struct Cell_t {
  int x;
  int y;
} Cell;

typedef struct Pair_t {
  Cell key;
  LessonArray val;
  struct Pair_t *next;
} Pair;

typedef struct CellMap_t {
  Pair **list;
  uint cap;
  uint len;
} CellMap;

typedef struct Item_t {
  Cell key;
  LessonArray val;
} Item;

CellMap *cellmap_init();
Error cellmap_get(CellMap *cellmap, Cell key, LessonArray *out);
void cellmap_set(CellMap *cellmap, Cell key, LessonArray val);
void cellmap_insert_or_push(CellMap *cellmap, Cell key, LessonArray val,
                            Lesson lesson);
void cellmap_iterate(CellMap *cellmap, void (*callback)(Cell, LessonArray));

void cellmap_collect(CellMap *cellmap, Item *items, int *item_count);
int compare_cells(const void *a, const void *b);

#endif // !CELLMAP_H
