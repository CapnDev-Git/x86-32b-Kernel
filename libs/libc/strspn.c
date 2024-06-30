#include <stddef.h>

size_t strspn(const char *str, const char *accept) {
  const char *p, *a;
  size_t count = 0;

  for (p = str; *p != '\0'; p++) {
    for (a = accept; *a != '\0'; a++) {
      if (*p == *a) {
        break;
      }
    }
    if (*a == '\0') {
      return count;
    }
    count++;
  }
  return count;
}
