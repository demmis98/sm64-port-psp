#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <stddef.h>

#define FS_MAX_PATH 1024

void fs_init(const char *argv0);
void fs_resolve_path(char *buffer, size_t size, const char *filename);
void fs_prepare_parent_path(const char *filename);

#endif
