#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include "shell.h"

#define MAX_CMD_SIZE 1024
#define MAX_ARG_SIZE 100
//#define MAX_VARS 100
#define HISTORY_SIZE 10

// typedef struct {
//     char *name;
//     char *value;
// } Variable;

// Variable variables[MAX_VARS];
// int var_count = 0;
// char prompt[MAX_CMD_SIZE] = "shell> ";
// char last_command[MAX_CMD_SIZE];
// char *argv[MAX_ARG_SIZE];
// int status;
// pid_t pid = -1;

char *history[HISTORY_SIZE];
int history_count = 0;
int current_history_index = -1;





// Function to enable raw mode for input
void enableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to disable raw mode
void disableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to read a line of input with history navigation
char *read_line(const char *prompt) {
    char *buffer = malloc(MAX_CMD_SIZE);  // Allocate memory for the input buffer
    int position = 0;
    char c;

    enableRawMode();

    printf("%s", prompt);  // Print the prompt
    fflush(stdout);

    while (1) {
        c = getchar();

        if (c == '\n') {
            buffer[position] = '\0';
            printf("\n");
            break;
        } else if (c == 27) {  // Escape sequence
            getchar(); // Skip the [
            switch(getchar()) { // Read the next character
                case 'A': // Up arrow
                    if (history_count > 0) {
                        if (current_history_index == -1) {
                            current_history_index = history_count - 1;
                        } else if (current_history_index > 0) {
                            current_history_index--;
                        }
                        if (history[current_history_index]) {
                            printf("\r\033[K%s%s", prompt, history[current_history_index]);
                            strcpy(buffer, history[current_history_index]);
                            position = strlen(buffer);
                        }
                    }
                    break;
                case 'B': // Down arrow
                    if (history_count > 0) {
                        if (current_history_index != -1) {
                            if (current_history_index < history_count - 1) {
                                current_history_index++;
                            } else {
                                current_history_index = -1;
                                buffer[0] = '\0';
                                position = 0;
                            }
                            if (current_history_index != -1 && history[current_history_index]) {
                                printf("\r\033[K%s%s", prompt, history[current_history_index]);
                                strcpy(buffer, history[current_history_index]);
                                position = strlen(buffer);
                            }
                        }
                    }
                    break;
            }
        } else if (c == 127) {  // Backspace
            if (position > 0) {
                buffer[--position] = '\0';
                printf("\b \b");
            }
        } else {
            buffer[position++] = c;
            printf("%c", c);
        }
    }

    disableRawMode();
    return buffer;  // Return the dynamically allocated input buffer
}


// Function to add a command to history
void add_to_history(char *cmd) {
    char *cmd_copy = strdup(cmd); // Duplicate the command string
    if (cmd_copy == NULL) {
        // Handle memory allocation failure
        perror("Error duplicating command");
        return;
    }
    if (history[history_count % HISTORY_SIZE]) {
        free(history[history_count % HISTORY_SIZE]);  // Free old command if present
    }
    history[history_count % HISTORY_SIZE] = cmd_copy;
    history_count++;
    if (history_count > HISTORY_SIZE) {
        history_count = HISTORY_SIZE;
    }
}


// Function to change the prompt
char *update_prompt(char **args) {
    strcpy(prompt, "");
    int len = args_length(args);
    int i = 2;
    while (i < len) {
        strcat(prompt, args[i]);
        strcat(prompt, " ");
        ++i;
    }
    return prompt;
}

// Function to add or update a variable
void add_variable(char *name, char *value, int dollarSignForReadCommand) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            free(variables[i].value);
            variables[i].value = strdup(value);
            return;
        }
    }
    if (var_count < MAX_VARS) {
        if (dollarSignForReadCommand) {
            variables[var_count].name = malloc(strlen(name) + 2);
            sprintf(variables[var_count].name, "$%s", name);
            variables[var_count].value = malloc(MAX_CMD_SIZE);
            fgets(variables[var_count].value, MAX_CMD_SIZE, stdin);
            variables[var_count].value[strlen(variables[var_count].value) - 1] = '\0';
        } else {
            variables[var_count].name = strdup(name);
            variables[var_count].value = strdup(value);
        }
        var_count++;
    } else {
        printf("Too many variables. Cannot add more.\n");
    }
}

