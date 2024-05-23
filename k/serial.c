#include "serial.h"

#include "io.h"
#include <k/types.h>

static void enable_baud_rate_divisor(u16 port) {
  // Enable DLAB (set baud rate divisor)
  outb(port + 3, 0x80);
  outb(port + 0, 0x03); // Set divisor to 3 (low byte) 38400 baud
  outb(port + 1, 0x00); //                  (high byte)
}

void serial_init(u16 port) {
  // Disable all interrupts
  outb(port + 1, 0x00); // (b5->b0 = 0, b6 & b7 reserved)

  // Enable DLAB (set baud rate divisor)
  enable_baud_rate_divisor(port);

  outb(port + 3, 0x03); // set 8bits length, no parity
  outb(port + 2, 0xC7); // enable FIFO + clear [transmit|receive], int. 14B
  outb(port + 1, 0x02); // enabling THR empty interrupt
}

int serial_read(u16 port) { return inb(port + THR_OFFSET); }

int serial_received(u16 port) { return inb(port + LSR_OFFSET) & 0x20; }

int serial_write(u16 port, const char *buf, size_t count) {
  for (size_t i = 0; i < count; i++) {
    // Wait until the Transmitter Holding Register is empty
    while (!serial_received(port))
      ;

    // Write the byte to the Transmitter Holding Register
    outb(port + THR_OFFSET, buf[i]);
  }
  return count;
}
