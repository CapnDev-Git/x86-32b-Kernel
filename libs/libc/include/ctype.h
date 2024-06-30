#ifndef CTYPE_H_
#define CTYPE_H_

static inline int tolower(int c) {
  if (c >= 'A' && c <= 'Z')
    return c + ('a' - 'A');
  return c;
}

static inline int toupper(int c) {
  if (c >= 'a' && c <= 'z')
    return c - ('a' - 'A');
  return c;
}

#endif /* !CTYPE_H_ */
