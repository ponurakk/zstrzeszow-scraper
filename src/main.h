#ifndef MAIN_H
#define MAIN_H

#include <curl/curl.h>
#include <stdio.h>

typedef struct {
  char *html;
  size_t size;
} CURLResponse;

static size_t WriteHTMLCallback(void *contents, size_t size, size_t nmemb,
                                void *userp);

CURLResponse GetRequest(CURL *curl_handle, const char *url);

#endif // !MAIN_H
