#include "list.h"
#include "../utils/logger.h"
#include <string.h>

void get_ward_list(Ward *ward_list, xmlNodePtr ward_html_element,
                   xmlXPathContextPtr context, int i) {
  xmlXPathSetContextNode(ward_html_element, context);
  xmlNodePtr url_html_element =
      xmlXPathEvalExpression((xmlChar *)".//a", context)
          ->nodesetval->nodeTab[0];

  char *a_href = (char *)xmlGetProp(url_html_element, (xmlChar *)"href");
  if (a_href == NULL) {
    print_error("Failed to get href property.");
    return;
  }

  char url[100];
  sscanf(a_href, "plany/%[^.].html", url);
  xmlFree(a_href);

  char *full = (char *)xmlNodeGetContent(url_html_element);

  int class_number;
  char tag[100];
  sscanf(full, "%d%[^.]", &class_number, tag);

  // Use strdup to ensure proper memory management
  // Because string allocated here goes out of scope
  ward_list[i].id = strdup(url);
  ward_list[i].full = strdup(full);
  ward_list[i].class_number = class_number;
  ward_list[i].tag = strdup(tag);
  xmlFree(full);
}

void get_teachers_list(Teacher *teacher_list, xmlNodePtr ward_html_element,
                       xmlXPathContextPtr context, int i) {
  xmlXPathSetContextNode(ward_html_element, context);
  xmlNodePtr url_html_element =
      xmlXPathEvalExpression((xmlChar *)".//a", context)
          ->nodesetval->nodeTab[0];

  char *a_href = (char *)xmlGetProp(url_html_element, (xmlChar *)"href");
  if (a_href == NULL) {
    print_error("Failed to get href property.");
    return;
  }

  char url[100];
  sscanf(a_href, "plany/%[^.].html", url);
  xmlFree(a_href);

  char *full = (char *)xmlNodeGetContent(url_html_element);

  char name[100];
  char initials[100];
  sscanf(full, "%[^(] (%[^)])", name, initials);
  xmlFree(full);

  teacher_list[i].id = strdup(url);
  teacher_list[i].name = strdup(name);
  teacher_list[i].initials = strdup(initials);
}
