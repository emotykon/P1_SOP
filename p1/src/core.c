#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include "core.h"
#include "types.h"
#include "utils.h"
#include "monitor.h"

static BackupTask tasks[MAX_BACKUPS];
static int task_count = 0;

void init_core() {
    task_count = 0;
    for(int i=0; i<MAX_BACKUPS; i++) tasks[i].active = 0;
}

// Pobiera źródło dla danej kopii (potrzebne do restore)
char* get_source_by_dest(const char *dest) {
    char dest_real[PATH_MAX];
    if (!realpath(dest, dest_real)) return NULL;

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
    printf("Aktywne kopie:\n");
    int active_found = 0;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].active) {
            // Sprawdź czy proces żyje (nieblokująco)
            int status;
            if (waitpid(tasks[i].pid, &status, WNOHANG) == 0) {
                printf("[%d] Źródło: %s  ->  Cel: %s\n", tasks[i].pid, tasks[i].source, tasks[i].dest);
                active_found = 1;
            } else {
                tasks[i].active = 0; // Proces umarł
            }
        }
    }
    if (!active_found) printf("(brak)\n");
}

void handle_end(int argc, char **argv) {
    for (int k = 1; k < argc; k++) {
        char dest_real[PATH_MAX];
        if (!realpath(argv[k], dest_real)) {
            printf("Błąd ścieżki: %s\n", argv[k]);
            continue;
        }
        
        int found = 0;
        for (int i = 0; i < task_count; i++) {
            if (tasks[i].active && strcmp(tasks[i].dest, dest_real) == 0) {
                kill(tasks[i].pid, SIGTERM);
                waitpid(tasks[i].pid, NULL, 0);
                tasks[i].active = 0;
                printf("Zakończono kopię do: %s\n", dest_real);
                found = 1;
            }
        }
        if (!found) printf("Nie znaleziono aktywnej kopii: %s\n", dest_real);
    }
}

void handle_add(int argc, char **argv) {
    if (argc < 3) {
        printf("Użycie: add <src> <dest1> [dest2 ...]\n");
        return;
    }

    char src_real[PATH_MAX];
    if (!realpath(argv[1], src_real)) {
        perror("Błąd ścieżki źródłowej");
        return;
    }

    for (int i = 2; i < argc; i++) {
        char dest_raw[PATH_MAX];
        strcpy(dest_raw, argv[i]);

        struct stat st;
        if (stat(dest_raw, &st) == 0) {
            if (!is_dir_empty(dest_raw)) {
                printf("Błąd: Katalog %s nie jest pusty.\n", dest_raw);
                continue;
            }
        } else {
            if (mkdir(dest_raw, 0755) == -1) {
                perror("Nie można utworzyć katalogu docelowego");
                continue;
            }
        }

        char dest_real[PATH_MAX];
        realpath(dest_raw, dest_real);

        // Walidacja pętli
        if (strncmp(dest_real, src_real, strlen(src_real)) == 0) {
            printf("Błąd: Nie można kopiować do wnętrza źródła (%s)\n", dest_real);
            continue;
        }

        // Sprawdź duplikaty
        int duplicate = 0;
        for (int j = 0; j < task_count; j++) {
            if (tasks[j].active && strcmp(tasks[j].dest, dest_real) == 0) {
                printf("Błąd: Kopia do %s już istnieje.\n", dest_real);
                duplicate = 1; 
                break;
            }
        }
        if (duplicate) continue;

        pid_t pid = fork();
        if (pid == 0) {
            // Dziecko
            run_backup_monitor(src_real, dest_real);
            exit(0);
        } else if (pid > 0) {
            // Rodzic
            tasks[task_count].pid = pid;
            strcpy(tasks[task_count].source, src_real);
            strcpy(tasks[task_count].dest, dest_real);
            tasks[task_count].active = 1;
            task_count++;
            printf("Uruchomiono: %s -> %s (PID: %d)\n", src_real, dest_real, pid);
        } else {
            perror("Fork error");
        }
    }
}