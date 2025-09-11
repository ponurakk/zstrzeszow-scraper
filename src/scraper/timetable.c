#include "timetable.h"
#include "../utils/logger.h"
#include "validator.h"
#include <libxml2/libxml/HTMLparser.h>
#include <stdio.h>
#include <string.h>

int get_left_of_dash_if_present(const char *str, char **out) {
  const char *en_dash = strstr(str, "–"); // UTF-8 en dash
  const char *dash = strchr(str, '-');    // ASCII dash

  const char *split_point = NULL;

  // Choose the first dash that appears
  if (en_dash && dash) {
    split_point = (en_dash < dash) ? en_dash : dash;
  } else if (en_dash) {
    split_point = en_dash;
  } else if (dash) {
    split_point = dash;
  } else {
    split_point = NULL;
  }

  size_t len = split_point ? (size_t)(split_point - str) : strlen(str);

  *out = malloc(len + 1);
  if (!*out)
    return 1;

  strncpy(*out, str, len);
  (*out)[len] = '\0';

  // Optional: trim trailing whitespace and non-breaking space
  while (len > 0 && ((*out)[len - 1] == ' ' || (*out)[len - 1] == '\t' ||
                     (unsigned char)(*out)[len - 1] == 0xC2 ||
                     (unsigned char)(*out)[len - 1] == 0xA0)) {
    (*out)[--len] = '\0';
  }

  return 0;
}

void trim(char *s) {
  char *p = s, *e;
  while ((unsigned char)*p == 0xC2 || (unsigned char)*p == 0xA0)
    p++;
  if (*p == 0) {
    *s = 0;
    return;
  }
  e = p + strlen(p) - 1;
  while (e > p && ((unsigned char)*e == 0xC2 || (unsigned char)*e == 0xA0))
    e--;
  *(e + 1) = 0;
  if (p != s)
    memmove(s, p, e - p + 2);
}

Error get_generation_date(xmlXPathContextPtr context, char **generation_date) {
  xmlXPathObjectPtr result =
      xmlXPathEvalExpression((const xmlChar *)"//meta[@name='date']", context);

  if (!result || xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    fprintf(stderr, "No <meta name='date'> found\n");
    xmlXPathFreeObject(result);
    return TIMETABLE_ERROR;
  }

  // Get content attribute
  xmlNodePtr meta = result->nodesetval->nodeTab[0];
  xmlChar *content = xmlGetProp(meta, (const xmlChar *)"content");

  if (content) {
    *generation_date = malloc(11);
    if (!*generation_date)
      return TIMETABLE_ERROR;
    strncpy(*generation_date, (const char *)content, 10);
    xmlFree(content);
  }

  xmlXPathFreeObject(result);
  return TIMETABLE_OK;
}

Error get_effective_date(xmlXPathContextPtr context, char **effective_date) {
  xmlXPathObjectPtr pages =
      xmlXPathEvalExpression((xmlChar *)"//body/div", context);

  if (pages == NULL || pages->nodesetval == NULL ||
      pages->nodesetval->nodeNr == 0) {
    if (pages)
      xmlXPathFreeObject(pages);
    fprintf(stderr, "Pages error");
    return TIMETABLE_ERROR;
  }

  xmlNodePtr page = pages->nodesetval->nodeTab[0];

  xmlXPathSetContextNode(page, context);
  xmlXPathObjectPtr cells = xmlXPathEvalExpression((xmlChar *)".//p", context);

  if (!cells || !cells->nodesetval || cells->nodesetval->nodeNr == 0) {
    if (cells)
      xmlXPathFreeObject(cells);
    fprintf(stderr, "Cells error");
    return TIMETABLE_ERROR;
  }

  for (int i = 0; i < cells->nodesetval->nodeNr; ++i) {
    xmlNodePtr cell = cells->nodesetval->nodeTab[i];
    char *text = (char *)xmlNodeGetContent(cell);
    trim(text);

    if (is_valid_date_format(text, effective_date) == 0) {
      break;
    }

    xmlFree(text);
  }

  return TIMETABLE_OK;
}

int are_within_radius(int num1, int num2, int radius) {
  return abs(num1 - num2) <= radius;
}

