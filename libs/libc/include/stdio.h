#ifndef STDIO_H_
#define STDIO_H_

#include <stdarg.h>

int puts(const char *s);
int printf(const char *format, ...);
int sprintf(char *buf, const char *format, ...);
int vsprintf(char *buf, const char *format, va_list args);

#endif /* !STDLIB_H_ */
