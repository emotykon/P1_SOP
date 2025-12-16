#include <signal.h>
#include <stdlib.h>
#include "signals.h"
#include "core.h"

void handle_sigint(int sig) {
  cleanup_all_processes();
  exit(0);
}
void setup_signals() {
  signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigint);
}