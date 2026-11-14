#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 1024

char *read_input(void) {
  int cap = CHUNK_SIZE;
  int idx = 0;
  char *str = malloc(cap);
  int ch;

  if (!str) {
    fprintf(stderr, "s-shell: memory allocation failure\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    ch = getchar();

    if (ch == EOF) {
      if (idx == 0) {
        free(str);
        return NULL;
      }
      str[idx] = '\0';
      return str;
    } else if (ch == '\n') {
      str[idx] = '\0';
      return str;
    } else {
      str[idx] = (char)ch;
    }
    idx++;

    if (idx >= cap) {
      cap += CHUNK_SIZE;
      char *new_str = realloc(str, cap);
      if (!new_str) {
        free(str);
        fprintf(stderr, "s-shell: memory reallocation failure\n");
        exit(EXIT_FAILURE);
      }
      str = new_str;
    }
  }
}

static char **tokenize_line(const char *line, int *out_count) {
  int cap = 32;
  int count = 0;
  char **tokens = malloc(cap * sizeof(char *));
  const char *p = line;

  if (!tokens) {
    return NULL;
  }

  while (*p != '\0') {
    while (*p != '\0' && isspace((unsigned char)*p)) {
      p++;
    }
    if (*p == '\0') {
      break;
    }

    const char *start;
    size_t len = 0;

    if (*p == '>' && *(p + 1) == '>') {
      start = p;
      len = 2;
      p += 2;
    } else if (*p == '<' || *p == '>' || *p == '|' || *p == '&') {
      start = p;
      len = 1;
      p++;
    } else if (*p == '"' || *p == '\'') {
      char quote = *p++;
      start = p;
      while (*p != '\0' && *p != quote) {
        p++;
      }
      len = p - start;
      if (*p == quote) {
        p++;
      }
    } else {
      start = p;
      while (*p != '\0' && !isspace((unsigned char)*p) && *p != '<' &&
             *p != '>' && *p != '|' && *p != '&') {
        p++;
      }
      len = p - start;
    }

    if (count >= cap) {
      cap *= 2;
      tokens = realloc(tokens, cap * sizeof(char *));
      if (!tokens) {
        return NULL;
      }
    }

    tokens[count] = malloc(len + 1);
    if (!tokens[count]) {
      return NULL;
    }
    memcpy(tokens[count], start, len);
    tokens[count][len] = '\0';
    count++;
  }

  *out_count = count;
  return tokens;
}

pipeline_t *parse_pipeline(char *line) {
  if (line == NULL) {
    return NULL;
  }

  int token_count = 0;
  char **tokens = tokenize_line(line, &token_count);
  if (tokens == NULL || token_count == 0) {
    free(tokens);
    return NULL;
  }

  pipeline_t *pipeline = malloc(sizeof(pipeline_t));
  if (!pipeline) {
    for (int i = 0; i < token_count; i++) {
      free(tokens[i]);
    }
    free(tokens);
    return NULL;
  }

  pipeline->background = 0;
  if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
    pipeline->background = 1;
    free(tokens[token_count - 1]);
    token_count--;
  }

  int cmd_count = 1;
  for (int i = 0; i < token_count; i++) {
    if (strcmp(tokens[i], "|") == 0) {
      cmd_count++;
    }
  }

  pipeline->num_commands = cmd_count;
  pipeline->commands = calloc(cmd_count, sizeof(command_t));
  if (!pipeline->commands) {
    free(pipeline);
    for (int i = 0; i < token_count; i++) {
      free(tokens[i]);
    }
    free(tokens);
    return NULL;
  }

  int cur_cmd = 0;
  int argv_cap = 8;
  pipeline->commands[cur_cmd].argv = malloc(argv_cap * sizeof(char *));
  pipeline->commands[cur_cmd].argc = 0;

  for (int i = 0; i < token_count; i++) {
    if (strcmp(tokens[i], "|") == 0) {
      pipeline->commands[cur_cmd].argv[pipeline->commands[cur_cmd].argc] = NULL;
      free(tokens[i]);

      cur_cmd++;
      argv_cap = 8;
      pipeline->commands[cur_cmd].argv = malloc(argv_cap * sizeof(char *));
      pipeline->commands[cur_cmd].argc = 0;
    } else if (strcmp(tokens[i], "<") == 0) {
      free(tokens[i]);
      if (i + 1 < token_count) {
        i++;
        pipeline->commands[cur_cmd].input_file = tokens[i];
      }
    } else if (strcmp(tokens[i], ">") == 0) {
      free(tokens[i]);
      if (i + 1 < token_count) {
        i++;
        pipeline->commands[cur_cmd].output_file = tokens[i];
        pipeline->commands[cur_cmd].append_output = 0;
      }
    } else if (strcmp(tokens[i], ">>") == 0) {
      free(tokens[i]);
      if (i + 1 < token_count) {
        i++;
        pipeline->commands[cur_cmd].output_file = tokens[i];
        pipeline->commands[cur_cmd].append_output = 1;
      }
    } else {
      command_t *cmd = &pipeline->commands[cur_cmd];
      if (cmd->argc + 1 >= argv_cap) {
        argv_cap *= 2;
        cmd->argv = realloc(cmd->argv, argv_cap * sizeof(char *));
      }
      cmd->argv[cmd->argc] = tokens[i];
      cmd->argc++;
    }
  }

  pipeline->commands[cur_cmd].argv[pipeline->commands[cur_cmd].argc] = NULL;
  free(tokens);

  return pipeline;
}

void free_pipeline(pipeline_t *pipeline) {
  if (pipeline == NULL) {
    return;
  }

  for (int i = 0; i < pipeline->num_commands; i++) {
    command_t *cmd = &pipeline->commands[i];
    if (cmd->argv) {
      for (int j = 0; j < cmd->argc; j++) {
        free(cmd->argv[j]);
      }
      free(cmd->argv);
    }
    if (cmd->input_file) {
      free(cmd->input_file);
    }
    if (cmd->output_file) {
      free(cmd->output_file);
    }
  }

  free(pipeline->commands);
  free(pipeline);
}
