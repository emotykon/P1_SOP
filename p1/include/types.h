//
// Created by motykaj on 12/15/25.
//
#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>
#include <limits.h>

#define MAX_ARGS 10
#define MAX_BACKUPS 100

// Struktura zadania backupu (widok rodzica)
typedef struct {
  pid_t pid;
  char source[PATH_MAX];
  char dest[PATH_MAX];
  int active;
} BackupTask;

// Struktura mapy inotify (widok dziecka)
typedef struct WatchMap {
  int wd;
  char path[PATH_MAX];
  struct WatchMap *next;
} WatchMap;

#endif