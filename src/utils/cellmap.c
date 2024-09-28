#include "cellmap.h"
#include "array.h"
#include "error.h"

CellMap *cellmap_init() {
  CellMap *cellmap = malloc(sizeof(*cellmap));
  cellmap->cap = 8;
  cellmap->len = 0;
  cellmap->list = calloc(cellmap->cap, sizeof(Pair *));
  return cellmap;
}

unsigned hashcode(Cell key, unsigned capacity) {
  unsigned code = 17;       // Start with a prime number
  code = 31 * code + key.x; // Combine 'x' with hash
  code = 31 * code + key.y; // Combine 'y' with hash
  return code % capacity;   // Map to the range [0, capacity-1]
}

Error cellmap_get(CellMap *cellmap, Cell key, LessonArray *out) {
  Pair *current;
  for (current = cellmap->list[hashcode(key, cellmap->cap)]; current;
       current = current->next) {
    if (current->key.x == key.x && current->key.y == key.y) {
      *out = current->val;
      return HASHMAP_OPERATION_OK;
    }
  }

  return HASHMAP_NULL;
}

void cellmap_resize(CellMap *cellmap) {
  uint new_cap = cellmap->cap * 2;
  Pair **new_list = calloc(new_cap, sizeof(Pair *)); // Allocate new list

  // Rehash all existing pairs
  for (uint i = 0; i < cellmap->cap; ++i) {
    Pair *current = cellmap->list[i];
    while (current) {
      Pair *next = current->next; // Save the next node in the chain
      uint new_index = hashcode(current->key, new_cap);

      // Insert current pair into new list
      current->next = new_list[new_index];
      new_list[new_index] = current;

      current = next;
    }
  }

  free(cellmap->list);      // Free the old list
  cellmap->list = new_list; // Point to the new resized list
  cellmap->cap = new_cap;   // Update the capacity
}

void cellmap_set(CellMap *cellmap, Cell key, LessonArray val) {
  if (cellmap->len == cellmap->cap) {
    cellmap_resize(cellmap);
  }

  uint index = hashcode(key, cellmap->cap);
  Pair *current;
  for (current = cellmap->list[index]; current; current = current->next) {
    // if key has been already in hashmap
    if (current->key.x == key.x && current->key.y == key.y) {
      current->val = val;
      return;
    }
  }
  // key is not in the hashmap
  Pair *p = malloc(sizeof(*p));
  p->key = key;
  p->val = val;
  p->next = cellmap->list[index];
  cellmap->list[index] = p;
  cellmap->len++;
}

void cellmap_insert_or_push(CellMap *cellmap, Cell key, LessonArray val,
                            Lesson lesson) {
  if (cellmap->len == cellmap->cap) {
    cellmap_resize(cellmap);
  }

  unsigned int index = hashcode(key, cellmap->cap);
  Pair *current;
  for (current = cellmap->list[index]; current; current = current->next) {
    // if key has been already in hashmap
    if (current->key.x == key.x && current->key.y == key.y) {
      arrayPush(&val, lesson);
      current->val = val;
      return;
    }
  }
  // key is not in the hashmap
  Pair *p = malloc(sizeof(*p));
  p->key = key;
  p->val = val;
  p->next = cellmap->list[index];
  cellmap->list[index] = p;
  cellmap->len++;
}

void collect_pairs(Cell key, LessonArray val, Item *items, int *index) {
  items[*index].key = key;
  items[*index].val = val;
  (*index)++;
}

void cellmap_collect(CellMap *cellmap, Item *items, int *item_count) {
  int index = 0;
  for (unsigned i = 0; i < cellmap->cap; i++) {
    Pair *current = cellmap->list[i];
    while (current) {
      collect_pairs(current->key, current->val, items, &index);
      current = current->next;
    }
  }
  *item_count = index; // Update the total item count
}

// Comparison function to sort by Cell keys (x first, then y)
int compare_cells(const void *a, const void *b) {
  const Item *item1 = (const Item *)a;
  const Item *item2 = (const Item *)b;

  if (item1->key.x != item2->key.x) {
    return item1->key.x - item2->key.x; // Sort by x-coordinate first
  } else {
    // If x is the same, sort by y-coordinate
    return item1->key.y - item2->key.y;
  }
}

void cellmap_iterate(CellMap *cellmap, void (*callback)(Cell, LessonArray)) {
  for (int i = 0; i < cellmap->cap; ++i) {
    Pair *current = cellmap->list[i]; // Start at each bucket
    while (current) {
      // Call the callback function for each key-value pair
      callback(current->key, current->val);
      current = current->next; // Move to the next pair in the chain (if any)
    }
  }
}
