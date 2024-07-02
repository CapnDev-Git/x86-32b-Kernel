#include "iso_filesystem.h"

#include "../memory.h"   // xmemory_reserve, memory_release
#include "atapi.h"       // macros and structs for ATAPI
#include "iso_helpers.h" // print_buffer_hexa
#include <k/iso9660.h>   // ISO9660 structures
#include <k/types.h>     // u8, u32
#include <stdio.h>       // printf
#include <string.h>      // memcpy, strncmp, strtok, strcpy, strrchr

#define PVD_BLOCK 16
#define PVD_BLOCKS_LEN 1
#define ROOT_ID 1
#define START_ID 1

/**
 * \brief Structure representing a target folder
 * \param directory_name The name of the target directory
 * \param parent_id The parent ID of the target directory
 * \note The target folder structure is used to navigate through the path table
 * to find the target file
 */
struct target_folder {
  const char *directory_name;
  u32 parent_id;
};

/**
 * \brief Retrieves the path table buffer from the CD-ROM (ISO9660)
 * \param path_table_index The index of the path table block found in the
 * Primary Volume Descriptor
 * \param path_table_size The size of the path table
 * \return The path table buffer (success) or NULL (failure)
 * \note The path table buffer must be released with memory_release after use
 * \note The path table buffer is a concatenation of all path table blocks
 * necessary to read the entire path table (size is a multiple of
 * ISO_BLOCK_SIZE)
 */
static void *get_path_table(u32 path_table_index, u32 path_table_size) {
  // Determine the number of blocks to read
  u32 nb_blocks = path_table_size / ISO_BLOCK_SIZE;
  if (path_table_size % ISO_BLOCK_SIZE != 0)
    nb_blocks++; // Add an extra block if there is a remainder

  // Allocate memory for the total buffer
  void *total_buffer = xmemory_reserve(nb_blocks * ISO_BLOCK_SIZE);
  if (total_buffer == NULL) {
    printf("Failed to allocate memory for path table\n");
    return NULL; // Failure
  }

  // Concatenate all path table blocks into the total buffer
  for (u32 i = 0; i < nb_blocks; i++) {
    // Read the current path table block
    void *tmp_block_buffer = read_block(path_table_index + i);
    if (tmp_block_buffer == NULL) {
      memory_release(total_buffer);
      printf("Failed to read path table block\n");
      return NULL; // Failure
    }

    // Copy the bytes of the block buffer to the total buffer
    memcpy(total_buffer + i * ISO_BLOCK_SIZE, tmp_block_buffer, ISO_BLOCK_SIZE);
    printf("Block %d copied to total buffer\n", path_table_index + i);

    // Free the temporary block buffer
    memory_release(tmp_block_buffer);
    printf("Block %d freed\n", path_table_index + i);
  }

  // Return the allocated buffer (needs to be released after use)
  printf("Path Table read (block %d)\n", path_table_index);
  return total_buffer; // Success
}

/**
 * \brief Retrieves the number of directories in the path table
 * \param path_table_buffer The buffer containing the path table
 * \param path_table_size The size of the path table
 * \return The number of directories in the path table
 */
static u32 get_path_table_length(void *path_table_buffer, u32 path_table_size) {
  u8 *byte_buffer = (u8 *)path_table_buffer; // Cast to bytes to move through
  u8 *ptr = byte_buffer;                     // Pointer to the buffer
  u8 *ptr_end = ptr + path_table_size;       // End of the buffer

  // Count the number of directories in the path table
  u32 nb_directories = 0;
  while (ptr < ptr_end) {
    void *table = ptr; // Pointer to the current entry
    struct iso_path_table_le *path_table = table; // Implicit casting
    nb_directories++;

    // Move to the next entry
    size_t len = path_table->idf_len;
    ptr += sizeof(struct iso_path_table_le) + len + (len & 1);
  }

  // Return the number of directories counted
  return nb_directories;
}

/**
 * \brief Frees the path table IDs structure
 * \param path_table_ids The path table IDs structure to free
 */
