#include "builtins.h"
#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int cmd_cd(char **args);
static int cmd_help(char **args);
static int cmd_exit(char **args);
static int cmd_history(char **args);

static const char *builtin_names[] = {
  "cd",
  "help",
  "exit",
  "history"
};

static int (*builtin_actions[])(char **) = {
  &cmd_cd,
  &cmd_help,
  &cmd_exit,
  &cmd_history
};

int builtin_count(void) {
  return sizeof(builtin_names) / sizeof(char *);
}

static int cmd_cd(char **args) {
  const char *target = args[1];
  if (target == NULL) {
    target = getenv("HOME");
    if (target == NULL) {
      fprintf(stderr, "s-shell: cd: HOME not set\n");
      return 1;
    }
  }

  if (chdir(target) != 0) {
    perror("s-shell: cd");
  }
  return 1;
}

static int cmd_help(char **args) {
  (void)args;
  printf("s-shell - simple modular shell\n");
  printf("Built-in commands:\n");
  for (int i = 0; i < builtin_count(); i++) {
    printf("  %s\n", builtin_names[i]);
  }
  printf("Supports pipes (|), redirection (<, >, >>), and background execution (&).\n");
  return 1;
}

static int cmd_exit(char **args) {
  (void)args;
  return 0;
}

static int cmd_history(char **args) {
  (void)args;
  history_print();
  return 1;
}

int is_builtin(const char *name) {
  if (name == NULL) {
    return 0;
  }
  for (int i = 0; i < builtin_count(); i++) {
    if (strcmp(name, builtin_names[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

int execute_builtin(char **args) {
  if (args == NULL || args[0] == NULL) {
    return 1;
  }
  for (int i = 0; i < builtin_count(); i++) {
    if (strcmp(args[0], builtin_names[i]) == 0) {
      return (*builtin_actions[i])(args);
    }
  }
  return 1;
}
