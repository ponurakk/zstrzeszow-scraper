#include "timetable.h"
#include "error.h"
#include "main.h"
#include <ctype.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/xpath.h>
#include <stdio.h>
#include <string.h>

Error getGenerationDate(xmlXPathContextPtr context, char *generationDate) {
  xmlXPathObjectPtr footer = xmlXPathEvalExpression(
      (xmlChar *)"//table/tr[3]/td[2]/table/tr/td", context);

  if (footer == NULL || footer->nodesetval == NULL ||
      footer->nodesetval->nodeNr == 0) {
    return TIMETABLE_ERROR;
  }

  xmlNodePtr footerHTML = footer->nodesetval->nodeTab[0];
  char *footerText = (char *)xmlNodeGetContent(footerHTML);

  if (footerText == NULL) {
    xmlXPathFreeObject(footer);
    return TIMETABLE_ERROR;
  }

  if (sscanf(footerText, "\nwygenerowano %10s", generationDate) != 1) {
    xmlFree(footerText);
    xmlXPathFreeObject(footer);
    return TIMETABLE_ERROR;
  }

  xmlFree(footerText);
  xmlXPathFreeObject(footer);
  return TIMETABLE_OK;
}

Error getValidDate(xmlXPathContextPtr context, char *validDate) {
  xmlXPathObjectPtr footer =
      xmlXPathEvalExpression((xmlChar *)"//div/table/tr[2]/td", context);

  if (footer == NULL || footer->nodesetval == NULL ||
      footer->nodesetval->nodeNr == 0) {
    return TIMETABLE_ERROR;
  }

  xmlNodePtr footerHTML = footer->nodesetval->nodeTab[0];
  char *footerText = (char *)xmlNodeGetContent(footerHTML);

  if (footerText == NULL) {
    xmlXPathFreeObject(footer);
    return TIMETABLE_ERROR;
  }

  int day, month, year;
  if (sscanf(footerText, "\nObowiązuje od: %2d.%2d.%4d", &day, &month, &year) !=
      3) {
    xmlFree(footerText);
    xmlXPathFreeObject(footer);
    return TIMETABLE_ERROR;
  }

  sprintf(validDate, "%04d-%02d-%02d", year, month, day);

  xmlFree(footerText);
  xmlXPathFreeObject(footer);
  return TIMETABLE_OK;
}

Error parse0Span(xmlNodePtr lessonCell) {
  printf("%s, ", (char *)xmlNodeGetContent(lessonCell));
  return TIMETABLE_OK;
}

Error parse2Spans(xmlXPathContextPtr context) {
  xmlXPathObjectPtr teacherHTML =
      xmlXPathEvalExpression((xmlChar *)".//a[1]", context);
  if (teacherHTML->nodesetval->nodeNr == 0) {
    xmlXPathObjectPtr subjectHTML =
        xmlXPathEvalExpression((xmlChar *)".//span[1]", context);
    xmlXPathObjectPtr classroomHTML =
        xmlXPathEvalExpression((xmlChar *)".//span[2]", context);

    xmlNodePtr subject = subjectHTML->nodesetval->nodeTab[0];
    xmlNodePtr classroom = classroomHTML->nodesetval->nodeTab[0];

    printf("%s ", (char *)xmlNodeGetContent(subject));
    // printf("%s ", (char *)xmlNodeGetContent(teacher));
    // printf("%s, ", (char *)xmlNodeGetContent(classroom));
  } else {
    xmlXPathObjectPtr subjectHTML =
        xmlXPathEvalExpression((xmlChar *)".//span[1]", context);
    xmlXPathObjectPtr classroomHTML =
        xmlXPathEvalExpression((xmlChar *)".//span[2]", context);

    xmlNodePtr teacher = teacherHTML->nodesetval->nodeTab[0];
    xmlNodePtr subject = subjectHTML->nodesetval->nodeTab[0];
    xmlNodePtr classroom = classroomHTML->nodesetval->nodeTab[0];

    printf("%s ", (char *)xmlNodeGetContent(subject));
    // printf("%s ", (char *)xmlNodeGetContent(teacher));
    // printf("%s, ", (char *)xmlNodeGetContent(classroom));
  }

  return TIMETABLE_OK;
}

