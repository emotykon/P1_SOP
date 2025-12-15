#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "core.h"
#include "restore.h"
#include "signals.h"
#include "types.h"

int main() {
  setup_signals();
  init_core();

  char input[1024];
  char *args[MAX_ARGS];
  int argc;

  printf("---sop-backup---\n");
  printf("Dostepne komendy: add, list, end, restore, exit\n");

  while (1) {
    printf("> ");
    if (!fgets(input, sizeof(input), stdin)) break;

    parse_input(input, args, &argc);
    if (argc == 0) continue;

    if (strcmp(args[0], "exit") == 0) {
      cleanup_all_processes();
      break;
    }
    else if (strcmp(args[0], "add") == 0) {
      handle_add(argc, args);
    }
    else if (strcmp(args[0], "list") == 0) {
      handle_list();
    }
    else if (strcmp(args[0], "end") == 0) {
      handle_end(argc, args);
    }
    else if (strcmp(args[0], "restore") == 0) {
      if (argc < 2) {
        printf("Błąd: Podaj ścieżkę kopii do przywrócenia.\n");
      } else {
        handle_restore(args[1]);
      }
    }
    else {
      printf("Nie ma takiej komendy.\n");
    }
  }

  return 0;
}
