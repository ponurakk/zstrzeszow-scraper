#ifndef LIST_H
#define LIST_H

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xpath.h>

typedef struct {
  // o1
  char *id;
  // 1AT
  char *full;
  // 1
  int class_number;
  // AT
  char *tag;
} Ward;

typedef struct {
  // n1
  char *id;
  // Ni
  char *initials;
  // N.Surname
  char *name;
} Teacher;

void get_ward_list(Ward *ward_list, xmlNodePtr ward_html_element,
                   xmlXPathContextPtr context, int i);

void get_teachers_list(Teacher *teacher_list, xmlNodePtr ward_html_element,
                       xmlXPathContextPtr context, int i);

#endif // !LIST_H
