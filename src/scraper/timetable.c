#include "timetable.h"
#include "../main.h"
#include <ctype.h>
#include <libxml2/libxml/HTMLparser.h>
#include <string.h>

Error get_generation_date(xmlXPathContextPtr context, char *generation_date) {
  xmlXPathObjectPtr footer = xmlXPathEvalExpression(
      (xmlChar *)"//table/tr[3]/td[2]/table/tr/td", context);

  if (footer == NULL || footer->nodesetval == NULL ||
      footer->nodesetval->nodeNr == 0) {
    fprintf(stderr, "ERROR: Failed getting footer\n");
    return TIMETABLE_ERROR;
  }

  xmlNodePtr footer_html = footer->nodesetval->nodeTab[0];
  char *footer_text = (char *)xmlNodeGetContent(footer_html);

  if (footer_text == NULL) {
    xmlXPathFreeObject(footer);
    fprintf(stderr, "ERROR: Failed freeing generation date\n");
    return TIMETABLE_ERROR;
  }

  if (sscanf(footer_text, "\nwygenerowano %10s", generation_date) != 1) {
    xmlFree(footer_text);
    xmlXPathFreeObject(footer);
    fprintf(stderr, "ERROR: Failed parsing generation date\n");
    return TIMETABLE_ERROR;
  }

  xmlFree(footer_text);
  xmlXPathFreeObject(footer);
  return TIMETABLE_OK;
}

Error get_valid_date(xmlXPathContextPtr context, char *valid_date) {
  xmlXPathObjectPtr footer =
      xmlXPathEvalExpression((xmlChar *)"//div/table/tr[2]/td", context);

  if (footer == NULL || footer->nodesetval == NULL ||
      footer->nodesetval->nodeNr == 0) {
    fprintf(stderr, "ERROR: Failed getting footer\n");
    return TIMETABLE_ERROR;
  }

  xmlNodePtr footer_html = footer->nodesetval->nodeTab[0];
  char *footer_text = (char *)xmlNodeGetContent(footer_html);

  if (footer_text == NULL) {
    xmlXPathFreeObject(footer);
    fprintf(stderr, "ERROR: Failed freeing generation date\n");
    return TIMETABLE_ERROR;
  }

  int day, month, year;
  if (sscanf(footer_text, "\nObowiązuje od: %2d.%2d.%4d", &day, &month,
             &year) != 3) {
    xmlFree(footer_text);
    xmlXPathFreeObject(footer);
    fprintf(stderr, "ERROR: Failed parsing valid from date\n");
    return TIMETABLE_ERROR;
  }

  sprintf(valid_date, "%04d-%02d-%02d", year, month, day);

  xmlFree(footer_text);
  xmlXPathFreeObject(footer);
  return TIMETABLE_OK;
}

Error parse_0_span(xmlNodePtr lesson_cell, Lesson *l) {
  l->lesson_name = strdup((char *)xmlNodeGetContent(lesson_cell));
  return TIMETABLE_OK;
}

Error parse_2_spans(xmlXPathContextPtr context, Lesson *l) {
  xmlXPathObjectPtr teacher_html =
      xmlXPathEvalExpression((xmlChar *)".//*[@class=\"n\"]", context);
  if (teacher_html->nodesetval->nodeNr == 0) {
    xmlXPathObjectPtr subject_html =
        xmlXPathEvalExpression((xmlChar *)".//*[@class=\"p\"]", context);
    xmlXPathObjectPtr classroom_html =
        xmlXPathEvalExpression((xmlChar *)".//*[@class=\"p\"]", context);

    xmlNodePtr subject = subject_html->nodesetval->nodeTab[0];
    xmlNodePtr classroom = classroom_html->nodesetval->nodeTab[0];

    l->lesson_name = strdup((char *)xmlNodeGetContent(subject));
    l->teacher_id = "";
    l->classroom = strdup((char *)xmlNodeGetContent(classroom));
  } else {
    xmlXPathObjectPtr subject_html =
        xmlXPathEvalExpression((xmlChar *)".//*[@class=\"p\"]", context);
    xmlXPathObjectPtr classroom_html =
        xmlXPathEvalExpression((xmlChar *)".//*[@class=\"s\"]", context);

    xmlNodePtr teacher = teacher_html->nodesetval->nodeTab[0];
    xmlNodePtr subject = subject_html->nodesetval->nodeTab[0];
    xmlNodePtr classroom = classroom_html->nodesetval->nodeTab[0];

    l->lesson_name = strdup((char *)xmlNodeGetContent(subject));
    l->teacher_id = strdup((char *)xmlNodeGetContent(teacher));
    l->classroom = strdup((char *)xmlNodeGetContent(classroom));
  }

  return TIMETABLE_OK;
}

