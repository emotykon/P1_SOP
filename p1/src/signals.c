#include <signal.h>
#include <stdlib.h>
#include "signals.h"
#include "core.h"

void handle_sigint(int sig) {
  cleanup_all_processes();
  exit(0);
}

void setup_signals() {
  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}