char *concat(const char *a, const char *b) {
  if (!a && !b)
    return strdup("");
  if (!a)
    return strdup(b);
  if (!b)
    return strdup(a);

  size_t len_a = strlen(a);
  size_t len_b = strlen(b);
  // need +1 for space +1 for '\0'
  char *result = malloc(len_a + 1 + len_b + 1);
  if (!result) {
    perror("malloc failed");
    exit(1);
  }

  memcpy(result, a, len_a);
  result[len_a] = ' ';
  memcpy(result + len_a + 1, b, len_b + 1);
  return result;
}

void parse_lesson_name(char *text, char **lesson, char **teacher,
                       char **classroom) {
  char *token = strtok(text, " \t\xA0");

  int idx = 0;
  while (token != NULL) {
    trim(token);

    if (idx == 0) {
      *lesson = strdup(token);
    } else if (idx == 1) {
      *teacher = strdup(token);
    } else if (idx == 2) {
      *classroom = strdup(token);
    } else if (idx == 3) {
      // *group = strdup(token);
    }

    token = strtok(NULL, " \t\xA0");
    idx++;
  }

  // If no group is found, set it to empty string
  // if (*group == NULL) {
  //   *group = "";
  // }
}

void parse_lesson_name_2(char *text, char **lesson, char **teacher,
                         char **classroom) {
  char *token = strtok(text, " \t\xA0");

  int idx = 0;
  while (token != NULL) {
    trim(token);

    if (idx == 0) {
      *teacher = strdup(token);
    } else if (idx == 1) {
      *classroom = strdup(token);
    } else if (idx == 2) {
      *lesson = concat(*lesson, token);
    }

    token = strtok(NULL, " \t\xA0");
    idx++;
  }
}

