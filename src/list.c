#include "list.h"
#include <stdio.h>
#include <string.h>

void getWardList(Ward *wardList, xmlNodePtr wardHTMLElement,
                 xmlXPathContextPtr context, int i) {
  xmlXPathSetContextNode(wardHTMLElement, context);
  xmlNodePtr urlHTMLElement = xmlXPathEvalExpression((xmlChar *)".//a", context)
                                  ->nodesetval->nodeTab[0];

  char *aHref = (char *)xmlGetProp(urlHTMLElement, (xmlChar *)"href");
  if (aHref == NULL) {
    fprintf(stderr, "Failed to get href property.\n");
    return;
  }

  char url[100];
  sscanf(aHref, "plany/%[^.].html", url);
  xmlFree(aHref);

  char *full = (char *)xmlNodeGetContent(urlHTMLElement);

  int class_number;
  char tag[100];
  sscanf(full, "%d%[^.]", &class_number, tag);

  // Use strdup to ensure proper memory management
  // Because string allocated here goes out of scope
  wardList[i].id = strdup(url);
  wardList[i].full = strdup(full);
  wardList[i].class_number = class_number;
  wardList[i].tag = strdup(tag);
  xmlFree(full);
}

void getTeachersList(Teacher *teacherList, xmlNodePtr wardHTMLElement,
                     xmlXPathContextPtr context, int i) {
  xmlXPathSetContextNode(wardHTMLElement, context);
  xmlNodePtr urlHTMLElement = xmlXPathEvalExpression((xmlChar *)".//a", context)
                                  ->nodesetval->nodeTab[0];

  char *aHref = (char *)xmlGetProp(urlHTMLElement, (xmlChar *)"href");
  if (aHref == NULL) {
    fprintf(stderr, "Failed to get href property.\n");
    return;
  }

  char url[100];
  sscanf(aHref, "plany/%[^.].html", url);
  xmlFree(aHref);

  char *full = (char *)xmlNodeGetContent(urlHTMLElement);

  char name[100];
  char initials[100];
  sscanf(full, "%[^(] (%[^)])", name, initials);
  xmlFree(full);

  teacherList[i].id = strdup(url);
  teacherList[i].name = strdup(name);
  teacherList[i].initials = strdup(initials);
}
