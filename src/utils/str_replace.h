#ifndef STR_REPLACE_H
#define STR_REPLACE_H
#include <stddef.h>

typedef struct {
  const char *find;
  const char *replace;
} ReplacePair;

int appendstr(char **string, const char *append);
char *strtokk(char *string, const char *strf);
void str_replace(char **target, const char *strf, const char *strr);
void str_replace_multiple(char **target, ReplacePair *replacements,
                          size_t num_replacements);

#endif // !STR_REPLACE_H
