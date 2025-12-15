#include <string.h>
#include "parser.h"
#include "types.h"

void parse_input(char *input, char **args, int *arg_count) {
  *arg_count = 0;
  char *ptr = input;

  // Usuń nową linię
  if (ptr[strlen(ptr)-1] == '\n') ptr[strlen(ptr)-1] = '\0';

  while (*ptr && *arg_count < MAX_ARGS) {
    while (*ptr == ' ') ptr++; // Pomiń spacje
    if (*ptr == '\0') break;

    if (*ptr == '"') {
      ptr++;
      args[*arg_count] = ptr;
      char *end = strchr(ptr, '"');
      if (end) {
        *end = '\0';
        ptr = end + 1;
      } else {
        ptr += strlen(ptr);
      }
    } else {
      args[*arg_count] = ptr;
      char *end = strchr(ptr, ' ');
      if (end) {
        *end = '\0';
        ptr = end + 1;
      } else {
        ptr += strlen(ptr);
      }
    }
    (*arg_count)++;
  }
}