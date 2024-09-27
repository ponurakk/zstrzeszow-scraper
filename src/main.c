#include "main.h"
#include "database.h"
#include "scraper/timetable.h"
#include "server/listener.h"
#include "utils/array.h"
#include "utils/cellmap.h"
#include "utils/error.h"
#include "utils/logger.h"
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

void print();

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
    print_error("GET request failed: %s", curl_easy_strerror(res));
  }

  return response;
}

int save_in_memory_to_file(sqlite3 *in_memory_db, const char *filename) {
  sqlite3 *file_db;
  sqlite3_backup *backup;
  int rc;

  rc = sqlite3_open(filename, &file_db);
  if (rc != SQLITE_OK) {
    print_error("Cannot open file-based database: %s", sqlite3_errmsg(file_db));
    return rc;
  }

  backup = sqlite3_backup_init(file_db, "main", in_memory_db, "main");
  if (backup) {
    sqlite3_backup_step(backup, -1);
    sqlite3_backup_finish(backup);
  }

  rc = sqlite3_errcode(file_db);
  if (rc != SQLITE_OK) {
    print_error("Error occurred during backup: %s", sqlite3_errmsg(file_db));
  }

  sqlite3_close(file_db);

  return rc;
}

Error fetch_timetable(CURL *curl_handle, sqlite3 *db, char *timetable_url) {
  Error err;
  char list_url[100];
  sprintf(list_url, "%s/lista.html", timetable_url);
  CURLResponse response = get_request(curl_handle, list_url);

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

  print_info("Parsing wards list");
  for (int i = 0; i < ward_html_elements->nodesetval->nodeNr; ++i) {
    xmlNodePtr ward_html_element = ward_html_elements->nodesetval->nodeTab[i];
    get_ward_list(ward_list, ward_html_element, context, i);
    // printf("%s\t(%s)\n", wardList[i].full, wardList[i].id);

    err = add_ward(db, ward_list[i]);
    if (err != SQLITE_SUCCESS) {
      print_error("%s", error_to_string(err));
      return SCRAPER_ERROR;
    }
  }

  print_info("Parsing teachers list");
  for (int i = 0; i < teachers_html_elements->nodesetval->nodeNr; ++i) {
    xmlNodePtr teacher_html_element =
        teachers_html_elements->nodesetval->nodeTab[i];
    get_teachers_list(teacher_list, teacher_html_element, context, i);
    // printf("%s\t(%s)\n", teacherList[i].name, teacherList[i].id);

    err = add_teacher(db, teacher_list[i]);
    if (err != SQLITE_SUCCESS) {
      print_error("%s", error_to_string(err));
      return SCRAPER_ERROR;
    }
  }

  int ward_list_size = sizeof(ward_list) / sizeof(ward_list[0]);

  LessonArray lesson_list;
  arrayInit(&lesson_list, 50);
  char generation_date[100];

  print_info("Parsing timetable");
  for (int i = 0; i < ward_list_size; ++i) {
    err = get_timetable(&lesson_list, i, timetable_url, &ward_list[i],
                        curl_handle, generation_date);
    if (err != TIMETABLE_OK) {
      print_error("%s", error_to_string(err));
      return SCRAPER_ERROR;
    }
  }

  for (int i = 0; i < lesson_list.count; ++i) {
    print_info("Adding lesson %s from class %s to database",
               lesson_list.array[i].lesson_name, lesson_list.array[i].class_id);
    add_lesson(db, lesson_list.array[i]);
  }

  struct stat stats;
  stat("backup", &stats);
  if (!S_ISDIR(stats.st_mode))
    mkdir("backup", 0755);

  char backup_name[100];
  sprintf(backup_name, "backup/%s.db", generation_date);
  save_in_memory_to_file(db, backup_name);

  free(response.html);
  xmlXPathFreeContext(context);
  xmlFreeDoc(doc);
  arrayFree(&lesson_list);

  return SCRAPER_OK;
}

void print_pair(Cell key, LessonArray val) {
  print_info("Cell (%d, %d):\tLessons Size: %zu", key.x, key.y, val.count);
}

int main() {
  sqlite3 *db;
  Error err;

  int rc = sqlite3_open(":memory:", &db);

  if (sqlite_result(db, rc, "Opened database successfully") != SQLITE_SUCCESS) {
    sqlite3_close(db);
    return 1;
  }

  err = create_database(db);

  if (err != SQLITE_SUCCESS) {
    print_error("%s", error_to_string(err));
    return 1;
  }

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl_handle = curl_easy_init();
  char *timetable_url = "http://zstrzeszow.pl/plan";

  // err = fetch_timetable(curl_handle, db, timetable_url);
  // if (err != SCRAPER_OK) {
  //   return 1;
  //   xmlCleanupParser();
  //   curl_easy_cleanup(curl_handle);
  //   curl_global_cleanup();
  // }

  err = server(db);
  if (err != WEB_SERVER_OK) {
    print_error("Failed launching web server");
    return 1;
  }

  xmlCleanupParser();
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();

  return 0;
}
