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

/**
 * \brief Initialize the serial port for communication on COM1.
 *
 * The serial port is configured with the following settings:
 *  * No interrupts
 *  * Baud rate: 38400
 *  * Data length: 8 bits
 *  * Parity: None
 *  * Stop bits: 1
 *  * FIFO enabled
 *  * Both receiver and transmitter FIFO queues cleared
 *  * THR empty interrupt enabled
 *
 * The serial port is configured to use COM1 (0x3f8).
 */
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

/**
 * \brief Check if the serial port is ready to transmit data.
 * \return 1 if the port is ready, 0 otherwise.
 */
static int is_transmit_empty(void) {
  return inb(SERIAL_PORT + LSR_OFFSET) & THR_MASK;
}

/**
 * \brief Read a byte from the serial port.
 * \return The byte read from the serial port.
 */
char serial_read(void) {
  while (!serial_received())
    ;
  return inb(SERIAL_PORT);
}

/**
 * \brief Write a buffer to the serial port.
 * \param buf The buffer to write.
 * \param count The number of bytes to write.
 * \return The number of bytes written.
 * \note This function blocks until all bytes are written.
 */
int serial_write(const char *buf, size_t count) {
  for (size_t i = 0; i < count; i++) {
    // Wait until the Transmitter Holding Register is empty
    while (!is_transmit_empty())
      ;

    // Write the byte to the Transmitter Holding Register
    outb(SERIAL_PORT, buf[i]);
  }
}

/**
 * \brief Write a buffer to the serial port.
 * \param buf The buffer to write.
 * \param count The number of bytes to write.
 * \return The number of bytes written.
 */
int write(const char *buf, size_t count) {
  serial_write(buf, count);
  return count;
}
