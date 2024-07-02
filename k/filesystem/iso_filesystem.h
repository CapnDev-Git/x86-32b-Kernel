#ifndef ISO_FILESYSTEM_H
#define ISO_FILESYSTEM_H

#include <k/iso9660.h> // struct path_table_ids
#include <k/types.h>   // u8

/**
 * \brief Structure representing a path table by an ID
 * \param path_table The path table entry address (pointer)
 * \param id The ID of the path table entry
 * \note The ID is used to navigate through the path table and retrieve the
 * target file entry
 */
struct path_table_id {
  struct iso_path_table_le *path_table;
  u32 id;
};

/**
 * \brief Structure representing a list of path table IDs
 * \param ids The list of path table IDs
 * \param nb_dir The number of directories in the path table
 * \note The path table IDs structure is used to retrieve the ID of a directory
 * which allows for navigation through the path table & ISO9660 filesystem
 */
struct path_table_ids {
  struct path_table_id **ids;
  u32 nb_dir;
};

/**
 * \brief Structure to hold the filesystem & its buffers
 * \param pvd Pointer to the Primary Volume Descriptor (PVD)
 * \param pt Pointer to the Path Table
 * \param pt_ids Pointer to the Path Table IDs
 * \note The PVD is the first sector of the ISO9660 filesystem and contains
 * information about the filesystem. The Path Table contains the directory
 * structure of the filesystem. The Path Table IDs are used to locate the
 * Path Table. The Path Table is used to locate the files in the filesystem.
 */
struct iso_filesystem {
  void *pvd;
  void *pt;
  struct path_table_ids *pt_ids;
};

/**
 * \brief Initialize the ISO9660 filesystem
 * \return A pointer to the ISO9660 filesystem structure
 * \note The function initializes the ISO9660 filesystem by reading the Primary
 * Volume Descriptor (PVD) and the Path Table. The Path Table IDs are also
 * initialized to allow for navigation through the filesystem.
 * \note The function returns NULL if the filesystem could not be initialized
 * successfully
 * \note The function allocates memory for the filesystem structure and its
 * buffers and must be freed using free_filesystem()
 */
struct iso_filesystem *init_filesystem(void);

/**
 * \brief Free the ISO9660 filesystem
 * \param iso_filesystem The ISO9660 filesystem structure to free
 * \note The function frees the memory allocated for the ISO9660 filesystem
 * structure and its buffers.
 */
void free_filesystem(struct iso_filesystem *iso_filesystem);

/**
 * \brief Get the file buffer from a path
 * \param path The path of the file to retrieve
 * \return A pointer to the file buffer (struct iso_dir)
 * \note The function retrieves the file buffer from the ISO9660 filesystem
 * structure by navigating through the filesystem using the path provided.
 * \note The function returns NULL if the file could not be found in the
 * filesystem.
 * \note The function allocates memory for the file buffer and must be freed
 * using memory_release()
 */
void *get_file_from_path(char *path);

#endif /* ! ISO_FILESYSTEM_H_ */