static void free_path_table_ids(struct path_table_ids *path_table_ids) {
  for (u32 i = 0; i < path_table_ids->nb_dir; i++)
    memory_release(path_table_ids->ids[i]);
  memory_release(path_table_ids->ids);
  memory_release(path_table_ids);
}

/**
 * \brief Retrieves the path table IDs from the path table buffer
 * \param path_table_buffer The buffer containing the path table
 * \param path_table_size The size of the path table
 * \return The path table IDs structure (success) or NULL (failure)
 * \note The path table IDs structure must be released with memory_release after
 * use. It is used to retrieve the ID of a directory which allows for navigation
 * through the path table & ISO9660 filesystem.
 */
static struct path_table_ids *get_path_table_ids(void *path_table_buffer,
                                                 u32 path_table_size) {
  // Allocate memory for the path table IDs global structure
  struct path_table_ids *path_table_id_ptr =
      xmemory_reserve(sizeof(struct path_table_ids));
  if (path_table_id_ptr == NULL) {
    printf("Failed to allocate memory for path table IDs structure\n");
    return NULL; // Failure
  }

  // Determine the number of directories in the path table
  path_table_id_ptr->nb_dir =
      get_path_table_length(path_table_buffer, path_table_size);
  if (path_table_id_ptr->nb_dir == 0) {
    memory_release(path_table_id_ptr);
    printf("No directories found in path table. Aborting\n");
    return NULL; // Failure
  }

  // Allocate memory for the path table IDs for each directory (entry)
  path_table_id_ptr->ids = xmemory_reserve(path_table_id_ptr->nb_dir *
                                           sizeof(struct path_table_id *));
  if (path_table_id_ptr->ids == NULL) {
    memory_release(path_table_id_ptr);
    printf("Failed to allocate memory for path table IDs\n");
    return NULL; // Failure
  }

  // Loop through the path table entries
  u8 *byte_buffer = (u8 *)path_table_buffer; // Cast to bytes to move through
  u8 *ptr = byte_buffer;                     // Pointer to the buffer
  u8 *ptr_end = ptr + path_table_size;       // End of the buffer
  u32 curr_id = START_ID;
  while (ptr < ptr_end) {
    void *table = ptr;
    struct iso_path_table_le *path_table = table;

    // Create a path table ID for the current entry
    struct path_table_id *path_table_id =
        xmemory_reserve(sizeof(struct path_table_id));
    if (path_table_id == NULL) {
      free_path_table_ids(path_table_id_ptr);
      printf("Failed to allocate memory for path table ID\n");
      return NULL; // Failure
    }

    // Assign the path table and ID to the path table ID
    path_table_id->path_table = path_table;
    path_table_id->id = curr_id;
    path_table_id_ptr->ids[curr_id - 1] = path_table_id;

    // Move to the next entry
    size_t len = path_table->idf_len;
    ptr += sizeof(struct iso_path_table_le) + len + (len & 1);
    curr_id++;
  }

  printf("Path Table IDs created\n");
  return path_table_id_ptr; // Success
}

/**
 * \brief Retrieves the ID of a directory from its name
 * \param path_table_ids The path table IDs structure
 * \param target The target folder structure containing the directory name and
 * parent ID
 * \return The ID of the directory (success) or 0 (failure)
 * \note The ID is used to navigate through the path table and retrieve the
 * target file entry
 * \note The ID is 0 if the directory is not found
 */
static u32 get_id_from_name(struct path_table_ids *path_table_ids,
                            struct target_folder target) {
  // Loop through the path table IDs
  for (u32 i = 0; i < path_table_ids->nb_dir; i++) {
    // Check if the directory name matches the target directory name
    struct path_table_id *path_table_id = path_table_ids->ids[i];
    if (strncmp(path_table_id->path_table->idf, target.directory_name,
                path_table_id->path_table->idf_len) == 0) {
      // If an ID is specified, check if it matches
      if (path_table_id->path_table->parent_dir == target.parent_id) {
        printf("Directory entry %s found\n", target.directory_name);
        return path_table_id->id; // Success
      }
    }
  }

  return 0; // Failure
}

