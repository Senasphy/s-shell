#ifndef BUILTINS_H
#define BUILTINS_H

int is_builtin(const char *name);
int execute_builtin(char **args);
int builtin_count(void);

#endif
