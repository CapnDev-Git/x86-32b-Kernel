#include <stddef.h>

size_t strcspn(const char *str, const char *reject) {
  const char *p, *r;
  size_t count = 0;

  for (p = str; *p != '\0'; p++) {
    for (r = reject; *r != '\0'; r++) {
      if (*p == *r) {
        return count;
      }
    }
    count++;
  }
  return count;
}
