# Advanced Programming Course Assignment 1

## Overview
This project implements a custom shell in C that supports various basic shell functionalities, including command execution, history navigation, variable management, directory changes, and simple conditional statements. The shell has been implemented in two primary files:
- `shell.c`: Contains the core logic and functions for the shell.
- `shell.h`: Header file declaring functions and structures used in `shell.c`.

Additionally, a `makefile` is provided to facilitate the building of the project.

## Files

### shell.c
This file contains the main implementation of the shell, including:

- **History Management**:
  - Commands are stored in a circular buffer for history tracking.
  - Navigation through history using arrow keys.

- **Raw Mode Input**:
  - Enables raw mode to handle character input directly, including handling backspaces and arrow keys for history navigation.

- **Command Execution**:
  - Executes commands using `execvp`.
  - Supports background execution with `&`.
  - Handles input/output redirection.

- **Variable Management**:
  - Supports setting and retrieving variables, including reading user input directly into variables.

- **Built-in Commands**:
  - `cd`: Change the current directory.
  - `echo`: Print arguments to the terminal.
  - `prompt`: Change the shell prompt.
  - `read`: Read input from the user and store it in a variable.
  - `if`: Simple conditional execution based on the exit status of commands.

- **Signal Handling**:
  - Custom handling for `CTRL+C` to terminate the current process or display a message.

### shell.h
This header file declares the constants, structures, and function prototypes used in `shell.c`. Key elements include:

- **Constants**:
  - `MAX_CMD_SIZE`, `MAX_VARS`, etc., to define buffer sizes and limits.

- **Structures**:
  - `Variable` structure for storing shell variables.

- **Function Prototypes**:
  - Functions for command execution, history management, variable management, and built-in commands.

### makefile
The makefile automates the build process. Key targets include:

- `all`: Default target that builds the `myshell` executable.
- `clean`: Removes all object files and the `myshell` executable.

## Compilation
To compile the shell, run:
```sh
make

./myshell

