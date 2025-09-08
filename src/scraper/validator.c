#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_day(const char *str) {
  if (strcmp(str, "Poniedziałek ") == 0 || strcmp(str, "Wtorek ") == 0 ||
      strcmp(str, "Środa ") == 0 || strcmp(str, "Czwartek ") == 0 ||
      strcmp(str, "Piątek ") == 0) {
    return 0;
  }

  return 1;
}

int is_valid_number_nbsp(const char *s) {
  if (!s)
    return 1;

  if (s[0] < '1' || s[0] > '9')
    return 1;

  if (s[1] >= '0' && s[1] <= '2') {
    // Two-digit number (10,11,12)
    if (s[2] == (char)0xC2 && s[3] == (char)0xA0 && s[4] == '\0')
      return 0;
    else
      return 1;
  } else {
    // One-digit number (1-9)
    if (s[1] == (char)0xC2 && s[2] == (char)0xA0 && s[3] == '\0')
      return 0;
    else
      return 1;
  }
}

int is_valid_date_format(const char *str, char **out) {
  int year, month, day;

  if (sscanf(str, "%2d.%2d.%4d", &day, &month, &year) == 3) {
  } else if (sscanf(str, "%2d-%2d-%4d", &day, &month, &year) == 3) {
  } else {
    const char *date_start = strstr(str, "–");
    if (!date_start)
      return 1;

    date_start += 3;
    while (*date_start == ' ' || *date_start == '\t' || *date_start == '\302' ||
           *date_start == '\240') {
      date_start++;
    }

    if (sscanf(date_start, "%2d.%2d.%4d", &day, &month, &year) != 3) {
      return 1;
    }
  }

  *out = malloc(11);
  if (!*out)
    return 1;

  snprintf(*out, 11, "%04d-%02d-%02d", year, month, day);

  return 0;
}

int is_valid_hour_format(const char *str) {
  int n1, n2, n3, n4;
  if (sscanf(str, "%2d:%2d - %2d:%2d", &n1, &n2, &n3, &n4) != 4)
    return 1;

  return 0;
}

int is_valid_class_format(const char *str) {
  if (!isdigit(*str))
    return 1;
  str++;

  int count = 0;
  while (isupper(*str)) {
    count++;
    str++;
  }
  if (count < 2)
    return 1;

  while (*str == ' ' || (unsigned char)*str == 0xC2 ||
         isspace((unsigned char)*str)) {
    str++;
  }

  return 0;
}

int is_wrapped_in_parentheses(const char *s) {
  if (!s || s[0] != '(')
    return 1;

  return 0;
}
