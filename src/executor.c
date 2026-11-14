#include "executor.h"
#include "builtins.h"
#include "signals.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int execute_single(command_t *cmd, int background) {
  if (cmd->argv == NULL || cmd->argv[0] == NULL) {
    return 1;
  }

  if (is_builtin(cmd->argv[0]) && !background && cmd->input_file == NULL &&
      cmd->output_file == NULL) {
    return execute_builtin(cmd->argv);
  }

  pid_t pid = fork();
  if (pid == 0) {
    signals_reset_child();

    if (cmd->input_file) {
      int fd = open(cmd->input_file, O_RDONLY);
      if (fd < 0) {
        perror("s-shell: input file");
        exit(EXIT_FAILURE);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
    }

    if (cmd->output_file) {
      int flags =
          O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
      int fd = open(cmd->output_file, flags, 0644);
      if (fd < 0) {
        perror("s-shell: output file");
        exit(EXIT_FAILURE);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }

    if (is_builtin(cmd->argv[0])) {
      int ret = execute_builtin(cmd->argv);
      exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    execvp(cmd->argv[0], cmd->argv);
    perror("s-shell");
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("s-shell: fork");
    return 1;
  }

  if (background) {
    printf("[1] %d\n", pid);
    return 1;
  }

  int status;
  do {
    waitpid(pid, &status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

  return 1;
}

static int execute_piped(pipeline_t *pipeline) {
  int n_cmds = pipeline->num_commands;
  int n_pipes = n_cmds - 1;
  int pipefds[n_pipes][2];

  for (int i = 0; i < n_pipes; i++) {
    if (pipe(pipefds[i]) < 0) {
      perror("s-shell: pipe");
      return 1;
    }
  }

  pid_t *pids = malloc(n_cmds * sizeof(pid_t));
  if (!pids) {
    perror("s-shell: memory allocation error");
    return 1;
  }

  for (int i = 0; i < n_cmds; i++) {
    command_t *cmd = &pipeline->commands[i];
    pids[i] = fork();

    if (pids[i] == 0) {
      signals_reset_child();

      if (i > 0) {
        dup2(pipefds[i - 1][0], STDIN_FILENO);
      }
      if (i < n_cmds - 1) {
        dup2(pipefds[i][1], STDOUT_FILENO);
      }

      for (int j = 0; j < n_pipes; j++) {
        close(pipefds[j][0]);
        close(pipefds[j][1]);
      }

      if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
          perror("s-shell: input file");
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
      }

      if (cmd->output_file) {
        int flags =
            O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int fd = open(cmd->output_file, flags, 0644);
        if (fd < 0) {
          perror("s-shell: output file");
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }

      if (is_builtin(cmd->argv[0])) {
        int ret = execute_builtin(cmd->argv);
        exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
      }

      execvp(cmd->argv[0], cmd->argv);
      perror("s-shell");
      exit(EXIT_FAILURE);
    } else if (pids[i] < 0) {
      perror("s-shell: fork");
    }
  }

  for (int j = 0; j < n_pipes; j++) {
    close(pipefds[j][0]);
    close(pipefds[j][1]);
  }

  if (pipeline->background) {
    printf("[1] %d\n", pids[n_cmds - 1]);
  } else {
    for (int i = 0; i < n_cmds; i++) {
      if (pids[i] > 0) {
        int status;
        waitpid(pids[i], &status, WUNTRACED);
      }
    }
  }

  free(pids);
  return 1;
}

int execute_pipeline(pipeline_t *pipeline) {
  if (pipeline == NULL || pipeline->num_commands == 0) {
    return 1;
  }

  if (pipeline->num_commands == 1) {
    return execute_single(&pipeline->commands[0], pipeline->background);
  }

  return execute_piped(pipeline);
}
