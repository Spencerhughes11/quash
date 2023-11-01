/* ***********************************************************************************
   *                                      QUASH                                      *
   *                                                                                 *
   *                    CREATED BY: Spencer Hughes and Pete Junge                    *
   *********************************************************************************** */
// libraries
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

/* ********************** GLOBAL VARIABLES ********************** */
#define MAX_BACKGROUND_JOBS 100

char input[100];
char command_in[128];
char command_out[128];
char *s, buf[1024];

char *argv[64];
int argc;

int job_num = 0;

DIR *dir;
struct dirent *entry;

#define MAX_ARGS 64

// Array to store the PIDs of background jobs
pid_t background_jobs[MAX_BACKGROUND_JOBS] = {0};
char background_processes[MAX_BACKGROUND_JOBS][128];

/* **********************************************************************************
   *                              BASIC SHELL COMMANDS                              *
   ********************************************************************************** */

// prints pwd
void run_pwd() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

void run_cd() {
    if (argv[1] == NULL || strcmp(argv[1], "~") == 0){
        chdir(getenv("HOME"));
    } else if (strcmp(argv[1], "..") == 0 || strcmp(argv[1], "../") == 0) {
        chdir("..");
    } else {
        if (chdir(argv[1]) == -1) {
            perror("cd");
        }
    }
}

void run_echo(int argc){
    char *single_quote = "'";
    char *path;
    char *env;
    int is_env = 0;

    for (int i = 1; i < argc; i++){
        // ignores comments
        if (strcmp("#", argv[i]) == 0){
            break;      
        }

        // Handle environment variables
        char *arg = argv[i];
        if (arg[0] == '$') {
            char *env_value = getenv(&arg[1]);  // Remove the '$' sign when calling getenv
            if (env_value != NULL) {
                printf("%s ", env_value);
            }
            is_env = 1;
        } 
        // ignore symbols
        if (strcmp(argv[i], "&") == 0 || strcmp(argv[i], ">") == 0
        || strcmp(argv[i], "<") == 0 || strcmp(argv[i], "|") == 0 || strcmp(argv[i], ">>") == 0){
                break;
        }
        if ((argv[i][0] == *single_quote) || (argv[i][0] == '"') ){       // handles if string begins with single or double quote
            argv[i] = argv[i] + 1;      // skips
        }
        if ((argv[i][strlen(argv[i]) - 1] == *single_quote) || (argv[i][strlen(argv[i]) - 1] == '"') ){       // handles if string ends with single or double quote
            argv[i][strlen(argv[i]) - 1] ='\0';         // sets to null
        }
        // if not environment case, echo args
        if (!is_env){
            printf("%s ", argv[i]);
        }
    }
    printf("\n");
}

void run_export() {
    // Split input into environment and the desired new path
    char *env = strtok(argv[1], "=");
    char *new_path = strtok(NULL, "=");
    setenv(env, new_path, 1);

}

/* ***********************************************************************************
   *                              REDIRECTION AND PIPES                              *
   *********************************************************************************** */

void redirect(int argc) {
    int is_in = 0, is_out = 0;
    int redirect_index = 0;

    char which_symbol[3];
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            redirect_index = i;
            is_out = 1;
            strncpy(which_symbol, argv[i], 2);
            which_symbol[2] = '\0';     // null terminate
        } 
        if (strcmp(argv[i], "<") == 0) {
            redirect_index = i;
            is_in = 1;
        }
    }
    char *file = argv[redirect_index + 1];
    // Remove the redirection symbol, the filename, and their arguments
    for (int i = redirect_index; i < argc - 2; i++) {
        argv[i] = argv[i + 2];
    }
    argv[redirect_index] = NULL;

    // Create a child process to execute the command
    pid_t pid = fork();
    if (pid == 0) {
        int fd_in, fd_out;
        if (is_out) { 
            if (strcmp(which_symbol, ">") == 0) {
                printf("IN >");
                fd_out = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            } else if (strcmp(which_symbol, ">>") == 0) {
                printf("IN >>");
                fd_out = open(file, O_WRONLY | O_APPEND, 0644);     // write as append
            } 
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        } else if (is_in) {
            fd_in = open(file, O_RDONLY);
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        // Execute the command in the child process
        if (execvp(argv[0], argv) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        // Wait for the child process to complete
        wait(NULL);
    }
}

void make_pipe(int argc) {
    int num_pipes = 0;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "|") == 0) {
            num_pipes++;
        }
    }

    // variables for separate commands
    int start_command = 0;
    int end_command;
    int fds[num_pipes][2];      // will need multiple file descriptors

    for (int pipe_num = 0; pipe_num < num_pipes; pipe_num++) {
        pipe(fds[pipe_num]);        // pipe for each command
    }

    for (int pipe_num = 0; pipe_num < num_pipes + 1; pipe_num++) {
        // Find the end of the current command
        end_command = start_command;
        while (end_command < argc && strcmp(argv[end_command], "|") != 0) {
            end_command++;
        }

        pid_t child_pid = fork();

        if (child_pid == 0) {
            // In the child process
            if (pipe_num > 0) {
                // Redirect in from the left pipe
                dup2(fds[pipe_num - 1][0], STDIN_FILENO);
                close(fds[pipe_num - 1][0]);
                close(fds[pipe_num - 1][1]);
            }
            if (pipe_num < num_pipes) {
                // Redirect output to the current pipe
                dup2(fds[pipe_num][1], STDOUT_FILENO);
                close(fds[pipe_num][0]);
                close(fds[pipe_num][1]);
            }

            // Execute the command
            int temp_argc = end_command - start_command;
            char *temp_argv[temp_argc + 1];

            // repopulate new argv array with only certain commands
            for (int i = 0; i < temp_argc; i++) {
                temp_argv[i] = argv[start_command + i];
            }

            temp_argv[temp_argc] = NULL;        // null terminate

            execvp(temp_argv[0], temp_argv);        // execute args in new array
            perror("Command execution failed");
            exit(1);

        } else {
            // parent
            if (pipe_num > 0) {
                // close pipes
                close(fds[pipe_num - 1][0]);
                close(fds[pipe_num - 1][1]);
            }

            // Move to the next command
            start_command = end_command + 1;
        }
    }

    // Close any remaining pipes
    for (int i = 0; i < num_pipes; i++) {
        close(fds[i][0]);
        close(fds[i][1]);
    }

    // Wait for child processes to finish
    for (int i = 0; i <= num_pipes; i++) {
        wait(NULL);
    }
}

