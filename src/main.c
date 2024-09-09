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

static size_t WriteHTMLCallback(void *contents, size_t size, size_t nmemb,
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

CURLResponse GetRequest(CURL *curl_handle, const char *url) {
  CURLcode res;
  CURLResponse response;

  // initialize the response
  response.html = malloc(1);
  response.size = 0;

  // specify URL to GET
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  // send all data returned by the server to WriteHTMLCallback
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteHTMLCallback);
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

  int rc = sqlite3_open(":memory:", &db);

  if (sqliteResult(db, rc, "Opened database successfully") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    exit(1);
  }

  err = createDatabase(db);

  if (err != SQLITE_SUCCESS) {
    fprintf(stderr, "%s\n", errorToString(err));
    exit(1);
  }

  // initialize curl globally
  curl_global_init(CURL_GLOBAL_ALL);

  // initialize a CURL instance
  CURL *curl_handle = curl_easy_init();

  // get the HTML document associated with the page
  CURLResponse response =
      GetRequest(curl_handle, "http://zstrzeszow.pl/plan/lista.html");

  // parse the HTML document returned by the server
  htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size,
                                  NULL, NULL, HTML_PARSE_NOERROR);
  xmlXPathContextPtr context = xmlXPathNewContext(doc);

  xmlXPathObjectPtr wardHTMLElements =
      xmlXPathEvalExpression((xmlChar *)"//ul[1]/li", context);

  xmlXPathObjectPtr teachersHTMLElements =
      xmlXPathEvalExpression((xmlChar *)"//ul[2]/li", context);

  Ward wardList[wardHTMLElements->nodesetval->nodeNr];
  Teacher teacherList[teachersHTMLElements->nodesetval->nodeNr];

  printf("INFO: Parsing wards list\n");
  for (int i = 0; i < wardHTMLElements->nodesetval->nodeNr; ++i) {
    xmlNodePtr wardHTMLElement = wardHTMLElements->nodesetval->nodeTab[i];
    getWardList(wardList, wardHTMLElement, context, i);
    // printf("%s\t(%s)\n", wardList[i].full, wardList[i].id);

    err = addWard(db, wardList[i]);
    if (err != SQLITE_SUCCESS) {
      fprintf(stderr, "%s\n", errorToString(err));
      exit(1);
    }
  }

  printf("INFO: Parsing teachers list\n");
  for (int i = 0; i < teachersHTMLElements->nodesetval->nodeNr; ++i) {
    xmlNodePtr teacherHTMLElement =
        teachersHTMLElements->nodesetval->nodeTab[i];
    getTeachersList(teacherList, teacherHTMLElement, context, i);
    // printf("%s\t(%s)\n", teacherList[i].name, teacherList[i].id);

    err = addTeacher(db, teacherList[i]);
    if (err != SQLITE_SUCCESS) {
      fprintf(stderr, "%s\n", errorToString(err));
      exit(1);
    }
  }

  int wardListSize = sizeof(wardList) / sizeof(wardList[0]);

  LessonArray lessonList;
  arrayInit(&lessonList, 50);

  printf("INFO: Parsing timetable\n");
  for (int i = 0; i < wardListSize; ++i) {
    err = getTimetable(&lessonList, i, &wardList[i], curl_handle);
    if (err != TIMETABLE_OK) {
      fprintf(stderr, "%s\n", errorToString(err));
      exit(1);
    }
  }

  for (int i = 0; i < lessonList.count; ++i) {
    printf("INFO: Adding lesson %s from class %s to database\n",
           lessonList.array[i].lesson_name, lessonList.array[i].class_id);
    addLesson(db, lessonList.array[i]);
  }

  // free up the allocated resources
  free(response.html);
  xmlXPathFreeContext(context);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  arrayFree(&lessonList);

  // cleanup the curl instance
  curl_easy_cleanup(curl_handle);
  // cleanup the curl resources
  curl_global_cleanup();

  return 0;
}