Error parse_page(xmlNodePtr page, xmlXPathContextPtr context,
                 LessonArray *lesson_list) {
  xmlXPathSetContextNode(page, context);
  xmlXPathObjectPtr cells = xmlXPathEvalExpression((xmlChar *)".//p", context);

  if (!cells || !cells->nodesetval || cells->nodesetval->nodeNr == 0) {
    if (cells)
      xmlXPathFreeObject(cells);
    fprintf(stderr, "Cells error");
    return TIMETABLE_ERROR;
  }

  char *class_id = NULL;
  int order = 0;

  for (int i = 0; i < cells->nodesetval->nodeNr; ++i) {
    xmlNodePtr cell = cells->nodesetval->nodeTab[i];
    char *style = (char *)xmlGetProp(cell, (const xmlChar *)"style");
    int top = -1, left = -1;
    if (style) {
      sscanf(style, "position:absolute;top:%dpx;left:%dpx", &top, &left);
      free(style);
    }
    char *text = (char *)xmlNodeGetContent(cell);

    if (is_day(text) == 0 || (order == 0 && strcmp(text, " ") == 0) ||
        is_valid_number_nbsp(text) == 0) {
      xmlFree(text);
      continue;
    }
    trim(text);

    if (is_valid_class_format(text) == 0) {
      trim(text);
      if (class_id)
        xmlFree(class_id);
      get_left_of_dash_if_present(text, &class_id);
      continue;
    }

    if (is_valid_hour_format(text) == 0) {
      char *xmlOrder =
          (char *)xmlNodeGetContent(cells->nodesetval->nodeTab[i - 1]);
      trim(xmlOrder);
      order = atoi(xmlOrder);
      xmlFree(xmlOrder);
      xmlFree(text);
      continue;
    }

    Lesson l = {.order = order,
                .lesson_name = strdup(text),
                .classroom = NULL,
                .teacher_id = NULL,
                .class_id = strdup(class_id),
                .weekday = 0};

    if (is_wrapped_in_parentheses(text) == 0) {
      char *child_txt = (char *)xmlNodeGetContent(cell->children); // xmlFree
      if (child_txt) {
        trim(child_txt);
        char *old = lesson_list->array[lesson_list->count - 1].lesson_name;
        char *joined = concat(old, child_txt);
        lesson_list->array[lesson_list->count - 1].lesson_name = joined;
        free(old);
        xmlFree(child_txt);
      }
    }

    xmlNode *b1 = NULL, *b2 = NULL;
    int b_count = 0;

    for (xmlNode *child = cell->children; child != NULL; child = child->next) {
      if (child->type == XML_ELEMENT_NODE &&
          xmlStrcmp(child->name, (const xmlChar *)"b") == 0) {
        ++b_count;
        if (b_count == 1) {
          b1 = child;
        } else if (b_count == 2) {
          b2 = child;
        }
      }
    }

    if (are_within_radius(left, 1021, 10)) {
      l.weekday = 4;
    } else if (are_within_radius(left, 793, 10)) {
      l.weekday = 3;
    } else if (are_within_radius(left, 566, 10)) {
      l.weekday = 2;
    } else if (are_within_radius(left, 339, 10)) {
      l.weekday = 1;
    } else if (are_within_radius(left, 112, 10)) {
      l.weekday = 0;
    }

    if (b_count == 1 || b_count == 2) {
      if (strcmp(text, "") == 0) {
        continue;
      }

      char *b1_content = (char *)xmlNodeGetContent(b1);
      trim(b1_content);
      char *next_content = NULL;
      if (b1->next && b1->next->type == XML_TEXT_NODE) {
        next_content = (char *)xmlNodeGetContent(b1->next);
      }

      if (next_content == NULL) {
        parse_lesson_name(b1_content, &l.lesson_name, &l.teacher_id,
                          &l.classroom);
      } else {
        trim(next_content);
        l.lesson_name = strdup(b1_content);
        parse_lesson_name_2(next_content, &l.lesson_name, &l.teacher_id,
                            &l.classroom);
      }

      if (b1->next && b1->next->next &&
          xmlStrcmp(b1->next->next->name, (const xmlChar *)"br") == 0) {
        if (b1->next->next->next &&
            b1->next->next->next->type == XML_TEXT_NODE) {
          l.lesson_name = concat(
              l.lesson_name, (char *)xmlNodeGetContent(b1->next->next->next));
        }
      }

      arrayPush(lesson_list, l);
      xmlFree(b1_content);
      xmlFree(next_content);
    }

    if (b_count == 2) {
      char *b2_content = (char *)xmlNodeGetContent(b2);
      trim(b2_content);
      char *next_content = NULL;
      if (b2->next && b2->next->type == XML_TEXT_NODE) {
        next_content = (char *)xmlNodeGetContent(b2->next);
      }

      if (next_content == NULL) {
        parse_lesson_name(b2_content, &l.lesson_name, &l.teacher_id,
                          &l.classroom);
      } else {
        trim(next_content);
        l.lesson_name = strdup(b2_content);
        parse_lesson_name_2(next_content, &l.lesson_name, &l.teacher_id,
                            &l.classroom);
      }

      if (b2->next && b2->next->next &&
          xmlStrcmp(b2->next->next->name, (const xmlChar *)"br") == 0) {
        if (b2->next->next->next &&
            b2->next->next->next->type == XML_TEXT_NODE) {
          l.lesson_name = concat(
              l.lesson_name, (char *)xmlNodeGetContent(b2->next->next->next));
        }
      }

      arrayPush(lesson_list, l);
      xmlFree(b2_content);
      xmlFree(next_content);
    }

    xmlFree(text);
  }

  xmlXPathFreeObject(cells);
  return TIMETABLE_OK;
}

Error get_timetable(LessonArray *lesson_list, xmlXPathContextPtr context,
                    char **effective_date) {
  if (get_effective_date(context, effective_date) != TIMETABLE_OK) {
    print_error("Failed getting generation date");
  }

  xmlXPathObjectPtr pages =
      xmlXPathEvalExpression((xmlChar *)"//body/div", context);

  if (pages == NULL || pages->nodesetval == NULL ||
      pages->nodesetval->nodeNr == 0) {
    if (pages)
      xmlXPathFreeObject(pages);
    fprintf(stderr, "Pages error");
    return TIMETABLE_ERROR;
  }

  for (int i = 0; i < pages->nodesetval->nodeNr; ++i) {
    xmlNodePtr page = pages->nodesetval->nodeTab[i];
    printf("%s\n", page->properties->id->value);

    if (parse_page(page, context, lesson_list) != TIMETABLE_OK) {
      print_error("Failed parsing timetable");
      return TIMETABLE_ERROR;
    }
  }

  xmlXPathFreeObject(pages);

  return TIMETABLE_OK;
}
