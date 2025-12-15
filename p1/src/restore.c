#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "restore.h"
#include "utils.h"
#include "core.h"
#include "types.h"

void sync_restore(const char *curr_src, const char *curr_dst, const char *root_src, const char *root_dst) {

    DIR *d = opendir(curr_dst);
    if (!d) return;
    struct dirent *dir;

    struct stat st_dst;
    lstat(curr_dst, &st_dst);
    mkdir(curr_src, st_dst.st_mode);

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

        char s_path[PATH_MAX], d_path[PATH_MAX];
        snprintf(s_path, sizeof(s_path), "%s/%s", curr_src, dir->d_name);
        snprintf(d_path, sizeof(d_path), "%s/%s", curr_dst, dir->d_name);

        struct stat st;
        lstat(d_path, &st);
        
        if (S_ISDIR(st.st_mode)) {
            sync_restore(s_path, d_path, root_src, root_dst);
        } else if (S_ISLNK(st.st_mode)) {
            copy_symlink(d_path, s_path, root_dst, root_src);
        } else {
            copy_file(d_path, s_path, 1);
        }
    }
    closedir(d);

    d = opendir(curr_src);
    if (!d) return;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
        
        char s_path[PATH_MAX], d_path[PATH_MAX];
        snprintf(s_path, sizeof(s_path), "%s/%s", curr_src, dir->d_name);
        snprintf(d_path, sizeof(d_path), "%s/%s", curr_dst, dir->d_name);

        if (access(d_path, F_OK) == -1) {
            delete_recursive(s_path);
        }
    }
    closedir(d);
}

void handle_restore(const char *dest_path) {
    char dest_real[PATH_MAX];
    if (!realpath(dest_path, dest_real)) {
        printf("Error: Invalid path.\n");
        return;
    }

    char *src_real = get_source_by_dest(dest_real);
    
    if (!src_real) {
        printf("Error: Invalid active backup task for this location.\n");
        printf("Błąd: Nie znaleziono aktywnego zadania backupu dla tej lokalizacji.\n");
        printf("Restoring available only for active sessions\n");
        return;
    }

    printf("Restoring...\nZ:  %s\nDo: %s\n", dest_real, src_real);
    sync_restore(src_real, dest_real, src_real, dest_real);
    printf("Restoring completed.\n");
}