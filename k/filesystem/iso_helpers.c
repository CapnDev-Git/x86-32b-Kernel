#include "iso_helpers.h"

#include <k/iso9660.h>
#include <k/types.h>
#include <stdio.h>
#include <string.h>

/**
 * \brief Format the date in the ISO9660 format
 * \param date The date to format
 * \note The date is formatted as follows: YYYY-MM-DD HH:MM:SS UTC
 */
static void format_date(char *date) {
  char year[5];
  char month[3];
  char day[3];
  char hour[3];
  char minute[3];
  char second[3];
  memcpy(year, date, 4);
  year[4] = '\0';
  memcpy(month, date + 4, 2);
  month[2] = '\0';
  memcpy(day, date + 6, 2);
  day[2] = '\0';
  memcpy(hour, date + 8, 2);
  hour[2] = '\0';
  memcpy(minute, date + 10, 2);
  minute[2] = '\0';
  memcpy(second, date + 12, 2);
  second[2] = '\0';
  printf("%s-%s-%s %s:%s:%s UTC\n", year, month, day, hour, minute, second);
}

/**
 * \brief Print a buffer as hexadecimal values
 * \param buffer The buffer to print
 * \param size The size of the buffer
 */
void print_buffer_hexa(void *buffer, size_t size) {
  u8 *byte_buffer = (u8 *)buffer;
  for (size_t i = 0; i < size; i++) {
    printf("%02x ", byte_buffer[i]);
    if ((i + 1) % 16 == 0)
      printf("\n");
  }
}

/**
 * \brief Print a buffer as characters
 * \param buffer The buffer to print
 * \param size The size of the buffer
 */
void print_buffer_char(void *buffer, size_t size) {
  u8 *byte_buffer = (u8 *)buffer;
  for (size_t i = 0; i < size; i++) {
    printf("%c", byte_buffer[i]);
    if ((i + 1) % 16 == 0)
      printf("\n");
  }
}

/**
 * \brief Print the Primary Volume Descriptor
 * \param pvd The Primary Volume Descriptor to print
 */
void print_volume_descriptor(struct iso_prim_voldesc *pvd) {
  printf("--- Primary Volume Descriptor ---\n");
  printf("Volume Identifier: %.32s\n", pvd->vol_idf);
  printf("Data Preparer Identifier: %.128s\n", pvd->dprep_idf);
  printf("Volume Creation Date: ");
  format_date(pvd->date_creat);
  printf("Path Table Block Index in ISO: %d\n", pvd->le_path_table_blk);
  printf("Path Table Size (bytes): %d\n", pvd->path_table_size.le);
  printf("--- ! Primary Volume Descriptor ---\n");
}

/**
 * \brief Print a Path Table Entry
 * \param path_table The Path Table Entry to print
 */
void print_path_table_entry(struct iso_path_table_le *path_table) {
  // printf("--- Path Table Entry ---\n");
  // printf("Extended attribute record length: %d\n", path_table->ext_size);
  printf("Parent ID: %d\t", path_table->parent_dir);
  printf("Content at block index: %d\t", path_table->data_blk);
  printf("Directory name: %s\n", path_table->idf);
  // printf("--- ! Path Table Entry ---\n");
}

/**
 * \brief Print all the Path Table Entries
 * \param path_table_buffer The Path Table buffer
 * \param path_table_size The size of the Path Table
 */
void print_path_table_entries(void *path_table_buffer, u32 path_table_size) {
  u8 *byte_buffer = (u8 *)path_table_buffer;
  u8 *ptr = byte_buffer;
  u8 *ptr_end = ptr + path_table_size;

  // Print the name of each directory in the path table
  printf("--- Directory Entries ---\n");
  while (ptr < ptr_end) {
    void *table = ptr;
    struct iso_path_table_le *path_table = table;
    print_path_table_entry(path_table);

    // Move to the next entry
    size_t len = path_table->idf_len;
    ptr += sizeof(struct iso_path_table_le) + len + (len & 1);
  }
  printf("--- ! Directory Entries ---\n");
}

/**
 * \brief Print a Directory Entry
 * \param directory_entry The Directory Entry to print
 */
void print_directory_entry(struct iso_dir *directory_entry) {
  printf("--- Directory Entry ---\n");
  printf("Directory Record Length: %d\n", directory_entry->dir_size);
  printf("Extended Attribute Record Length: %d\n", directory_entry->ext_size);
  printf("File Data Block Index: %d\n", directory_entry->data_blk.le);
  printf("File Size: %d\n", directory_entry->file_size.le);
  printf("File Type: %d\n", directory_entry->type);
  printf("File Unit Size: %d\n", directory_entry->unit_size);
  printf("Interleave Gap Size: %d\n", directory_entry->gap_size);
  printf("Volume Sequence Number: %d\n", directory_entry->vol_seq.le);
  printf("Identifier Length: %d\n", directory_entry->idf_len);
  printf("File Name: %s\n", directory_entry->idf);
  printf("--- ! Directory Entry ---\n");

  // Print the directory entry as hexadecimal values
  print_buffer_hexa(directory_entry, directory_entry->dir_size);
  //   print_buffer_char(directory_entry, directory_entry->dir_size);
}

/**
 * \brief Print the Path Table IDs
 * \param path_table_ids The Path Table IDs to print
 */
void print_path_table_ids(struct path_table_ids *path_table_ids) {
  printf("--- Path Table IDs ---\n");
  for (u32 i = 0; i < path_table_ids->nb_dir; i++) {
    struct path_table_id *path_table_id = path_table_ids->ids[i];
    printf("ID: %d, Name: %s\n", path_table_id->id,
           path_table_id->path_table->idf);
  }
  printf("--- ! Path Table IDs ---\n");
}
