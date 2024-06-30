#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>

void *memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strdup(const char *s);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
char *strtok(char *str, const char *delim);
size_t strspn(const char *str, const char *accept);
size_t strcspn(const char *str, const char *reject);
char *strrchr(const char *s, int c);

#endif /* !STRING_H_ */
