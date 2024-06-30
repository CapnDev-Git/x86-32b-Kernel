#include "iso_filesystem.h"

#include "../memory.h"
#include "atapi.h"     // macros and structs for ATAPI
#include <k/iso9660.h> // ISO9660 structures
#include <k/types.h>   // u16, u32
#include <stdio.h>     // printf
#include <string.h>    // memcpy, strncmp, strtok

#define PVD_BLOCK 16
#define PVD_BLOCKS_LEN 1

static void print_buffer_hexa(void *buffer, size_t size) {
  u8 *byte_buffer = (u8 *)buffer;
  for (size_t i = 0; i < size; i++) {
    printf("%02x ", byte_buffer[i]);
    if ((i + 1) % (sizeof(struct iso_dir)) == 0)
      printf("\n");
  }
}

static void print_buffer_char(void *buffer, size_t size) {
  u8 *byte_buffer = (u8 *)buffer;
  for (size_t i = 0; i < size; i++) {
    printf("%c", byte_buffer[i]);
    if ((i + 1) % 16 == 0)
      printf("\n");
  }
}

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

void print_path_table_entry(struct iso_path_table_le *path_table) {
  // printf("--- Path Table Entry ---\n");
  // printf("Extended attribute record length: %d\n", path_table->ext_size);
  printf("Parent ID: %d\t", path_table->parent_dir);
  printf("Content at block index: %d\t", path_table->data_blk);
  printf("Directory name: %s\n", path_table->idf);
  // printf("--- ! Path Table Entry ---\n");
}

void print_path_table_directories(void *path_table_buffer,
                                  u32 path_table_size) {
  u8 *byte_buffer = (u8 *)path_table_buffer;
  u8 *start = byte_buffer;
  u8 *end = start + path_table_size;

  // Print the name of each directory in the path table
  printf("--- Directory Entries ---\n");
  while (start < end) {
    void *table = start;
    struct iso_path_table_le *path_table = table;
    print_path_table_entry(path_table);

    // Move to the next entry
    size_t len = path_table->idf_len;
    start += sizeof(struct iso_path_table_le) + len + (len & 1);
  }
  printf("--- ! Directory Entries ---\n");
}

void print_iso_dir_entries(void *directory_buffer, u32 directory_size) {
  u8 *byte_buffer = (u8 *)directory_buffer;
  u8 *start = byte_buffer;
  u8 *end = start + directory_size;

  // Print the name of each directory in the path table
  printf("--- Directory Entries ---\n");
  while (start < end) {
    void *table = start;
    struct iso_dir *directory_entry = table;
    print_directory_entry(directory_entry);

    // Move to the next entry
    size_t len = directory_entry->idf_len;
    start += directory_entry->dir_size;
  }
  printf("--- ! Directory Entries ---\n");
}

void print_directory_entry(struct iso_dir *directory_entry) {
  // printf("--- Directory Entry ---\n");
  // printf("Directory Record Length: %d\n", directory_entry->dir_size);
  // printf("Extended Attribute Record Length: %d\n",
  // directory_entry->ext_size); printf("File Data Block Index: %d\n",
  // directory_entry->data_blk.le); printf("File Size: %d\n",
  // directory_entry->file_size.le); printf("File Type: %d\n",
  // directory_entry->type); printf("File Unit Size: %d\n",
  // directory_entry->unit_size); printf("Interleave Gap Size: %d\n",
  // directory_entry->gap_size); printf("Volume Sequence Number: %d\n",
  // directory_entry->vol_seq.le); printf("Identifier Length: %d\n",
  // directory_entry->idf_len);
  printf("File Name: %s\n", directory_entry->idf);
  // printf("--- ! Directory Entry ---\n");

  // Print the directory entry as hexadecimal values
  // print_buffer_hexa(directory_entry, directory_entry->dir_size);
  // print_buffer_char(directory_entry, directory_entry->dir_size);
}

void print_path_table_ids(struct path_table_ids *path_table_ids) {
  printf("--- Path Table IDs ---\n");
  for (u32 i = 0; i < path_table_ids->nb_dir; i++) {
    struct path_table_id *path_table_id = path_table_ids->ids[i];
    printf("ID: %d, Name: %s\n", path_table_id->id,
           path_table_id->path_table->idf);
  }
  printf("--- ! Path Table IDs ---\n");
}

/****/

void *get_primary_volume_descriptor(void) {
  void *pvd_block_buffer = read_block(PVD_BLOCK);
  if (pvd_block_buffer == NULL) {
    printf("Failed to read block 16\n");
    return NULL; // Failure
  }

  printf("Primary Volume Descriptor read (block 16)\n");
  return pvd_block_buffer; // Success
}

/*****/

