#ifndef MAIN_H
#define MAIN_H

#include <curl/curl.h>
#include <stdio.h>

typedef struct {
  char *html;
  size_t size;
} CURLResponse;

static size_t write_html_callback(void *contents, size_t size, size_t nmemb,
                                  void *userp);

CURLResponse get_request(CURL *curl_handle, const char *url);

#endif // !MAIN_H
