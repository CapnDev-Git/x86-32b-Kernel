#include "serial.h"

#include "io.h"
#include <k/types.h>

// Serial port addresses
#define COM1 0x3f8
#define SERIAL_PORT COM1

// UART registers offsets & masks
#define DLAB 0x80
#define DLL_VALUE 0x03
#define DLM_VALUE 0x00
#define _8N1 0x03
#define IFIFO 0xC7
#define IER_THREI 0x02

// Line Status Register (LSR) offsets & masks
#define LSR_OFFSET 5
#define RDA_MASK 0x01
#define THR_MASK 0x20

void serial_init(void) {
  // Disable all interrupts
  outb(SERIAL_PORT + 1, 0x00);

  // Enable DLAB (set baud rate divisor)
  outb(SERIAL_PORT + 3, DLAB);
  outb(SERIAL_PORT + 0, DLL_VALUE);
  outb(SERIAL_PORT + 1, DLM_VALUE);

  // Set 8-bits length, no parity, one stop bit
  outb(SERIAL_PORT + 3, _8N1);

  // Enable FIFO, clear both receiver and transmitter FIFO queues
  outb(SERIAL_PORT + 2, IFIFO);

  // Enable THR empty interrupt
  outb(SERIAL_PORT + 1, IER_THREI);
}

/**
 * \brief Check if the serial port has received data.
 * \return 1 if data is available, 0 otherwise.
 */
static int serial_received(void) {
  return inb(SERIAL_PORT + LSR_OFFSET) & RDA_MASK;
}

char serial_read(void) {
  while (!serial_received())
    ;
  return inb(SERIAL_PORT);
}

/**
 * \brief Check if the serial port is ready to transmit data.
 * \return 1 if the port is ready, 0 otherwise.
 */
static int is_transmit_empty(void) {
  return inb(SERIAL_PORT + LSR_OFFSET) & THR_MASK;
}

int serial_write(const char *buf, size_t count) {
  for (size_t i = 0; i < count; i++) {
    // Wait until the Transmitter Holding Register is empty
    while (!is_transmit_empty())
      ;

    // Write the byte to the Transmitter Holding Register
    outb(SERIAL_PORT, buf[i]);
  }
}

int write(const char *buf, size_t count) {
  serial_write(buf, count);
  return count;
}