void *get_path_table(u32 path_table_index, u32 path_table_size) {
  // Determine the number of blocks to read
  u32 nb_blocks = path_table_size / CD_BLOCK_SZ;
  if (path_table_size % CD_BLOCK_SZ != 0)
    nb_blocks++; // Add an extra block if there is a remainder

  void *total_buffer = memory_reserve(nb_blocks * CD_BLOCK_SZ);
  if (total_buffer == NULL) {
    printf("Failed to reserve memory for path table\n");
    return NULL; // Failure
  }

  for (u32 i = 0; i < nb_blocks; i++) {
    void *block_buffer = read_block(path_table_index + i);
    if (block_buffer == NULL) {
      printf("Failed to read path table block\n");
      return NULL; // Failure
    }

    // Copy the block buffer to the total buffer
    memcpy(total_buffer + i * CD_BLOCK_SZ, block_buffer, CD_BLOCK_SZ);
    printf("Block %d copied to total buffer\n", path_table_index + i);

    // Free the block buffer
    memory_release(block_buffer);
    printf("Block %d freed\n", path_table_index + i);
  }

  printf("Path Table read (block %d)\n", path_table_index);
  return total_buffer; // Success
}

u32 get_path_table_length(void *path_table_buffer, u32 path_table_size) {
  u8 *byte_buffer = (u8 *)path_table_buffer;
  u8 *start = byte_buffer;
  u8 *end = start + path_table_size;

  // Count the number of directories in the path table
  u32 nb_directories = 0;
  while (start < end) {
    void *table = start;
    struct iso_path_table_le *path_table = table;
    nb_directories++;

    // Move to the next entry
    size_t len = path_table->idf_len;
    start += sizeof(struct iso_path_table_le) + len + (len & 1);
  }
  return nb_directories;
}

struct path_table_ids *get_path_table_ids(void *path_table_buffer,
                                          u32 path_table_size) {
  u8 *byte_buffer = (u8 *)path_table_buffer;
  u8 *start = byte_buffer;
  u8 *end = start + path_table_size;

  // Allocate memory for the path table IDs global structure
  struct path_table_ids *path_table_id_ptr =
      memory_reserve(sizeof(struct path_table_ids));
  if (path_table_id_ptr == NULL) {
    printf("Failed to reserve memory for path table IDs\n");
    return NULL; // Failure
  }

  // Determine the number of directories in the path table
  path_table_id_ptr->nb_dir =
      get_path_table_length(path_table_buffer, path_table_size);

  // Allocate memory for the path table IDs
  path_table_id_ptr->ids = memory_reserve(path_table_id_ptr->nb_dir *
                                          sizeof(struct path_table_id *));
  if (path_table_id_ptr->ids == NULL) {
    printf("Failed to reserve memory for path table IDs\n");
    return NULL; // Failure
  }

  // Print the name of each directory in the path table
  u32 curr_id = 1;
  start = byte_buffer;
  while (start < end) {
    void *table = start;
    struct iso_path_table_le *path_table = table;

    // Create a path table ID for the current entry
    struct path_table_id *path_table_id =
        memory_reserve(sizeof(struct path_table_id));
    if (path_table_id == NULL) {
      printf("Failed to reserve memory for path table ID\n");
      return NULL; // Failure
    }

    // Assign the path table and ID to the path table ID
    path_table_id->path_table = path_table;
    path_table_id->id = curr_id;
    path_table_id_ptr->ids[curr_id - 1] = path_table_id;

    // Move to the next entry
    size_t len = path_table->idf_len;
    start += sizeof(struct iso_path_table_le) + len + (len & 1);
    curr_id++;
  }

  printf("Path Table IDs created\n");
  return path_table_id_ptr; // Success
}

u32 get_id_from_name(struct path_table_ids *path_table_ids,
                     struct target_folder target) {
  // Loop through the path table IDs
  for (u32 i = 0; i < path_table_ids->nb_dir; i++) {
    struct path_table_id *path_table_id = path_table_ids->ids[i];
    if (strncmp(path_table_id->path_table->idf, target.directory_name,
                path_table_id->path_table->idf_len) == 0) {
      // If an ID is specified, check if it matches
      if (path_table_id->path_table->parent_dir == target.target_parent_id) {
        printf("Directory entry %s found\n", target.directory_name);
        return path_table_id->id; // Success
      }
    }
  }
  return 0; // Failure
}

void free_path_table_ids(struct path_table_ids *path_table_ids) {
  for (u32 i = 0; i < path_table_ids->nb_dir; i++)
    memory_release(path_table_ids->ids[i]);
  memory_release(path_table_ids);
}

void *get_path_table_entry(void *path_table_buffer, u32 path_table_size,
                           const char *directory_name) {
  u8 *byte_buffer = (u8 *)path_table_buffer;
  u8 *start = byte_buffer;
  u8 *end = start + path_table_size;

  // Print the name of each directory in the path table
  while (start < end) {
    void *table = start;
    struct iso_path_table_le *path_table = table;

    // Check if the directory name matches
    if (strncmp(path_table->idf, directory_name, path_table->idf_len) == 0) {
      print_path_table_entry(path_table);
      printf("Directory entry %s found\n", directory_name);
      return table; // Success
    }

    // Move to the next entry
    size_t len = path_table->idf_len;
    start += sizeof(struct iso_path_table_le) + len + (len & 1);
  }

  printf("Directory entry %s NOT found\n", directory_name);
  return NULL; // Failure
}

/*****/

