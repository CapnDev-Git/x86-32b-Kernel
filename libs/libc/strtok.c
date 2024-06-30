#include <stdio.h>
#include <string.h>

char *strtok(char *str, const char *delim) {
  static char *last;
  char *start;

  if (str != NULL) {
    last = str;
  }

  if (last == NULL) {
    return NULL;
  }

  // Skip leading delimiters
  start = last + strspn(last, delim);
  if (*start == '\0') {
    last = NULL;
    return NULL;
  }

  // Find end of token
  last = start + strcspn(start, delim);
  if (*last != '\0') {
    *last++ = '\0';
  } else {
    last = NULL;
  }

  return start;
}
