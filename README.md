# s-shell

`s-shell` is an experimental Unix shell written in C, originally developed by following Stephen Brennan's [Write a Shell in C](https://brennan.io/2015/01/16/write-a-shell-in-c/) guide and expanded into a modular implementation.

## Features

- Built-in commands (`cd`, `help`, `history`, `exit`)
- Dynamic prompt displaying current working directory
- Process execution via `fork` and `execvp`
- Input and output redirection (`<`, `>`, `>>`)
- Multi-command pipelines (`|`)
- Background job execution (`&`)
- Signal handling for `SIGINT` (Ctrl+C) and `SIGCHLD` (zombie reaping)

## Build

```bash
make
./s-shell
```

To clean build artifacts:

```bash
make clean
```
