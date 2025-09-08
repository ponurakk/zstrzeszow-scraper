#include "main.h"
#include "database.h"
#include "scraper/timetable.h"
#include "server/listener.h"
#include "utils/array.h"
#include "utils/cellmap.h"
#include "utils/error.h"
#include "utils/logger.h"
#include <dirent.h>
#include <libxml/HTMLparser.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

DbCacheArray gDb_cache;

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  size_t realsize = size * nmemb;
  CURLResponse *mem = (CURLResponse *)userp;

  char *ptr = realloc(mem->data, mem->size + realsize);
  if (ptr == NULL) {
    fprintf(stderr, "realloc failed\n");
    return 0;
  }

  mem->data = ptr;
  memcpy(&(mem->data[mem->size]), contents, realsize);
  mem->size += realsize;
  return realsize;
}

CURLResponse get_request(CURL *curl_handle, const char *url) {
  CURLcode res;
  CURLResponse response;

  // initialize the response
  response.data = malloc(1);
  response.size = 0;

  // specify URL to GET
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  // send all data returned by the server to WriteHTMLCallback
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
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

Error get_html(CURLResponse response, CURLResponse *html_data) {
  int to_child[2], from_child[2];

  if (pipe(to_child) == -1 || pipe(from_child) == -1) {
    perror("pipe");
    return IO_ERROR;
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return IO_ERROR;
  }

  if (pid == 0) {
    dup2(to_child[0], STDIN_FILENO);
    dup2(from_child[1], STDOUT_FILENO);

    close(to_child[1]);
    close(from_child[0]);

    // i'm not a fan of this approach but it works :P
    execlp("pdftohtml", "pdftohtml", "-enc", "UTF-8", "-noframes", "-s", "-i",
           "-stdout", "-", "out.html", (char *)NULL);

    perror("execlp");
    _exit(1);
  }

  close(to_child[0]);
  close(from_child[1]);

  // write PDF bytes into child stdin
  write(to_child[1], response.data, response.size);
  close(to_child[1]);
  free(response.data);

  // read HTML output
  CURLResponse html = {NULL, 0};
  char buf[4096];
  ssize_t n;
  while ((n = read(from_child[0], buf, sizeof(buf))) > 0) {
    char *ptr = realloc(html.data, html.size + n + 1);
    if (!ptr) {
      fprintf(stderr, "realloc failed\n");
      free(html.data);
      return IO_ERROR;
    }
    html.data = ptr;
    memcpy(html.data + html.size, buf, n);
    html.size += n;
    html.data[html.size] = '\0';
  }
  close(from_child[0]);

  html_data->size = html.size;
  html_data->data = strdup(html.data);

  free(html.data);

  return 0;
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

Error fetch_timetable(xmlXPathContextPtr context, sqlite3 *db,
                      char *timetable_url, char *generation_date) {
  Error err;

  LessonArray lesson_list;
  arrayInit(&lesson_list, 50);
  char *effective_date = NULL;

  print_info("Parsing timetable");
  err = get_timetable(&lesson_list, context, &effective_date);

  if (err != TIMETABLE_OK) {
    print_error("%s", error_to_string(err));
    return SCRAPER_ERROR;
  }

  add_date(db, effective_date, generation_date);

  print_info("Adding lessons to database");
  for (int i = 0; i < lesson_list.count; ++i) {
    add_lesson(db, lesson_list.array[i]);
  }

  char backup_name[100];
  sprintf(backup_name, "backup/%s.db", generation_date);
  save_in_memory_to_file(db, backup_name);

  xmlXPathFreeContext(context);
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

  char *generation_date = NULL;
  char *effective_date = NULL;

  char timetable_path[100];
  sprintf(timetable_path, "%s/plan%%20oddzia%%C5%%82%%C3%%B3w.pdf",
          timetable_url);
  CURLResponse response = get_request(curl_handle, timetable_path);
  CURLResponse html;
  get_html(response, &html);

  htmlDocPtr doc = htmlReadMemory(html.data, (unsigned long)html.size, NULL,
                                  NULL, HTML_PARSE_NOERROR);
  xmlXPathContextPtr context = xmlXPathNewContext(doc);

  if (get_effective_date(context, &effective_date) != TIMETABLE_OK) {
    print_error("Failed getting effective date");
  }

  if (get_generation_date(context, &generation_date) != TIMETABLE_OK) {
    print_error("Failed getting generation date");
  }

  char *last_generation_date = "";
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

    err = fetch_timetable(context, tmp_db, timetable_url, generation_date);
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
  // xmlXPathFreeContext(context);
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
