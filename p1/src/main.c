#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
static void print_menu(void) {
  printf("Dostępne komendy:\n");
  printf("  add <source> <target...>\n");
  printf("  end <source> <target...>\n");
  printf("  list\n");
  printf("  restore <source> <target>\n");
  printf("  exit\n");
}
static volatile sig_atomic_t running = 1;
static void handle_signal(int sig) {
  (void)sig;
  running = 0;
}
int main() {

  print_menu();
  while (running) {
    printf("> ");
    fflush(stdout);

    ssize_t n = getline(&line, &len, stdin);

    /* EOF (Ctrl+D) */
    if (n == -1) {
      break;
    }

    /* usuń '\n' */
    if (n > 0 && line[n - 1] == '\n') {
      line[n - 1] = '\0';
    }

    /* exit kończy program */
    if (strcmp(line, "exit") == 0) {
      break;
    }

    /* na razie: tylko echo */
    printf("Wpisano: \"%s\"\n", line);
  }
  free(line);
  printf("Koniec programu.\n");
return 0;
}
