#include "builtins.h"
#include "executor.h"
#include "history.h"
#include "parser.h"
#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_prompt(void) {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    const char *home = getenv("HOME");
    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
      printf("s-shell:~%s$ ", cwd + strlen(home));
    } else {
      printf("s-shell:%s$ ", cwd);
    }
  } else {
    printf("s-shell$ ");
  }
  fflush(stdout);
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  signals_init();
  history_init();

  int running = 1;
  while (running) {
    print_prompt();

    char *line = read_input();
    if (line == NULL) {
      printf("\n");
      break;
    }

    const char *p = line;
    while (*p == ' ' || *p == '\t' || *p == '\n') {
      p++;
    }

    if (*p != '\0') {
      history_add(line);
      pipeline_t *pipeline = parse_pipeline(line);
      if (pipeline) {
        running = execute_pipeline(pipeline);
        free_pipeline(pipeline);
      }
    }

    free(line);
  }

  history_cleanup();
  return EXIT_SUCCESS;
}
