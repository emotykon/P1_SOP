#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include "monitor.h"
#include "utils.h"
#include "types.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static WatchMap *head_wd = NULL;

void add_watch_recursive(int fd, const char *path) {
    int wd = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE);
    if (wd == -1) return;

    WatchMap *wm = malloc(sizeof(WatchMap));
    wm->wd = wd;
    strncpy(wm->path, path, PATH_MAX);
    wm->next = head_wd;
    head_wd = wm;

    DIR *d = opendir(path);
    if (!d) return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            char subpath[PATH_MAX];
            snprintf(subpath, sizeof(subpath), "%s/%s", path, dir->d_name);
            add_watch_recursive(fd, subpath);
        }
    }
    closedir(d);
}

char* get_path_by_wd(int wd) {
    WatchMap *curr = head_wd;
    while (curr) {
        if (curr->wd == wd) return curr->path;
        curr = curr->next;
    }
    return NULL;
}

void run_backup_monitor(const char *src_root, const char *dst_root) {
    // Resetuj sygnał w dziecku, aby nie reagowało na Ctrl+C terminala w ten sam sposób co rodzic
    signal(SIGINT, SIG_IGN); 

    // 1. Kopia początkowa
    copy_recursive(src_root, dst_root, src_root, dst_root, 0);

    // 2. Init Inotify
    int fd = inotify_init();
    if (fd < 0) exit(1);

    add_watch_recursive(fd, src_root);

    char buf[BUF_LEN];
    while (1) {
        ssize_t len = read(fd, buf, BUF_LEN);
        if (len <= 0) break;

        const char *ptr = buf;
        while (ptr < buf + len) {
            struct inotify_event *event = (struct inotify_event *)ptr;
            if (event->len) {
                char *src_dir = get_path_by_wd(event->wd);
                if (src_dir) {
                    char full_src[PATH_MAX], full_dst[PATH_MAX];
                    snprintf(full_src, sizeof(full_src), "%s/%s", src_dir, event->name);

                    // Oblicz ścieżkę relatywną
                    char relative[PATH_MAX];
                    if (strlen(src_dir) == strlen(src_root)) {
                         snprintf(relative, sizeof(relative), "/%s", event->name);
                    } else {
                         snprintf(relative, sizeof(relative), "%s/%s", src_dir + strlen(src_root), event->name);
                    }
                    snprintf(full_dst, sizeof(full_dst), "%s%s", dst_root, relative);

                    if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO) {
                        struct stat st;
                        if (lstat(full_src, &st) != -1) {
                            if (S_ISDIR(st.st_mode)) {
                                mkdir(full_dst, st.st_mode);
                                add_watch_recursive(fd, full_src);
                                copy_recursive(full_src, full_dst, src_root, dst_root, 0);
                            } else {
                                copy_recursive(full_src, full_dst, src_root, dst_root, 0);
                            }
                        }
                    } else if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MODIFY) {
                        struct stat st;
                        if (lstat(full_src, &st) != -1 && !S_ISDIR(st.st_mode)) {
                            copy_recursive(full_src, full_dst, src_root, dst_root, 1);
                        }
                    } else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM) {
                        delete_recursive(full_dst);
                    }
                }
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
    close(fd);
    exit(0);
}