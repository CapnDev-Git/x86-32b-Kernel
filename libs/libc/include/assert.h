#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>

#ifndef NDEBUG
#define assert(exp)                                                            \
  do {                                                                         \
    if (!(exp)) {                                                              \
      printf("%s, %d: assertion '%s' failed\n", __BASE_FILE__, __LINE__,       \
             #exp);                                                            \
      printf("System halted.\n");                                              \
      while (1)                                                                \
        continue;                                                              \
    }                                                                          \
  } while (0)
#else
#define assert(exp) ((void)0)
#endif

#endif /* !ASSERT_H */