Error parse3Spans(xmlXPathContextPtr context) {
  xmlXPathObjectPtr teacherHTML =
      xmlXPathEvalExpression((xmlChar *)".//span/a[1]", context);
  if (teacherHTML->nodesetval->nodeNr == 0) {
    xmlXPathObjectPtr subjectHTML =
        xmlXPathEvalExpression((xmlChar *)".//span[1]", context);
    xmlXPathObjectPtr teacherHTML =
        xmlXPathEvalExpression((xmlChar *)".//span[2]", context);
    xmlXPathObjectPtr classroomHTML =
        xmlXPathEvalExpression((xmlChar *)".//span[3]", context);

    xmlNodePtr subject = subjectHTML->nodesetval->nodeTab[0];
    xmlNodePtr teacher = teacherHTML->nodesetval->nodeTab[0];
    xmlNodePtr classroom = classroomHTML->nodesetval->nodeTab[0];

    printf("%s, ", (char *)xmlNodeGetContent(subject));
    // printf("%s, ", (char *)xmlNodeGetContent(teacher));
    // printf("%s, ", (char *)xmlNodeGetContent(classroom));
  } else {
    xmlXPathObjectPtr subjectHTML =
        xmlXPathEvalExpression((xmlChar *)".//span/span[1]", context);
    xmlXPathObjectPtr teacherHTML =
        xmlXPathEvalExpression((xmlChar *)".//span/a[1]", context);
    xmlXPathObjectPtr classroomHTML =
        xmlXPathEvalExpression((xmlChar *)".//span/span[2]", context);

    xmlNodePtr subject = subjectHTML->nodesetval->nodeTab[0];
    xmlNodePtr teacher = teacherHTML->nodesetval->nodeTab[0];
    xmlNodePtr classroom = classroomHTML->nodesetval->nodeTab[0];

    printf("%s ", (char *)xmlNodeGetContent(subject));
    // printf("%s ", (char *)xmlNodeGetContent(teacher));
    // printf("%s, ", (char *)xmlNodeGetContent(classroom));
  }

  return TIMETABLE_OK;
}

Error parseLesson(xmlNodePtr lessonCell, xmlXPathContextPtr context,
                  Lesson *lesson) {
  if (lessonCell == NULL) {
    fprintf(stderr, "ERROR: lessonCell is NULL\n");
    return TIMETABLE_ERROR;
  }

  if (xmlXPathSetContextNode(lessonCell, context) != 0) {
    fprintf(stderr, "ERROR: Failed to set context node\n");
    return TIMETABLE_ERROR;
  }

  xmlXPathObjectPtr subjectHTML =
      xmlXPathEvalExpression((xmlChar *)".//span", context);

  // Handle o5, o20, o23
  if (subjectHTML == NULL || subjectHTML->nodesetval == NULL ||
      subjectHTML->nodesetval->nodeNr == 0) {
    if (subjectHTML)
      xmlXPathFreeObject(subjectHTML);
    parse0Span(lessonCell);
    // fprintf(stderr, "ERROR: XPath expression returned no results\n");
    return TIMETABLE_OK;
  }

  int count = subjectHTML->nodesetval->nodeNr;

  if (count == 2) { // Normal
    parse2Spans(context);
  } else if (count == 3) { // Single group
    parse3Spans(context);
  } else if (count >= 6) { // Two groups or more
    for (int i = 0; i < count / 3; ++i) {
      parse3Spans(context);
    }
  }

  printf("\t");

  return TIMETABLE_OK;
}