// Function to find a variable by name
char *find_variable(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return NULL;
}

// Function to print variable or value
void print_variable_or_value(char *arg) {
    if (arg && arg[0] == '$') {
        char *value = find_variable(arg);
        if (value != NULL)
            printf("%s ", value);
    } else {
        printf("%s ", arg);
    }
}

// Function to return length of args
int args_length(char **args) {
    char **ptr = args;
    int len = 0;
    while (*ptr != NULL) {
        ptr++;
        len++;
    }
    return len;
}

// Function to change directory
void change_dir(char **args, struct passwd *pPasswd) {
    char *path = args[1];
    if (!path || strcmp(path, "~") == 0) {
        path = pPasswd->pw_dir;
    }
    if (chdir(path) != 0) {
        perror("Error changing directory");
    }
}

// Function to handle CTRL+C
void handle_sigint() {
    if (pid != -1) {
        kill(pid, SIGKILL);
        printf("\nYou typed Control-C! [Process Killed]\n");
    } else {
        printf("\nYou typed Control-C!\n");
    }
    printf("%s", prompt);
    fflush(stdout);
}

// Function to separate commands
void separate_commands(char *command) {
    char *token = NULL;
    int i = 0;
    for (token = strtok(command, " "); token != NULL; token = strtok(NULL, " ")) {
        argv[i++] = token;
    }
    argv[i] = NULL;
}

// Function to find pipe command
char **find_pipe(char **args) {
    char **ptr = args;
    while (*ptr != NULL && strcmp(*ptr, "|") != 0) {
        ptr++;
    }
    if (*ptr != NULL) {
        return ptr;
    }
    return NULL;
}

