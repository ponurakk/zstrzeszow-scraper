#include <string.h>

char *get_content_type(const char *path) {
  const char *ext = strrchr(path, '.');

  // If no extension is found, default to binary stream
  if (!ext || ext == path) {
    return "application/octet-stream";
  }

  if (strcmp(ext, ".html") == 0) {
    return "text/html";
  } else if (strcmp(ext, ".css") == 0) {
    return "text/css";
  } else if (strcmp(ext, ".js") == 0) {
    return "application/javascript";
  } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
    return "image/jpeg";
  } else {
    return "application/octet-stream";
  }
}