Error parse_3_spans(xmlXPathContextPtr context, Lesson *l, int i) {
  char subject_xpath[25];
  char teacher_xpath[25];
  char classroom_xpath[25];

  xmlXPathObjectPtr has_children =
      xmlXPathEvalExpression((xmlChar *)".//span[1]/span", context);
  if (has_children->nodesetval->nodeNr == 0) {
    sprintf(subject_xpath, ".//*[@class=\"p\"]");
    sprintf(teacher_xpath, ".//*[@class=\"n\"]");
    sprintf(classroom_xpath, ".//*[@class=\"s\"]");
  } else {
    sprintf(subject_xpath, ".//span[%i]/*[@class=\"p\"]", i + 1);
    sprintf(teacher_xpath, ".//span[%i]/*[@class=\"n\"]", i + 1);
    sprintf(classroom_xpath, ".//span[%i]/*[@class=\"s\"]", i + 1);
  }

  xmlXPathObjectPtr subject_html =
      xmlXPathEvalExpression((xmlChar *)subject_xpath, context);
  xmlXPathObjectPtr teacher_html =
      xmlXPathEvalExpression((xmlChar *)teacher_xpath, context);
  xmlXPathObjectPtr classroom_html =
      xmlXPathEvalExpression((xmlChar *)classroom_xpath, context);

  xmlNodePtr subject = subject_html->nodesetval->nodeTab[0];
  xmlNodePtr teacher = teacher_html->nodesetval->nodeTab[0];
  xmlNodePtr classroom = classroom_html->nodesetval->nodeTab[0];

  l->lesson_name = strdup((char *)xmlNodeGetContent(subject));
  l->teacher_id = strdup((char *)xmlNodeGetContent(teacher));
  l->classroom = strdup((char *)xmlNodeGetContent(classroom));

  return TIMETABLE_OK;
}

Error parse_lesson(xmlNodePtr lesson_cell, xmlXPathContextPtr context,
                   LessonArray *lesson_list, int order, char *hours,
                   char *ward_id, int weekday) {
  Lesson l;
  l.hours = strdup(hours);
  l.order = order;
  l.lesson_name = "";
  l.classroom = "";
  l.teacher_id = "";
  l.class_id = strdup(ward_id);
  l.weekday = weekday;

  if (lesson_cell == NULL) {
    fprintf(stderr, "ERROR: lessonCell is NULL\n");
    return TIMETABLE_ERROR;
  }

  if (xmlXPathSetContextNode(lesson_cell, context) != 0) {
    fprintf(stderr, "ERROR: Failed to set context node\n");
    return TIMETABLE_ERROR;
  }

  xmlXPathObjectPtr subject_html =
      xmlXPathEvalExpression((xmlChar *)".//span", context);

  // Handle o5, o20, o23
  if (subject_html == NULL || subject_html->nodesetval == NULL ||
      subject_html->nodesetval->nodeNr == 0) {
    if (subject_html)
      xmlXPathFreeObject(subject_html);

    parse_0_span(lesson_cell, &l);
    if (strcmp(l.lesson_name, " ")) {
      arrayPush(lesson_list, l);
    }
    // fprintf(stderr, "ERROR: XPath expression returned no results\n");
    return TIMETABLE_OK;
  }

  int count = subject_html->nodesetval->nodeNr;

  if (count == 2) { // Normal
    parse_2_spans(context, &l);
    arrayPush(lesson_list, l);
  } else if (count == 3) { // Single group
    parse_3_spans(context, &l, 0);
    arrayPush(lesson_list, l);
  } else if (count >= 6) { // Two groups or more
    // TODO: it iters correctly but always gets first element
    for (int i = 0; i < count / 3; ++i) {
      parse_3_spans(context, &l, i);
      arrayPush(lesson_list, l);
      l.hours = strdup(hours);
      l.order = order;
      l.lesson_name = "";
      l.classroom = "";
      l.teacher_id = "";
      l.class_id = strdup(ward_id);
    }
  }

  return TIMETABLE_OK;
}