Error parseRow(xmlNodePtr row, xmlXPathContextPtr context, Lesson *lesson,
               int size) {
  xmlXPathSetContextNode(row, context);

  xmlNodePtr orderCell = xmlXPathEvalExpression((xmlChar *)".//td[1]", context)
                             ->nodesetval->nodeTab[0];
  if (orderCell == NULL) {
    return TIMETABLE_ERROR;
  }

  char *order_str = (char *)xmlNodeGetContent(orderCell);
  int order;
  sscanf(order_str, "%d", &order);

  lesson[size].order = order;

  xmlNodePtr hourCell = xmlXPathEvalExpression((xmlChar *)".//td[2]", context)
                            ->nodesetval->nodeTab[0];
  if (hourCell == NULL) {
    return TIMETABLE_ERROR;
  }
  char *hour = (char *)xmlNodeGetContent(hourCell);

  lesson[size].hours = hour;

  int len = strlen(hour);
  for (int i = 0; i < len; ++i)
    if (isspace(hour[i]))
      for (int j = i; j < len; ++j)
        hour[j] = hour[j + 1];

  printf("%s. %s - ", order_str, hour);

  for (int i = 2; i < 7; ++i) {
    xmlXPathSetContextNode(row, context);
    char xpath[100];
    sprintf(xpath, ".//td[%i]", i + 1);

    xmlXPathObjectPtr xpathObj =
        xmlXPathEvalExpression((xmlChar *)xpath, context);

    if (xpathObj == NULL || xpathObj->nodesetval == NULL ||
        xpathObj->nodesetval->nodeNr == 0) {
      if (xpathObj)
        xmlXPathFreeObject(xpathObj);
      return TIMETABLE_ERROR;
    }

    xmlNodePtr lessonCell = xpathObj->nodesetval->nodeTab[0];

    if (parseLesson(lessonCell, context, lesson) != TIMETABLE_OK) {
      xmlXPathFreeObject(xpathObj);
      return TIMETABLE_ERROR;
    }

    xmlXPathFreeObject(xpathObj);
  }

  printf("\n");

  return TIMETABLE_OK;
}

Error parseTimetable(xmlXPathContextPtr context, Lesson *lesson, int size) {
  xmlXPathObjectPtr rows = xmlXPathEvalExpression(
      (xmlChar *)"//div/table/tr[1]/td/table/tr", context);

  if (rows == NULL || rows->nodesetval == NULL ||
      rows->nodesetval->nodeNr == 0) {
    if (rows)
      xmlXPathFreeObject(rows);
    return TIMETABLE_ERROR;
  }

  for (int i = 1; i < rows->nodesetval->nodeNr; ++i) {
    xmlNodePtr row = rows->nodesetval->nodeTab[i];
    if (parseRow(row, context, lesson, i) != TIMETABLE_OK) {
      continue;
    }
  }

  xmlXPathFreeObject(rows);
  return TIMETABLE_OK;
}

Error getTimetable(Lesson *lesson, int i, Ward *ward, CURL *curl_handle) {
  char timetablePath[100];
  sprintf(timetablePath, "http://zstrzeszow.pl/plan/plany/%s.html", ward->id);
  CURLResponse response = GetRequest(curl_handle, timetablePath);

  // parse the HTML document returned by the server
  htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size,
                                  NULL, NULL, HTML_PARSE_NOERROR);
  xmlXPathContextPtr context = xmlXPathNewContext(doc);

  char generationDate[100];
  if (getGenerationDate(context, generationDate) != TIMETABLE_OK) {
    return TIMETABLE_ERROR;
  }
  // printf("%s\n", generationDate);

  char validDate[100];
  if (getValidDate(context, validDate) != TIMETABLE_OK) {
    return TIMETABLE_ERROR;
  }
  // printf("%s\n", validDate);

  if (parseTimetable(context, lesson, i) != TIMETABLE_OK) {
    return TIMETABLE_ERROR;
  }

  return TIMETABLE_OK;
}
