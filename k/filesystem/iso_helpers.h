#ifndef ISO_HELPERS_H
#define ISO_HELPERS_H

#include <k/iso9660.h> // ISO structures
#include <k/types.h>   // u32
#include <stddef.h>    // size_t

#include "iso_filesystem.h"

/**
 * \brief Print a buffer as hexadecimal values
 * \param buffer The buffer to print
 * \param size The size of the buffer
 */
void print_buffer_hexa(void *buffer, size_t size);

/**
 * \brief Print a buffer as characters
 * \param buffer The buffer to print
 * \param size The size of the buffer
 */
void print_buffer_char(void *buffer, size_t size);

/**
 * \brief Print the Primary Volume Descriptor
 * \param pvd The Primary Volume Descriptor to print
 */
void print_volume_descriptor(struct iso_prim_voldesc *pvd);

/**
 * \brief Print a Path Table Entry
 * \param path_table The Path Table Entry to print
 */
void print_path_table_entry(struct iso_path_table_le *path_table);

/**
 * \brief Print all the Path Table Entries
 * \param path_table_buffer The Path Table buffer
 * \param path_table_size The size of the Path Table
 */
void print_path_table_entries(void *path_table_buffer, u32 path_table_size);

/**
 * \brief Print a Directory Entry
 * \param directory_entry The Directory Entry to print
 */
void print_directory_entry(struct iso_dir *directory_entry);

/**
 * \brief Print the Path Table IDs
 * \param path_table_ids The Path Table IDs to print
 */
void print_path_table_ids(struct path_table_ids *path_table_ids);

#endif /* ! ISO_HELPERS_H */
