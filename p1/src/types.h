#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>
#include <limits.h>

#define MAX_ARGS 10
#define MAX_BACKUPS 100

typedef struct {
  pid_t pid;
  char source[PATH_MAX];
  char dest[PATH_MAX];
  int active;
} BackupTask;

typedef struct WatchMap{
  int wd;
  char path[PATH_MAX];
  struct WatchMap *next;
} WatchMap;

#endif