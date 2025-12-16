#ifndef CORE_H
#define CORE_H

void init_core();
void handle_add(int argc, char **argv);
void handle_list();
void handle_end(int argc, char **argv);
void cleanup_all_processes();
char *get_source_by_dest(const char *dest);

#endif
