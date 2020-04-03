#include <stdio.h>
#include <string.h>     // Contains: Useful string functions
#include <sys/wait.h>   // Contains: wait functions
#include <unistd.h>     // Contains: chdir, fork, exec, pid_t
#include <stdlib.h>     // Contains: malloc, realloc, free, exit, execvp, EXIT_SUCCESS, EXIT_FAILURE
#include<signal.h>      // Contains: SIGCHLD signal

/**
 * Defining Macros
 */
#define INITIAL_LINE_SIZE 1000
#define INITIAL_ARGS_SIZE 100
#define INITIAL_ARG_SIZE 256

/**
 * Defining Constants
 */
const char *SHELL_NAME = "shell";

// Logs file name
const char *LOGS_FILE_NAME = "logs";

/**
 * @param args: array of arguments starting from args[1]
 * @return 1 to continue running, 0 to stop execution
 */
int cd_command(char **args);


/**
 * @return 0 to exit shell
 */
int exit_command();

/**
 * Defining Global Variables
 */
// List of program builtin commands
char *commands_list[] = {
        "cd",
        "exit"
};


// List of program builtin commands' functions
int (*commands_functions[])(char **) = {
        &cd_command,
        &exit_command
};


/**
 * @brief A SIGCHLD signal handler
 * @param sig: the SIGSCHILD signal number which is 17
 */
void sigchld_handler(int sig);

/**
 * @param line: input line taken from user
 * @return an array of characters representing user input
 */
char *get_user_input(char *line);

/**
 * @param line: input line taken from user
 * @param args: An empty pointer of pointers array
 * representing a command and its arguments
 * @return An array of pointers representing a command and arguments parsed from user input
 */
char **parse_user_input(char *line, char **args);

/**
 * @param args: An array of pointers representing a command and its arguments
 * @param background: if the process is to be executed in background, when it equals 1
 * @return 1 to keep the shell running, 0 to exit
 */
int execute_commands(char **args, int background);

/**
 * A function to start the shell program
 */
void run();

/**
 * @brief Removes the extra line taken from user input
 * @param line: user input
 */
void remove_new_line(char *line);

/**
 * @brief frees memory allocated for line and args
 * @param line: user input
 * @param args: Array of pointers containing parsed user input
 */
void free_memory(char *line, char **args);

/**
 * @brief Checks if the command is system defined or builtin in this program and runs the appropriate logic
 * @param args: An array of pointers representing a command and arguments parsed from user input
 * @param background: if the process is to be executed in background, when it equals 1
 * @return 1 to continue running the program
 */
int check_command(char **args, int background);

/**
 * @return number of builtin commands in this program
 */
int get_commands_count();

/**
 * @return number of arguments for the command taken from user
 */
int get_args_count(char **args);

/**
 * @return 1 if user entered ampersand '&' for running the command in background
 */
int check_background(char **args, int argsCount);

/**
 * @brief removes ampersand '&' taken from user input
 */
void remove_ampersand(char **args, int argsCount);

/**
 * @brief logs child termination information
 * @param pid: take process id
 */
void log_child_termination(pid_t pid);

int main() {

    // Start the program
    run();

    return EXIT_SUCCESS;
}

void run() {

    // Input line taken from user
    char *line = NULL;

    // Array of pointers to hold parsed user input into a command and its arguments
    char **args = NULL;

    // Program status
    int is_running;

    // Specify if the command to run in the background, when equals 1
    int run_in_background;

    do {
        // Take user input
        line = get_user_input(line);

        // Parse user input into arguments
        args = parse_user_input(line, args);

        // Checking if use entered '&' at the end
        // So that the command should run in background
        run_in_background = check_background(args, get_args_count(args));

        // Execute user command
        is_running = check_command(args, run_in_background);

        // Free allocated memory
        free_memory(line, args);

    } while (is_running);

}

int check_background(char **args, int argsCount) {

    // if the last arguments equals '&' remove it
    if (strcmp(args[argsCount - 1], "&") == 0) {
        remove_ampersand(args, argsCount);
        return 1;
    }
    return 0;
}

void remove_ampersand(char **args, int argsCount) {
    args[argsCount - 1] = NULL;
}

int check_command(char **args, int background) {

    // Ignore empty command
    if (args[0] == NULL)
        return 1;

    int num_commands = get_commands_count();

    // Loop through program builtin commands to check if one matches user input
    for (int i = 0; i < num_commands; ++i) {
        if (strcmp(args[0], commands_list[i]) == 0)
            return (*commands_functions[i])(args);
    }

    return execute_commands(args, background);
}

int get_commands_count() {
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
    remove_new_line(line);

    return line;
}

void remove_new_line(char *line) {
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

int execute_commands(char **args, int background) {

    // SIGCHLD signal handler call
    signal(SIGCHLD, sigchld_handler);

    pid_t pid, wpid;

    // Forking a child process
    pid = fork();

    // Child status holder
    int status;

    if (pid < 0) {
        // Error happened on forking
        perror(SHELL_NAME);
        exit(EXIT_FAILURE);

    } else if (pid == 0) {

        // Successful child process creation

        if (execvp(args[0], args) < 0)
            // Error Happened when executing command
            perror(SHELL_NAME);


        exit(EXIT_SUCCESS);

    } else {
        // Parent process

        // Don't wait in case of background command
        if (!background)

            do {

                wpid = waitpid(pid, &status, WUNTRACED);

                if (wpid > 0) {

                    if (WIFEXITED(status) && WEXITSTATUS(status)) {

                        if (WEXITSTATUS(status) == 127)
                            // Execution failed
                            perror("Execution Failed");
                        else
                            printf("Program terminated normally, but returned a non-zero status\n");

                    } else
                        printf("Program didn't terminate normally\n");

                } else
                    printf("Waitpid() failed");

                log_child_termination(pid);

            } while (!WIFEXITED(status) && !WIFSIGNALED(status));


    }
    // Retuning 1 to continue running the program
    return 1;
}

void log_child_termination(pid_t pid) {

    // Creating a file object in appending mode
    FILE *log;
    log = fopen(LOGS_FILE_NAME, "a");

    // Writing in the log file
    fprintf(log, "Child process with pid: %u was terminated.\n", pid);

    // Closing the file and checking for errors
    fclose(log);
}

int exit_command() {
    return 0;
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

int get_args_count(char **args) {
    int i = 0;
    while (args[i] != NULL)
        i++;

    return i;
}

void sigchld_handler(int sig) {
    pid_t pid;

    pid = wait(NULL);

    log_child_termination(pid);
}