Error parse_row(xmlNodePtr row, xmlXPathContextPtr context,
                LessonArray *lesson_list, char *ward_id) {
  xmlXPathSetContextNode(row, context);

  xmlNodePtr order_cell = xmlXPathEvalExpression((xmlChar *)".//td[1]", context)
                              ->nodesetval->nodeTab[0];
  if (order_cell == NULL) {
    return TIMETABLE_ERROR;
  }

  char *order_str = (char *)xmlNodeGetContent(order_cell);
  int order;
  sscanf(order_str, "%d", &order);

  xmlNodePtr hour_cell = xmlXPathEvalExpression((xmlChar *)".//td[2]", context)
                             ->nodesetval->nodeTab[0];
  if (hour_cell == NULL) {
    return TIMETABLE_ERROR;
  }
  char *hour = (char *)xmlNodeGetContent(hour_cell);

  int len = strlen(hour);
  for (int i = 0; i < len; ++i)
    if (isspace(hour[i]))
      for (int j = i; j < len; ++j)
        hour[j] = hour[j + 1];

  // printf("%s. %s - ", order_str, hour);

  for (int i = 2; i < 7; ++i) {
    xmlXPathSetContextNode(row, context);
    char xpath[100];
    sprintf(xpath, ".//td[%i]", i + 1);

    xmlXPathObjectPtr xpath_obj =
        xmlXPathEvalExpression((xmlChar *)xpath, context);

    if (xpath_obj == NULL || xpath_obj->nodesetval == NULL ||
        xpath_obj->nodesetval->nodeNr == 0) {
      if (xpath_obj)
        xmlXPathFreeObject(xpath_obj);
      return TIMETABLE_ERROR;
    }

    xmlNodePtr lesson_cell = xpath_obj->nodesetval->nodeTab[0];

    if (parse_lesson(lesson_cell, context, lesson_list, order, hour, ward_id,
                     i - 2) != TIMETABLE_OK) {
      xmlXPathFreeObject(xpath_obj);
      return TIMETABLE_ERROR;
    }

    xmlXPathFreeObject(xpath_obj);
  }

  // printf("\n");

  return TIMETABLE_OK;
}

Error parse_timetable(xmlXPathContextPtr context, LessonArray *lesson,
                      char *ward_id) {
  printf("INFO: Parsing timetable %s\n", ward_id);
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
    if (parse_row(row, context, lesson, ward_id) != TIMETABLE_OK) {
      continue;
    }
  }

  xmlXPathFreeObject(rows);
  return TIMETABLE_OK;
}

Error get_timetable(LessonArray *lesson_list, int i, Ward *ward,
                    CURL *curl_handle) {
  char timetable_path[100];
  sprintf(timetable_path, "http://zstrzeszow.pl/plan/plany/%s.html", ward->id);
  CURLResponse response = get_request(curl_handle, timetable_path);

  // parse the HTML document returned by the server
  htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size,
                                  NULL, NULL, HTML_PARSE_NOERROR);
  xmlXPathContextPtr context = xmlXPathNewContext(doc);

  // TODO: Do something with that
  char generationDate[100];
  if (get_generation_date(context, generationDate) != TIMETABLE_OK) {
    fprintf(stderr, "ERROR: Failed getting generation date\n");
  }

  char validDate[100];
  if (get_valid_date(context, validDate) != TIMETABLE_OK) {
    fprintf(stderr, "ERROR: Failed getting valid from date\n");
  }

  if (parse_timetable(context, lesson_list, ward->id) != TIMETABLE_OK) {
    fprintf(stderr, "ERROR: Failed parsing timetable\n");
    return TIMETABLE_ERROR;
  }

  return TIMETABLE_OK;
}
