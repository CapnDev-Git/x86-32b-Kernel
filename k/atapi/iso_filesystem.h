#ifndef ISO_FILESYSTEM_H_
#define ISO_FILESYSTEM_H_

#include <k/iso9660.h>
#include <k/types.h>

struct path_table_id {
  struct iso_path_table_le *path_table;
  u32 id;
};

struct path_table_ids {
  struct path_table_id **ids;
  u32 nb_dir;
};

struct target_folder {
  const char *directory_name;
  u32 target_parent_id;
};

void print_volume_descriptor(struct iso_prim_voldesc *pvd);
void print_path_table_entry(struct iso_path_table_le *path_table);
void print_path_table_directories(void *path_table_buffer, u32 path_table_size);
void print_directory_entry(struct iso_dir *directory_entry);
void print_path_table_ids(struct path_table_ids *path_table_ids);

u32 get_path_table_length(void *path_table_buffer, u32 path_table_size);
void *get_primary_volume_descriptor(void);
void *get_path_table(u32 path_table_index, u32 path_table_size);
void *get_path_table_entry(void *path_table_buffer, u32 path_table_size,
                           const char *directory_name);
void *get_directory_entry(void *path_table_buffer, u32 path_table_size,
                          const char *directory_name);
void *get_file_from_path(char *path);

struct path_table_ids *get_path_table_ids(void *path_table_buffer,
                                          u32 path_table_size);
u32 get_id_from_name(struct path_table_ids *path_table_ids,
                     struct target_folder target_folder);
void free_path_table_ids(struct path_table_ids *path_table_ids);

#endif /* ! ISO_FILESYSTEM_H_ */
