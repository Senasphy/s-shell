#ifndef PARSER_H
#define PARSER_H

typedef struct {
  char **argv;
  int argc;
  char *input_file;
  char *output_file;
  int append_output;
} command_t;

typedef struct {
  command_t *commands;
  int num_commands;
  int background;
} pipeline_t;

char *read_input(void);
pipeline_t *parse_pipeline(char *line);
void free_pipeline(pipeline_t *pipeline);

#endif
