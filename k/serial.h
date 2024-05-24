#ifndef SERIAL_H
#define SERIAL_H

#include <k/types.h>

void serial_init(void);
char serial_read(void);
int serial_write(const char *buf, size_t count);
int write(const char *buf, size_t count);

#endif /* ! SERIAL_H */