/**
 * \brief Reads the primary volume descriptor from the CD-ROM (ISO9660)
 * \return The primary volume descriptor buffer (success) or NULL (failure)
 * \note The primary volume descriptor buffer must be released with
 * memory_release after use
 * \note The primary volume descriptor describes the entire CD-ROM filesystem
 * structure and is used to retrieve the path table and other necessary
 * information like the root directory entry
 * \note The primary volume descriptor is located at block 16 (PVD_BLOCK)
 */
static void *get_primary_volume_descriptor(void) {
  void *pvd_block_buffer = read_block(PVD_BLOCK);
  if (pvd_block_buffer == NULL) {
    printf("Failed to read block 16\n");
    return NULL; // Failure
  }

  printf("Primary Volume Descriptor read (block 16)\n");
  return pvd_block_buffer; // Success
}

/**
 * \brief Retrieves the file entry buffer from the target directory
 * \param target_dir The target directory buffer
 * \param target_file The name of the target file
 * \return The file entry buffer (success) or NULL (failure)
 * \note The file entry buffer must be released with memory_release after use
 * \note The file entry buffer is a copy of the target file entry found in the
 * target directory in order to allow returning the buffer to the caller
 */
static void *get_file_entry(struct iso_dir *target_dir,
                            const char *target_file) {
  // Loop through the target directory & search for the target file
  void *ptr = target_dir; // Pointer to the target directory
  void *ptr_end = target_dir + target_dir->dir_size; // End of the directory
  while (ptr < ptr_end) {
    struct iso_dir *dir_entry = ptr;

    // Break if the length of the identifier is 0 (end of directory)
    if (dir_entry->idf_len == 0)
      break; // Finished reading the directory (file not found)

    // Copy the name of the file
    u8 len = dir_entry->idf_len - 2;
    char name[256];
    memcpy(name, dir_entry->idf, len);
    name[len] = '\0';

    // Check if the name matches the target file
    if (strncmp(name, target_file, len) == 0) {
      printf("File '%s' found at 0x%08p\n", target_file, ptr);

      // Allocate memory for the found file entry buffer
      size_t struct_size = sizeof(struct iso_dir);
      void *found_buffer = xmemory_reserve(struct_size);
      if (found_buffer == NULL) {
        printf("Failed to allocate memory for found file entry buffer\n");
        return NULL; // Failure
      }

      // Copy the target file to the final buffer
      memcpy(found_buffer, dir_entry, struct_size);
      printf("File entry found buffer created\n");
      return found_buffer; // Success
    }

    // Move to the next entry
    ptr += dir_entry->dir_size;
  }

  printf("File '%s' not found\n", target_file);
  return NULL; // Failure
}

/**
 * \brief Navigates through the path table to find the target file
 * \param fs The ISO9660 filesystem structure
 * \param path The path to the target file
 * \param target_file The name of the target file
 * \return The ID of the target file directory (success) or 0 (failure)
 * \note The ID is used to retrieve the target file entry
 * \note The ID is 0 if the target file is not found
 * \note The path is used to navigate through the path table to find the target
 * file
 * \note The target file is the last part of the path
 * \note The path is a string with directories separated by '/'
 */
static u32 navigate_path(struct iso_filesystem *fs, char *path,
                         char *target_file) {
  // Copy the path to a temporary buffer (tokenize copy not original pointer)
  const char *delim = "/"; // Delimiter for strtok
  char _path[256];         // Temporary buffer for path
  strcpy(_path, path);     // Copy the path to a temporary buffer

  // Tokenize the path to navigate through the directories
  char *token = strtok(_path, delim); // Get the root directory name
  if (token == NULL) {
    printf("No directory found in path\n");
    return 0; // Failure
  }

  // Go through each directory in the path
  struct target_folder next_target;
  u32 target_parent_id = ROOT_ID;
  while (strcmp(token, target_file) != 0) {
    printf("Navigating to directory %s\n", token);

    next_target.directory_name = token;       // Set the searched directory name
    next_target.parent_id = target_parent_id; // Set the parent ID

    // Get the ID of the current directory or file
    target_parent_id = get_id_from_name(fs->pt_ids, next_target);
    if (target_parent_id == 0) {
      printf("Directory %s not found\n", token);
      return 0; // Failure
    }

    // Get the next token
    token = strtok(NULL, delim);
  }

  return target_parent_id; // Success
}