// Function to handle redirection
void handle_redirection(char **args, char **outfile, int size) {
    int redirectFd = -1;

    if (size >= 2 && (!strcmp(args[size - 2], ">") || !strcmp(args[size - 2], ">>"))) {
        *outfile = args[size - 1];
        redirectFd = STDOUT_FILENO;
    } else if (size >= 2 && !strcmp(args[size - 2], "<")) {
        *outfile = args[size - 1];
        redirectFd = STDIN_FILENO;
    } else if (size >= 2 && !strcmp(args[size - 2], "2>")) {
        *outfile = args[size - 1];
        redirectFd = STDERR_FILENO;
    }
    if (redirectFd >= 0) {
        int fd;
        if (!strcmp(args[size - 2], ">>")) {
            fd = open(*outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
            lseek(fd, 0, SEEK_END);
            if (fd < 0) {
                perror("Error opening file for appending");
            }
        } else if (!strcmp(args[size - 2], ">") || !strcmp(args[size - 2], "2>")) {
            fd = open(*outfile, O_WRONLY | O_CREAT | O_TRUNC, 0660);
            if (fd < 0) {
                perror("Error opening file for writing");
            }
        } else {
            fd = open(*outfile, O_RDONLY);
            if (fd < 0) {
                perror("Error opening file for reading");
            }
        }
        close(redirectFd);
        dup(fd);
        close(fd);
        args[size - 2] = NULL;
    }
}

// Function to handle echo command
void handle_echo_command(char **args, int curr_status) {
    char **echo_args = args + 1;

    if (!strcmp(*echo_args, "$?")) {
        printf("%d\n", curr_status);
        return;
    }

    while (*echo_args) {
        print_variable_or_value(*echo_args);
        echo_args++;
    }
    printf("\n");
}

// Function to handle if statement
void handle_if_statement(char **args) {
    char statement[MAX_CMD_SIZE];
    int curr_status = execute(args);

    if (!curr_status) {
        read_input(statement);
        if (!strcmp(statement, "then")) {
            read_input(statement);
            process_inside_statements(statement, 1);
        } else {
            printf("Syntax Error: Bad If statement\n");
            return;
        }
    } else {
        read_input(statement);
        while (strcmp(statement, "else") != 0) {
            read_input(statement);
        }
        process_inside_statements(statement, 2);
    }
    return;
}

//Function to process inside if statements
void process_inside_statements(char *statement, int flag) {
    int elseFlag = 1;
    while (strcmp(statement, "fi") != 0) {
        if (flag == 1 && elseFlag && strcmp(statement, "else") == 0) {
            elseFlag = 0;
        }
        if (elseFlag) {
            separate_commands(statement);
            process(argv);
        }
        read_input(statement);
    }
}
// Function to read input for if else
void read_input(char *input) {
    fgets(input, MAX_CMD_SIZE, stdin);
    input[strlen(input) - 1] = '\0';
}

// Main execution command
int execute(char **args) {
    struct passwd *pw = getpwuid(getuid());
    char *outfile = NULL;
    int len = args_length(args);
    int ampersand;
    int curr_status = -1;
    int hasPip = 0;

    char **pipPointer = find_pipe(args);
    int pipe_fd[2];

    if (pipPointer != NULL) {
        hasPip = 1;
        *pipPointer = NULL;
        len = args_length(args);
        pipe(pipe_fd);

        if (fork() == 0) {
            close(pipe_fd[1]);
            close(STDIN_FILENO);
            dup(pipe_fd[0]);
            execute(pipPointer + 1);
            exit(0);
        }

        stdoutfd = dup(STDOUT_FILENO);
        dup2(pipe_fd[1], STDOUT_FILENO);
    }

    if (args[0] == NULL)
        return 0;
    if (!strcmp(args[0], "!!")) {
        separate_commands(last_command);
        execute(argv);
        return 0;
    }
    if (args[0][0] == '$' && len >= 3) {
        add_variable(args[0], args[2], 0);
        return 0;
    }

    if (!strncmp(args[0], "if", 2)) {
        int j = 1;
        while (argv[j] != NULL) {
            argv[j - 1] = argv[j];
            j++;
        }
        argv[j - 1] = NULL;
        handle_if_statement(args);
        return 0;
    }

    if (!strcmp(args[0], "read")) {
        add_variable(args[1], NULL, 1);
        return 0;
    }
    if (!strcmp(args[0], "cd")) {
        change_dir(args, pw);
        return 0;
    }
    if (!strcmp(args[0], "prompt")) {
        update_prompt(args);
        return 0;
    }
    if (!strcmp(args[0], "echo")) {
        handle_echo_command(args, status);
        return 0;
    }
    if (!strcmp(args[len - 1], "&")) {
        ampersand = 1;
        args[len - 1] = NULL;
    } else {
        ampersand = 0;
    }
    if ((pid = fork()) == 0) {
        handle_redirection(args, &outfile, len);
        execvp(args[0], args);
    }
    if (ampersand == 0) {
        wait(&status);
        curr_status = status;
        pid = -1;
    }
    if (hasPip) {
        close(STDOUT_FILENO);
        close(pipe_fd[1]);
        dup(stdoutfd);
        wait(NULL);
    }
    return curr_status;
}

// Function to execute shell
int process(char **args) {
    int curr_status;
    if (args[0] == NULL) { curr_status = 0; }
    else { curr_status = execute(args); }
    return curr_status;
}

int main() {
    signal(SIGINT, handle_sigint);
    char command[MAX_CMD_SIZE];

    while (1) {
        //printf("%s", prompt);
        fflush(stdout);

        char *input = read_line(prompt);
        if (input == NULL) {
            continue;
        }

        if (strcmp(input, "quit") == 0) {
            free(input);
            exit(0);
        }

        if (strcmp(input, "!!") != 0) {
            strcpy(last_command, input);
        }

        add_to_history(input);

        separate_commands(input);
        status = process(argv);

        free(input);
    }
}
