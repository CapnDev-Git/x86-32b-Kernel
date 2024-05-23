#ifndef SERIAL_H
#define SERIAL_H

#include <k/types.h>

#define COM1 0x3f8   // COM1 base port
#define COM2 0x2f8   // COM2 base port
#define COM3 0x3e8   // COM3 base port
#define COM4 0x2e8   // COM4 base port
#define LSR_OFFSET 5 // Line Status Register offset
#define THR_OFFSET 0 // Transmitter Holding Register offset

void serial_init(u16 port);
int serial_read(u16 port);
int serial_received(u16 port);
int serial_write(u16 port, const char *buf, size_t count);

#endif /* ! SERIAL_H */
