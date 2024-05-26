//
// Created by malaklinux on 1/31/24.
//
#include <sys/stat.h>
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include <signal.h>
#include <pwd.h>
#define MAX_CMD_SIZE 1024
#define MAX_VARS 64
#define DOLLAR_SIGN_FOR_READ_COMMAND 1

#ifndef ADVANCED_SHELL_H
#define ADVANCED_SHELL_H

typedef struct {
    char *name;
    char *value;
} Variable;

char prompt[MAX_CMD_SIZE] = "Hello: ";
Variable variables[MAX_VARS];
char last_command[MAX_CMD_SIZE];
char *argv[MAX_CMD_SIZE];
int var_count = 0;
int status = 0;
int stdoutfd = 0;
pid_t pid = -1;

char *update_prompt(char **args);

void add_variable(char *name, char *value, int dollarSignForReadCommand);

char *find_variable(char *name);

void print_variable_or_value(char *arg);

int args_length(char **args);

void change_dir(char **args, struct passwd *pPasswd);

void handle_sigint();

void separate_commands(char *command);

char **find_pipe(char **args);

void handle_redirection(char **args, char **outfile, int size);

void handle_echo_command(char **args, int curr_status);

void handle_if_statement(char **argv);

void process_inside_statements(char *statement, int flag);

void read_input(char *input);

int execute(char **args);

int process(char **args);

int main();

#endif //ADVANCED_SHELL_H
