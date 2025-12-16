#define _DEFAULT_SOURCE
#include "core.h"
#include "parser.h"
#include "signals.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  setup_signals();
  init_core();

  int argc;

  printf("---sop-backup---\n");
  printf("Available commands: add, list, end, exit, NO restore\n");

  while (1) {
    char input[1024];
    char *args[MAX_ARGS];
    printf("> ");
    if (!fgets(input, sizeof(input), stdin))
      break;

    parse_input(input, args, &argc);
    if (argc == 0)
      continue;

    if (strcmp(args[0], "exit") == 0) {
      cleanup_all_processes();
      break;
    } else if (strcmp(args[0], "add") == 0) {
      handle_add(argc, args);
    } else if (strcmp(args[0], "list") == 0) {
      handle_list();
    } else if (strcmp(args[0], "end") == 0) {
      handle_end(argc, args);
    } else {
      printf("Unknown command.\n");
    }
  }

  return 0;
}
