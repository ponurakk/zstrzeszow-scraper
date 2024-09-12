#include "main.h"
#include "array.h"
#include "database.h"
#include "error.h"
#include "list.h"
#include "timetable.h"
#include <libxml/HTMLparser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t write_html_callback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  CURLResponse *mem = (CURLResponse *)userp;
  char *ptr = realloc(mem->html, mem->size + realsize + 1);

  if (!ptr) {
    printf("Not enough memory available (realloc returned NULL)\n");
    return 0;
  }

  mem->html = ptr;
  memcpy(&(mem->html[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->html[mem->size] = 0;

  return realsize;
}

CURLResponse get_request(CURL *curl_handle, const char *url) {
  CURLcode res;
  CURLResponse response;

  // initialize the response
  response.html = malloc(1);
  response.size = 0;

  // specify URL to GET
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  // send all data returned by the server to WriteHTMLCallback
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_html_callback);
  // pass "response" to the callback function
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response);
  // set a User-Agent header
  curl_easy_setopt(
      curl_handle, CURLOPT_USERAGENT,
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/117.0.0.0 Safari/537.36");
  // perform the GET request
  res = curl_easy_perform(curl_handle);

  // check for HTTP errors
  if (res != CURLE_OK) {
    fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));
  }

  return response;
}

int main() {
  sqlite3 *db;
  Error err;

  int rc = sqlite3_open("./sqlite.db", &db);

  if (sqlite_result(db, rc, "Opened database successfully") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    exit(1);
  }

  err = create_database(db);

  if (err != SQLITE_SUCCESS) {
    fprintf(stderr, "%s\n", error_to_string(err));
    exit(1);
  }

  // initialize curl globally
  curl_global_init(CURL_GLOBAL_ALL);

  // initialize a CURL instance
  CURL *curl_handle = curl_easy_init();

  // get the HTML document associated with the page
  CURLResponse response =
      get_request(curl_handle, "http://zstrzeszow.pl/plan/lista.html");

  // parse the HTML document returned by the server
  htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size,
                                  NULL, NULL, HTML_PARSE_NOERROR);
  xmlXPathContextPtr context = xmlXPathNewContext(doc);

  xmlXPathObjectPtr ward_html_elements =
      xmlXPathEvalExpression((xmlChar *)"//ul[1]/li", context);

  xmlXPathObjectPtr teachers_html_elements =
      xmlXPathEvalExpression((xmlChar *)"//ul[2]/li", context);

  Ward ward_list[ward_html_elements->nodesetval->nodeNr];
  Teacher teacher_list[teachers_html_elements->nodesetval->nodeNr];

  printf("INFO: Parsing wards list\n");
  for (int i = 0; i < ward_html_elements->nodesetval->nodeNr; ++i) {
    xmlNodePtr ward_html_element = ward_html_elements->nodesetval->nodeTab[i];
    get_ward_list(ward_list, ward_html_element, context, i);
    // printf("%s\t(%s)\n", wardList[i].full, wardList[i].id);

    err = add_ward(db, ward_list[i]);
    if (err != SQLITE_SUCCESS) {
      fprintf(stderr, "%s\n", error_to_string(err));
      exit(1);
    }
  }

  printf("INFO: Parsing teachers list\n");
  for (int i = 0; i < teachers_html_elements->nodesetval->nodeNr; ++i) {
    xmlNodePtr teacher_html_element =
        teachers_html_elements->nodesetval->nodeTab[i];
    get_teachers_list(teacher_list, teacher_html_element, context, i);
    // printf("%s\t(%s)\n", teacherList[i].name, teacherList[i].id);

    err = add_teacher(db, teacher_list[i]);
    if (err != SQLITE_SUCCESS) {
      fprintf(stderr, "%s\n", error_to_string(err));
      exit(1);
    }
  }

  int ward_list_size = sizeof(ward_list) / sizeof(ward_list[0]);

  LessonArray lesson_list;
  arrayInit(&lesson_list, 50);

  printf("INFO: Parsing timetable\n");
  for (int i = 0; i < ward_list_size; ++i) {
    err = get_timetable(&lesson_list, i, &ward_list[i], curl_handle);
    if (err != TIMETABLE_OK) {
      fprintf(stderr, "%s\n", error_to_string(err));
      exit(1);
    }
  }

  for (int i = 0; i < lesson_list.count; ++i) {
    printf("INFO: Adding lesson %s from class %s to database\n",
           lesson_list.array[i].lesson_name, lesson_list.array[i].class_id);
    add_lesson(db, lesson_list.array[i]);
  }

  // free up the allocated resources
  free(response.html);
  xmlXPathFreeContext(context);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  arrayFree(&lesson_list);

  // cleanup the curl instance
  curl_easy_cleanup(curl_handle);
  // cleanup the curl resources
  curl_global_cleanup();

  return 0;
}
