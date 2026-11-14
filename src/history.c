#define _POSIX_C_SOURCE 200809L
#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 100

static char *history_entries[MAX_HISTORY];
static int history_count = 0;

void history_init(void) {
  for (int i = 0; i < MAX_HISTORY; i++) {
    history_entries[i] = NULL;
  }
  history_count = 0;
}

void history_add(const char *entry) {
  if (entry == NULL || *entry == '\0') {
    return;
  }

  if (history_count < MAX_HISTORY) {
    history_entries[history_count] = strdup(entry);
    history_count++;
  } else {
    free(history_entries[0]);
    for (int i = 1; i < MAX_HISTORY; i++) {
      history_entries[i - 1] = history_entries[i];
    }
    history_entries[MAX_HISTORY - 1] = strdup(entry);
  }
}

void history_print(void) {
  for (int i = 0; i < history_count; i++) {
    if (history_entries[i]) {
      printf("%4d  %s\n", i + 1, history_entries[i]);
    }
  }
}

void history_cleanup(void) {
  for (int i = 0; i < history_count; i++) {
    if (history_entries[i]) {
      free(history_entries[i]);
      history_entries[i] = NULL;
    }
  }
  history_count = 0;
}
