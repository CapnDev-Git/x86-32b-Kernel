#include "syscalls_io.h"

#define MAX_FILE_DESCRIPTORS 16

#include "../../memory.h"
#include "atapi.h"
#include "iso_filesystem.h"
#include "iso_helpers.h"
#include <k/types.h>
#include <stdio.h>
#include <string.h>

/**
 * \brief Structure representing a file descriptor
 * \param iso_file The ISO9660 file structure
 * \param offset The current file reading offset
 * \note The file descriptor structure is used to keep track of the file being
 * read and the current reading offset in the file
 */
struct file_descriptor {
  struct iso_dir *iso_file;
  int offset;
};

/**
 * \brief Structure representing a file descriptor table
 * \param table The file descriptor table
 * \note The file descriptor table structure is used to keep track of the file
 * descriptors in use by the system
 */
struct file_descriptor_table {
  struct file_descriptor table[MAX_FILE_DESCRIPTORS];
};

struct file_descriptor_table fd_table;

/**
 * \brief Convert a string to uppercase
 * \param str The string to convert
 * \param len The length of the string
 * \note The function converts a string to uppercase by iterating over each
 * character and converting it to uppercase if it is a lowercase letter
 * \note The function modifies the input string in place
 * \note This function is used to convert a path to be opened to uppercase to
 * match the ISO9660 filesystem.
 */
static void path_to_upper(char *str, int len) {
  for (int i = 0; i < len; i++)
    // Convert lowercase letters to uppercase in place
    if (str[i] >= 'a' && str[i] <= 'z')
      str[i] -= 32;
}

/**
 * \brief Delete a file descriptor from the file descriptor table
 * \param fd The file descriptor to delete
 * \note The function releases all resources associated with the file descriptor
 * and clears the file descriptor entry in the file descriptor table
 * \note This function is used to close a file and release the file descriptor
 */
static void delete_fd(int fd) {
  // Release all resources associated with the file descriptor
  memory_release(fd_table.table[fd].iso_file);

  // Clear the file descriptor entry
  fd_table.table[fd].iso_file = NULL;
  fd_table.table[fd].offset = 0;
}

int open(const char *pathname, int flags) {
  // Only support read-only files
  if (flags != O_RDONLY)
    return -1;

  // Copy pathname to new buffer
  int len = strlen(pathname);
  char iso_path[len + 1];
  strcpy(iso_path, pathname);
  iso_path[len] = '\0';

  // Convert path to uppercase
  path_to_upper(iso_path, len);

  // Look for free file descriptor
  for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
    if (fd_table.table[i].iso_file == NULL) {
      // Fetch file iso_dir buffer from path
      void *file_buffer = get_file_from_path(iso_path);
      if (file_buffer == NULL) {
        printf("open: file not found\n");
        return -1;
      }

      // Store file buffer in fd_table
      fd_table.table[i].iso_file = (struct iso_dir *)file_buffer;
      fd_table.table[i].offset = 0;

      // Return file descriptor
      return i; // Success
    }
  }

  // No free file descriptors
  printf("fs_open: no free file descriptors\n");
  return -1; // Failure
}

ssize_t read(int fd, void *buf, size_t count) {
  // Check if file descriptor is valid
  if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
    return -1;

  // Get the file buffer from the file descriptor
  struct iso_dir *file = fd_table.table[fd].iso_file;
  if (file == NULL)
    return -1;

  // Get the current offset and file size
  u32 offset = fd_table.table[fd].offset;
  u32 size = file->file_size.le;

  // Prevent reading past the end of the file
  if (offset >= size)
    return 0;

  // Calculate the number of bytes to read
  u32 remaining = size - offset;
  if (count > remaining)
    count = remaining;

  // Compute the data block and read offset
  u32 data_block = file->data_blk.le + offset / ISO_BLOCK_SIZE;
  u32 read_offset = offset % ISO_BLOCK_SIZE;
  u32 amount = (read_offset + count + ISO_BLOCK_SIZE - 1) / ISO_BLOCK_SIZE;

  // Read the data blocks
  size_t total = 0;
  for (u32 i = 0; i < amount; i++) {
    void *data = read_block(data_block + i);
    if (data == NULL) {
      printf("Failed to read\n");
      return -1;
    }

    // Don't consider the read offset for the first block
    if (i == 0) {
      // Compute the number of bytes to copy from the first block
      u32 left_to_copy = ISO_BLOCK_SIZE - read_offset;
      u32 to_copy = left_to_copy < count ? left_to_copy : count;

      // Copy the data to the buffer
      memcpy(buf, ((u8 *)data) + read_offset, to_copy);
      total += to_copy; // Update the total number of bytes read
    } else if (total + ISO_BLOCK_SIZE < count) {
      // Copy the entire block to the buffer if no more bytes are needed
      memcpy(buf + total, ((u8 *)data), ISO_BLOCK_SIZE);
      total += ISO_BLOCK_SIZE;
    } else {
      // Copy the remaining bytes to the buffer
      memcpy(buf + total, ((u8 *)data), count - total);
      total += count - total;
    }

    // Release the current data block
    memory_release(data);
  }

  // Update the file descriptor offset for next read
  fd_table.table[fd].offset += count;

  // Return the number of bytes read
  return count;
}

off_t seek(int fd, off_t offset, int whence) {
  // Check if file descriptor is valid
  if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
    return -1;

  // Get the file buffer from the file descriptor
  struct iso_dir *file = fd_table.table[fd].iso_file;
  if (file == NULL)
    return -1;

  // Get the current file size
  u32 size = file->file_size.le;

  // Compute the new offset based on the whence parameter
  switch (whence) {
  case SEEK_SET:
    // Check if the new offset is valid & within the file size
    if (offset < 0 || offset > (int)size)
      return -1;

    // Set the new offset to the specified value
    fd_table.table[fd].offset = offset;
    break;
  case SEEK_CUR:
    // Check if the new offset is within the file size
    if (fd_table.table[fd].offset + offset > (int)size)
      return -1;

    // Update the offset by the specified value
    fd_table.table[fd].offset += offset;
    break;
  case SEEK_END:
    // Check if the new offset is within the file size
    if (offset < -(int)size || offset > 0)
      return -1;

    // Set the new offset to the end of the file
    fd_table.table[fd].offset = size + offset;
    break;
  default:
    // Invalid whence parameter
    return -1;
  }

  // Return the new file offset
  return fd_table.table[fd].offset;
}

int close(int fd) {
  // Check if file descriptor is valid
  if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
    return -1;

  // Free the resources allocated for the file descriptor & clear the entry
  delete_fd(fd);
  return 0;
}
