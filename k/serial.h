#ifndef SERIAL_H
#define SERIAL_H

#include <k/types.h>

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
void serial_init(void);

/**
 * \brief Read a byte from the serial port.
 * \return The byte read from the serial port.
 */
char serial_read(void);

/**
 * \brief Write a buffer to the serial port.
 * \param buf The buffer to write.
 * \param count The number of bytes to write.
 * \return The number of bytes written.
 * \note This function blocks until all bytes are written.
 */
int serial_write(const char *buf, size_t count);

/**
 * \brief Write a buffer to the serial port.
 * \param buf The buffer to write.
 * \param count The number of bytes to write.
 * \return The number of bytes written.
 */
int write(const char *buf, size_t count);

#endif /* ! SERIAL_H */
