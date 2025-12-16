#define _DEFAULT_SOURCE
#include "utils.h"
#include "types.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int is_dir_empty(const char *path) {
  DIR *d = opendir(path);
  if (!d)
    return 1;
  struct dirent *dir;
  int n = 0;
  while ((dir = readdir(d)) != NULL) {
    if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
      n++;
      break;
    }
  }
  closedir(d);
  return n == 0;
}

void copy_file(const char *src, const char *dst, int update_only) {
  struct stat src_stat, dst_stat;
  if (lstat(src, &src_stat) == -1)
    return;

  if (update_only && lstat(dst, &dst_stat) == 0) {
    if (src_stat.st_size == dst_stat.st_size &&
        src_stat.st_mtime == dst_stat.st_mtime) {
      return;
    }
  }

  int in = open(src, O_RDONLY);
  if (in < 0)
    return;

  unlink(dst);

  int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
  if (out < 0) {
    close(in);
    return;
  }

  char buf[8192];
  ssize_t n;
  while ((n = read(in, buf, sizeof(buf))) > 0) {
    write(out, buf, n);
  }

  struct timespec times[2];
  times[0] = src_stat.st_atim;
  times[1] = src_stat.st_mtim;
  futimens(out, times);

  close(in);
  close(out);
}

char *resolve_symlink_target(const char *src_path, const char *link_content,
                             const char *root_src, const char *root_dst) {
  if (strncmp(link_content, root_src, strlen(root_src)) == 0) {
    char *new_target = malloc(PATH_MAX);
    snprintf(new_target, PATH_MAX, "%s%s", root_dst,
             link_content + strlen(root_src));
    return new_target;
  }
  return (char *)link_content;
}

void copy_symlink(const char *src, const char *dst, const char *root_src,
                  const char *root_dst) {
  char link_target[PATH_MAX];
  ssize_t len = readlink(src, link_target, sizeof(link_target) - 1);
  if (len == -1)
    return;
  link_target[len] = '\0';
  char *final_target =
      resolve_symlink_target(src, link_target, root_src, root_dst);
  unlink(dst);
  symlink(final_target, dst);
  if (final_target != link_target)
    free(final_target);
}

void copy_recursive(const char *src, const char *dst, const char *root_src,
                    const char *root_dst, int update_only) {
  struct stat st;
  if (lstat(src, &st) == -1)
    return;

  if (S_ISDIR(st.st_mode)) {
    mkdir(dst, st.st_mode);
    DIR *d = opendir(src);
    if (!d)
      return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
        continue;

      char new_src[PATH_MAX], new_dst[PATH_MAX];
      snprintf(new_src, sizeof(new_src), "%s/%s", src, dir->d_name);
      snprintf(new_dst, sizeof(new_dst), "%s/%s", dst, dir->d_name);

      copy_recursive(new_src, new_dst, root_src, root_dst, update_only);
    }
    closedir(d);
  } else if (S_ISLNK(st.st_mode)) {
    copy_symlink(src, dst, root_src, root_dst);
  } else if (S_ISREG(st.st_mode)) {
    copy_file(src, dst, update_only);
  }
}

void delete_recursive(const char *path) {
  struct stat st;
  if (lstat(path, &st) == -1)
    return;

  if (S_ISDIR(st.st_mode)) {
    DIR *d = opendir(path);
    if (d) {
      struct dirent *dir;
      while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
          continue;
        char subpath[PATH_MAX];
        snprintf(subpath, sizeof(subpath), "%s/%s", path, dir->d_name);
        delete_recursive(subpath);
      }
      closedir(d);
    }
    rmdir(path);
  } else {
    unlink(path);
  }
}