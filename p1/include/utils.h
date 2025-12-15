#ifndef UTILS_H
#define UTILS_H

#include <sys/stat.h>

int is_dir_empty(const char *path);
void copy_file(const char *src, const char *dst, int update_only);
void copy_symlink(const char *src, const char *dst, const char *root_src, const char *root_dst);
void copy_recursive(const char *src, const char *dst, const char *root_src, const char *root_dst, int update_only);
void delete_recursive(const char *path);
char* resolve_symlink_target(const char *src_path, const char *link_content, const char *root_src, const char *root_dst);

#endif