void *get_directory_entry(void *path_table_buffer, u32 path_table_size,
                          const char *directory_name) {
  struct iso_path_table_le *directory_entry =
      get_path_table_entry(path_table_buffer, path_table_size, directory_name);
  if (directory_entry == NULL) {
    printf("Failed to get directory entry\n");
    return NULL; // Failure
  }

  // Get the block index of the directory
  u32 block_index = directory_entry->data_blk;
  void *directory_buffer = read_block(block_index);
  if (directory_buffer == NULL) {
    printf("Failed to read directory block\n");
    return NULL; // Failure
  }

  printf("Directory block %d read\n", block_index);
  return directory_buffer; // Success
}

/*****/

#define ROOT_ID 1

void *get_file_from_path(char *path) {
  const char *delim = "/"; // Delimiter for strtok
  char _path[256];         // Temporary buffer for path
  strcpy(_path, path);     // Copy the path to a temporary buffer

  // Get the target file name
  char *target_file = strrchr(path, '/'); // Find the last occurrence of '/'
  target_file++;                          // Move to the next character

  // Get the primary volume descriptor
  void *pvd_buffer = get_primary_volume_descriptor();
  if (pvd_buffer == NULL) {
    printf("Failed to get primary volume descriptor\n");
    return NULL; // Failure
  }
  struct iso_prim_voldesc *pvd = (struct iso_prim_voldesc *)pvd_buffer;

  // Get the path table buffer
  u32 path_table_index = pvd->le_path_table_blk;
  u32 path_table_size = pvd->path_table_size.le;
  void *path_table_buffer = get_path_table(path_table_index, path_table_size);
  if (path_table_buffer == NULL) {
    printf("Failed to get path table\n");
    return NULL; // Failure
  }
  print_path_table_directories(path_table_buffer, path_table_size);

  // Get the path table IDs
  struct path_table_ids *path_table_ids =
      get_path_table_ids(path_table_buffer, path_table_size);
  if (path_table_ids == NULL) {
    printf("Failed to get path table IDs\n");
    return NULL; // Failure
  }

  // Get the first directory name
  char *token = strtok(_path, delim);
  printf("Root directory: %s\n", token);
  if (token == NULL) {
    printf("No directory found in path\n");
    return NULL; // Failure
  }

  // Go through each directory in the path
  struct target_folder next_target;
  u32 current_id = ROOT_ID;
  while (strcmp(token, target_file) != 0) {
    next_target.directory_name = token;
    next_target.target_parent_id = current_id;

    // Get the ID of the current directory or file
    current_id = get_id_from_name(path_table_ids, next_target);
    if (current_id == 0) {
      printf("Directory %s not found\n", token);
      return NULL; // Failure
    }
    printf("ID of %s: %d\n", token, current_id);

    // Get the next token
    token = strtok(NULL, delim);
  }

  // Read the content of the last visited directory's block from last ID
  u32 target_block_index =
      path_table_ids->ids[current_id - 1]->path_table->data_blk;

  void *target_dir_buffer = read_block(target_block_index);
  if (target_dir_buffer == NULL) {
    printf("Failed to read target block\n");
    return NULL; // Failure
  }
  printf("Target block %d read\n", target_block_index);

  print_directory_entry((struct iso_dir *)target_dir_buffer);

  struct iso_dir *target_dir = (struct iso_dir *)target_dir_buffer;
  u32 target_dir_size = target_dir->dir_size;

  print_buffer_hexa(target_dir_buffer, CD_BLOCK_SZ);

  u8 *byte_buffer = (u8 *)target_dir_buffer;
  u8 *start = byte_buffer;
  u8 *end = start + CD_BLOCK_SZ;
  while (start < end && start < byte_buffer + CD_BLOCK_SZ) {
    void *table = start;
    struct iso_dir *directory_entry = table;
    print_directory_entry(directory_entry);

    // Move to the next entry
    size_t len = directory_entry->idf_len;
    start += sizeof(struct iso_dir);
  }

  // print_buffer_hexa(target_buffer, CD_BLOCK_SZ);
  // print_buffer_char(target_buffer, CD_BLOCK_SZ);

  // Read the block at the target block index
  // struct iso_dir *target_dir = (struct iso_dir *)target_buffer;
  // void *random_entry = target_buffer + 2 * sizeof(struct iso_dir);
  // print_buffer_hexa(random_entry, sizeof(struct iso_dir) * 3);
  // print_buffer_char(random_entry, sizeof(struct iso_dir) * 3);

  // print_iso_dir_entries(target_buffer, target_dir->dir_size);

  // Read the content of the target file's block
  // u32 target_file_block_index = target_dir->data_blk.le;
  // void *target_file_buffer = read_block(target_file_block_index);
  // if (target_file_buffer == NULL) {
  //   printf("Failed to read target file block\n");
  //   return NULL; // Failure
  // }
  // printf("Target file block %d read\n", target_file_block_index);
  // print_buffer_hexa(target_file_buffer, CD_BLOCK_SZ);

  // TODO FREE MEMORY
  // free_path_table_ids(path_table_ids);
  // memory_release(path_table_buffer);
  // memory_release(pvd_buffer);

  return NULL; // Success
}
