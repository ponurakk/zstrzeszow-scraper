#include "main.h"
#include "database.h"
#include "scraper/timetable.h"
#include "server/listener.h"
#include "utils/array.h"
#include "utils/cellmap.h"
#include "utils/logger.h"
#include <dirent.h>
#include <libxml/HTMLparser.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

DbCacheArray gDb_cache;

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
  char effective_date[100];

  print_info("Parsing timetable");
  for (int i = 0; i < ward_list_size; ++i) {
    err = get_timetable(&lesson_list, i, timetable_url, &ward_list[i],
                        curl_handle, generation_date, effective_date);
    if (err != TIMETABLE_OK) {
      print_error("%s", error_to_string(err));
      return SCRAPER_ERROR;
    }
  }

  add_date(db, effective_date, generation_date);

  print_info("Adding lessons to database");
  for (int i = 0; i < lesson_list.count; ++i) {
    add_lesson(db, lesson_list.array[i]);
  }

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

int compare_db_cache(const void *a, const void *b) {
  return strcmp(((const DbCache *)a)->date, ((const DbCache *)b)->date);
}

void update_timetable(int signum) {
  print_debug("Fetching newest timetable...");
  Error err;
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl_handle = curl_easy_init();
  char *timetable_url = TIMETABLE_URL;

  char generation_date[100];
  char effective_date[100];

  char timetable_path[100];
  sprintf(timetable_path, "%s/plany/o1.html", timetable_url);
  CURLResponse response = get_request(curl_handle, timetable_path);
  htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size,
                                  NULL, NULL, HTML_PARSE_NOERROR);
  xmlXPathContextPtr context = xmlXPathNewContext(doc);

  free(response.html);

  if (get_generation_date(context, generation_date) != TIMETABLE_OK) {
    print_error("Failed getting generation date");
  }

  if (get_effective_date(context, effective_date) != TIMETABLE_OK) {
    print_error("Failed getting valid from date");
  }

  char *last_generation_date;
  if (gDb_cache.array[gDb_cache.count - 1].date != NULL) {
    last_generation_date = gDb_cache.array[gDb_cache.count - 1].date;
  }

  if (strcmp(generation_date, last_generation_date) != 0) {
    sqlite3 *tmp_db;
    int rc = sqlite3_open(":memory:", &tmp_db);

    if (sqlite_result(tmp_db, rc, "Opened database successfully") !=
        SQLITE_SUCCESS) {
      sqlite3_close(tmp_db);
      return;
    }

    err = create_database(tmp_db);

    if (err != SQLITE_SUCCESS) {
      print_error("%s", error_to_string(err));
      return;
    }

    err = fetch_timetable(curl_handle, tmp_db, timetable_url);
    if (err != SCRAPER_OK) {
      return;
      xmlCleanupParser();
      curl_easy_cleanup(curl_handle);
      curl_global_cleanup();
    }

    sqlite3_close(tmp_db);

    char db_path[22];
    sprintf(db_path, "./backup/%s.db", generation_date);

    sqlite3 *db;
    rc = sqlite3_open(db_path, &db);
    DbCache cache = {.db = db, .date = strdup(generation_date)};

    arrayPush(&gDb_cache, cache);
    qsort(gDb_cache.array, gDb_cache.count, sizeof(DbCache), compare_db_cache);

    print_debug("Successfully fetched %s", generation_date);
  }

  xmlCleanupParser();
  xmlFreeDoc(doc);
  xmlXPathFreeContext(context);
  curl_easy_cleanup(curl_handle);
}

int main() {
  Error err;
  arrayInit(&gDb_cache, 8);

  struct stat stats;
  stat("backup", &stats);
  if (!S_ISDIR(stats.st_mode))
    mkdir("backup", 0755);

  struct dirent *de;
  DIR *dir = opendir("./backup");

  if (dir == NULL) {
    printf("Could not open current directory");
    return 0;
  }

  while ((de = readdir(dir)) != NULL) {
    if (strcmp(de->d_name, "..") != 0 && strcmp(de->d_name, ".") != 0) {
      char db_path[22];
      sprintf(db_path, "./backup/%s", de->d_name);

      sqlite3 *db;
      int rc = sqlite3_open(db_path, &db);
      DbCache cache = {.db = db, .date = strdup(strtok(de->d_name, "."))};

      if (sqlite_result(db, rc, "Opened database successfully") !=
          SQLITE_SUCCESS) {
        sqlite3_close(db);
        return 1;
      }

      arrayPush(&gDb_cache, cache);
    }
  }
  closedir(dir);

  qsort(gDb_cache.array, gDb_cache.count, sizeof(DbCache), compare_db_cache);

  struct itimerval timer;
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));

  // Install signal handler for SIGALRM
  sa.sa_handler = &update_timetable;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGALRM, &sa, NULL);

  // Configure the timer to expire after 1 second, then every 1 second
  timer.it_value.tv_sec = 1;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 60 * 60 * 24;
  timer.it_interval.tv_usec = 0;

  // Start the timer
  setitimer(ITIMER_REAL, &timer, NULL);

  err = server(&gDb_cache);
  if (err != WEB_SERVER_OK) {
    print_error("Failed launching web server");
    return 1;
  }

  for (int i = 0; i < gDb_cache.count; ++i) {
    free(gDb_cache.array[i].date);
    sqlite3_close(gDb_cache.array[i].db);
  }

  arrayFree(&gDb_cache);
  xmlCleanupParser();
  curl_global_cleanup();

  return 0;
}
