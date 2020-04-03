#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>   // Contains: waitpid
#include <unistd.h>     // Contains: chdir, fork, exec, pid_t
#include <stdlib.h>     // Contains: malloc, realloc, free, exit, execvp, EXIT_SUCCESS, EXIT_FAILURE

// Defining Macros
#define INITIAL_LINE_SIZE 1000
#define INITIAL_ARGS_SIZE 100
#define INITIAL_ARG_SIZE 256

// Constants
const char *SHELL_NAME = "shell";

int cd_command(char **args);

int exit_command();

// List of commands
char *commands_list[] = {
        "cd",
        "exit"
};

// List of commands' functions
int (*commands_functions[])(char **) = {
        &cd_command,
        &exit_command
};

char *get_user_input(char *line);

char **parse_user_input(char *line, char **args);

int execute_commands(char **args);

void run();

void init(char **line, char **args);

void removeNewLine(char *line);

void free_memory(char *line, char **args);

int check_args(char **args);

int get_commands_num();

/**
 *
 * @return shell exit status
 */
int main() {

    // Run the program
    run();

    return EXIT_SUCCESS;
}

void run() {

    char *line = NULL;
    char **args = NULL;
    int isRunning;


    do {
        // Take user input
        line = get_user_input(line);

        // Parse user input into arguments
        args = parse_user_input(line, args);

        // Execute user commands
        isRunning = check_args(args);

        // Free allocated memory
        free_memory(line, args);

    } while (isRunning);

}

int check_args(char **args) {

    // Ignore command
    if (args[0] == NULL)
        return 1;

    int num_commands = get_commands_num();
    for (int i = 0; i < num_commands; ++i) {
        if (strcmp(args[0], commands_list[i]) == 0)
            return (*commands_functions[i])(args);
    }

    return execute_commands(args);
}

int get_commands_num() {
    return sizeof(commands_list) / sizeof(char *);
}

void free_memory(char *line, char **args) {
    free(line);
    for (int i = 0; args[i] != NULL; ++i) {
        free(args[i]);
    }
    free(args);
}

char *get_user_input(char *line) {
    // Allocating initial space for the line of arguments
    line = malloc(INITIAL_LINE_SIZE * sizeof(char));

    printf("> ");
    fgets(line, INITIAL_LINE_SIZE, stdin);
    removeNewLine(line);

    return line;
}

void removeNewLine(char *line) {
    char *newLine;

    newLine = strrchr(line, '\n');

    if (newLine)
        *newLine = '\0';
}

char **parse_user_input(char *line, char **args) {

    // Allocating initial space for the arguments
    args = malloc(INITIAL_ARGS_SIZE * sizeof(char *));
    for (int i = 0; i < INITIAL_ARGS_SIZE; ++i) {
        args[i] = malloc(INITIAL_ARG_SIZE * sizeof(char));
    }

    // Track characters of each argument
    int argCharCount = 0;

    // Track arguments count
    int argsCount = 0;

    // Track line characters' count
    int lineTotalChars = 0;

    // Loop through the line until a string terminator '\0' is found
    for (int i = 0; line[i] != '\0'; i++, lineTotalChars++) {

        // If a space is found, insert a string terminator at the end of the argument
        if (line[i] == ' ') {

            // Reallocating the amount of memory for each argument
            args[argsCount] = realloc(args[argsCount], argCharCount * sizeof(char));

            // Assigning string terminator at the end of each argument
            args[argsCount++][argCharCount] = '\0';

            // Resetting counter for argument characters
            argCharCount = 0;

        } else {

            // Assigning the the line character to the argument
            args[argsCount][argCharCount++] = line[i];
        }
    }

    // Handling empty line case
    if (lineTotalChars == 0)
        args[argsCount] = NULL;
    else
        args[++argsCount] = NULL;

    // Reallocating the exact amount of memory for the arguments
    args = realloc(args, argsCount * sizeof(char *) + 1);

    // Reallocating the exact amount of memory for line array
    line = realloc(line, lineTotalChars * sizeof(char) + 1);

    return args;
}

int execute_commands(char **args) {
    pid_t pid;
    pid = fork();
    int status;

    if (pid < 0) {
        // Error happened on forking
        perror(SHELL_NAME);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {

        // Successful child process creation
        // Child process

        printf("Child Process Created: %u\n", getpid());

        if (execvp(args[0], args) < 0) {
            // Error Happened when executing command
            perror(SHELL_NAME);
        }

        exit(EXIT_FAILURE);

    } else {
        // Parent process
        printf("Parent Process: %u\n", getppid());

        if (waitpid(pid, &status, 0) > 0) {

             if (WIFEXITED(status) && !WEXITSTATUS(status))
                 printf("Program Execution Successful\n");

             else if (WIFEXITED(status) && WEXITSTATUS(status)) {

                 if (WEXITSTATUS(status) == 127)
                     // Execution failed
                     printf("Execution Failed");
                 else
                     printf("Program terminated normally, but returned a non-zero status\n");

             } else
                 printf("Program didn't terminate normally\n");

        } else
            // waitpid failed
            printf("Failed waiting for child process\n");

        return 1;
    }

}

int exit_command() {
    return EXIT_SUCCESS;
}

int cd_command(char **args) {

    if (args[1] == NULL)
        printf("cd: argument is expected");
    else {

        if (chdir(args[1]) != 0)
            perror(SHELL_NAME);

    }

    // To resume executing commands
    return 1;
}