/* **********************************************************************************
   *                                      JOBS                                      *
   ********************************************************************************** */

int run_kill(int signal, int pid){
    if (kill(pid, signal) == 0) {
        printf("Signal %d sent to process with ID %d\n", signal, pid);
        return 0;
    } else {
        perror("Signal sending failed");
        return -1;
    }
}
void run_jobs(){
    for (int i = 0; i < 10;i++){
        if (background_jobs[i] != 0){
            printf("[%d] %d %s \n",i+1,background_jobs[i],background_processes[i]);
        }
    }
}

// checks background jobs for completion
void check_background_jobs() {
    for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
        if (background_jobs[i] != 0) {
            int status;
            pid_t result = waitpid(background_jobs[i], &status, WNOHANG);
            if (result > 0) {
                // completed background job notification
                printf("Completed: [%d] %d %s\n", job_num, result, background_processes[i]);       
                background_jobs[i] = 0;         // Clear job from array

            }
        }
    }
}

void run_execution() {
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child
        // Execute the command in the child process
        if (execvp(argv[0], argv) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        // In the parent process, you can print the background job started message
        job_num++;
        printf("Background job started: [%d] %d\n", job_num, pid);

        for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
            if (background_jobs[i] == 0) {
                strncpy(background_processes[i],input,sizeof(input));
                background_jobs[i] = pid;
                break;
            }
        }

    }

}

void run_foreground() {
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Execute command in the child process
        if (execvp(argv[0], argv) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        int status;
        // Wait for  child process to complete
        if (waitpid(pid, &status, 0) == -1) {
            perror("Wait failed");
        } 
    }
}

// strips environment variables of $ to pass 
void environment_variables(int argc) {
    for (int i = 0; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] == '$') {
            char *env_name = &arg[1]; // Remove the '$' sign when accessing the variable name
            char *env_value = getenv(env_name);
            strcpy(argv[i], env_value);
         }
    }
}

int run_commands(){
    /* COMMAND LINE PARSER 
        stores argc and argv values to access
        command line input outside of main
    */
    int argc = 0;
    char *token = strtok(input, " \n\t");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    // handles environment variables
    environment_variables(argc);

    if (strcmp("exit", argv[0]) == 0 || strcmp(argv[0], "quit") == 0 ){
        printf("Exiting quash...\n\n");
        exit(0);
    }

    // ignore comments
    if (strcmp("#", argv[0]) == 0){
        return 0;
    }

    // checks for redirection or pipes
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">>") == 0) {
            redirect(argc);
            return 0;
        }
        if (strcmp(argv[i], "|") == 0) {
            make_pipe(argc);
            return 0;
        }
    }
    
    if (strcmp("cd", argv[0]) == 0) {
        run_cd();
        return 0;
    }

    if (strcmp("kill", argv[0]) == 0) {
        int signalNumber = atoi(argv[1]);       // Convert signal number to integer
        int targetPID = atoi(argv[2]);
        run_kill(signalNumber, targetPID);
        return 0;
    }

    if (strcmp("jobs", argv[0]) == 0) {
        run_jobs();
        return 0;
    }

    if (strcmp("pwd", argv[0]) == 0) {
        run_pwd();
        return 0;
    }
    
    if (strcmp("export", argv[0]) == 0) {
        run_export();
        return 0;
    }

    if (strcmp("echo", argv[0]) == 0) {
        run_echo(argc);
        return 0;
    }

    // Background vs. Foreground execution
    if (strcmp("&", argv[argc - 1]) == 0){

        argv[argc - 1] = NULL;
        run_execution();
        return 0;
     
    } else{
        run_foreground();

    }

}

void ignore_signals() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);       
}

int main (int argc, char *argv[])  {
    printf("\nWelcome to Quash!\n\n");
    while (1) {
        ignore_signals();

        printf("[QUASH]$ ");
        //take input
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';        // Remove newline 

        // skip empty input
        if (strlen(input) == 0) {
            continue;
        }
      
        run_commands();
        check_background_jobs();
    }

    return 0;
}