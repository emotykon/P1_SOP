#define _DEFAULT_SOURCE
#include "core.h"
#include "monitor.h"
#include "types.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static BackupTask tasks[MAX_BACKUPS];
static int task_count = 0;

void init_core() {
  task_count = 0;
  for (int i = 0; i < MAX_BACKUPS; i++)
    tasks[i].active = 0;
}
char *get_source_by_dest(const char *dest) {
  char dest_real[PATH_MAX];
  if (!realpath(dest, dest_real))
    return NULL;

  for (int i = 0; i < task_count; i++) {
    if (tasks[i].active && strcmp(tasks[i].dest, dest_real) == 0) {
      return tasks[i].source;
    }
  }
  return NULL;
}

void cleanup_all_processes() {
  for (int i = 0; i < task_count; i++) {
    if (tasks[i].active) {
      kill(tasks[i].pid, SIGTERM);
      waitpid(tasks[i].pid, NULL, 0);
    }
  }
}

void handle_list() {
  printf("Active copies:\n");
  int active_found = 0;
  for (int i = 0; i < task_count; i++) {
    if (tasks[i].active) {
      int status;
      if (waitpid(tasks[i].pid, &status, WNOHANG) == 0) {
        printf("[%d] Source: %s  ->  Target: %s\n", tasks[i].pid,
               tasks[i].source, tasks[i].dest);
        active_found = 1;
      } else {
        tasks[i].active = 0;
      }
    }
  }
  if (!active_found)
    printf("(not found)\n");
}

void handle_end(int argc, char **argv) {
  for (int k = 1; k < argc; k++) {
    char dest_real[PATH_MAX];
    if (!realpath(argv[k], dest_real)) {
      printf("Path error: %s\n", argv[k]);
      continue;
    }

    int found = 0;
    for (int i = 0; i < task_count; i++) {
      if (tasks[i].active && strcmp(tasks[i].dest, dest_real) == 0) {
        kill(tasks[i].pid, SIGTERM);
        waitpid(tasks[i].pid, NULL, 0);
        tasks[i].active = 0;
        found = 1;
      }
    }
    if (!found)
      printf("Copy not found\n");
  }
}
void handle_add(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: add <src> <dest1> [dest2 ...]\n");
    return;
  }
  char src_real[PATH_MAX];
  if (!realpath(argv[1], src_real)) {
    perror("Path error");
    return;
  }
  for (int i = 2; i < argc; i++) {
    char dest_raw[PATH_MAX];
    strcpy(dest_raw, argv[i]);
    struct stat st;
    if (stat(dest_raw, &st) == 0) {
      if (!is_dir_empty(dest_raw)) {
        printf("Error: Directory %s is not empty.\n", dest_raw);
        continue;
      }
    } else {
      if (mkdir(dest_raw, 0755) == -1) {
        perror("Directory creation error");
        continue;
      }
    }
    char dest_real[PATH_MAX];
    realpath(dest_raw, dest_real);

    if (strncmp(dest_real, src_real, strlen(src_real)) == 0) {
      printf("Error: Don't copy inside source (%s)\n", dest_real);
      continue;
    }
    int duplicate = 0;
    for (int j = 0; j < task_count; j++) {
      if (tasks[j].active && strcmp(tasks[j].dest, dest_real) == 0) {
        printf("Error: Copy %s already exist.\n", dest_real);
        duplicate = 1;
        break;
      }
    }
    if (duplicate)
      continue;

    pid_t pid = fork();
    if (pid == 0) {
      run_backup_monitor(src_real, dest_real);
      exit(0);
    } else if (pid > 0) {
      tasks[task_count].pid = pid;
      strcpy(tasks[task_count].source, src_real);
      strcpy(tasks[task_count].dest, dest_real);
      tasks[task_count].active = 1;
      task_count++;
      printf("Started: %s -> %s (PID: %d)\n", src_real, dest_real, pid);
    } else {
      perror("Fork error");
    }
  }
}