struct iso_filesystem *init_filesystem(void) {
  // Get the primary volume descriptor
  void *pvd_buffer = get_primary_volume_descriptor();
  if (pvd_buffer == NULL) {
    printf("Failed to get primary volume descriptor\n");
    return NULL; // Failure
  }

  // Extract data block & size from the PVD
  struct iso_prim_voldesc *pvd = (struct iso_prim_voldesc *)pvd_buffer;
  u32 path_table_index = pvd->le_path_table_blk;
  u32 path_table_size = pvd->path_table_size.le;

  // Get the path table buffer
  void *pt_buffer = get_path_table(path_table_index, path_table_size);
  if (pt_buffer == NULL) {
    memory_release(pvd_buffer);
    printf("Failed to get path table\n");
    return NULL; // Failure
  }

  // Get the path table IDs
  struct path_table_ids *pt_ids =
      get_path_table_ids(pt_buffer, path_table_size);
  if (pt_ids == NULL) {
    memory_release(pt_buffer);
    memory_release(pvd_buffer);
    printf("Failed to get path table IDs\n");
    return NULL; // Failure
  }

  // Allocate memory for the filesystem structure
  struct iso_filesystem *fs = xmemory_reserve(sizeof(struct iso_filesystem));
  if (fs == NULL) {
    free_path_table_ids(pt_ids);
    memory_release(pt_buffer);
    memory_release(pvd_buffer);
    printf("Failed to allocate memory for filesystem\n");
    return NULL; // Failure
  }

  // Assign the buffers to the filesystem structure
  fs->pvd = pvd_buffer;
  fs->pt = pt_buffer;
  fs->pt_ids = pt_ids;
  return fs; // Success
}

void free_filesystem(struct iso_filesystem *fs) {
  free_path_table_ids(fs->pt_ids);
  memory_release(fs->pt);
  memory_release(fs->pvd);
  memory_release(fs);
}

void *get_file_from_path(char *path) {
  // Get the filesystem
  struct iso_filesystem *fs = init_filesystem();
  if (!fs) {
    printf("Failed to load ISO9660 filesystem\n");
    return NULL; // Failure
  }

  // Extract the target file name from the path
  char *target_file = strrchr(path, '/'); // Find the last part of the path
  target_file++;                          // Skip the '/' character

  // Navigate to the last visited directory
  u32 target_parent_id = navigate_path(fs, path, target_file);
  if (target_parent_id == 0) {
    free_filesystem(fs);
    printf("Failed to navigate path\n");
    return NULL; // Failure
  }

  // Fetch the content of the last visited directory
  struct path_table_id *target_dir_id = fs->pt_ids->ids[target_parent_id - 1];
  struct iso_path_table_le *target_path_table = target_dir_id->path_table;
  u32 target_block_index = target_path_table->data_blk;
  void *target_dir_buffer = read_block(target_block_index);
  if (target_dir_buffer == NULL) {
    free_filesystem(fs);
    printf("Failed to read target block\n");
    return NULL; // Failure
  }

  // Get the target file entry
  struct iso_dir *target_dir = (struct iso_dir *)target_dir_buffer;
  void *target_file_entry = get_file_entry(target_dir, target_file);
  if (target_file_entry == NULL) {
    memory_release(target_dir_buffer);
    free_filesystem(fs);
    printf("Failed to get target file entry\n");
    return NULL; // Failure
  }

  // Free fs & unused buffers
  memory_release(target_dir_buffer);
  free_filesystem(fs);

  return target_file_entry; // Success
}
