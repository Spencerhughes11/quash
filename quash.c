/* ***********************************************************************************
   *                                      QUASH                                      *
   *                                                                                 *
   *                    CREATED BY: Spencer Hughes and Pete Junge                    *
   *********************************************************************************** */


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

// Define a maximum number of background jobs
#define MAX_BACKGROUND_JOBS 100
#define PATH ""

char input[100];
char command_in[128];
char command_out[128];
char *s, buf[1024];


char **argv;
// char *argv[1024];
int argc;

int job_num = 0;

DIR *dir;
struct dirent *entry;

#define MAX_ARGS 64


// Array to store the PIDs of background jobs
pid_t background_jobs[MAX_BACKGROUND_JOBS] = {0};
char background_processes[MAX_BACKGROUND_JOBS][128];


// prints shell line
void print_quash(){
    printf("[QUASH]$ ");
}

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
    // char up_dir = "..";
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

// * FIXME: won't run multilevel env echoes
// * ex: echo $HOME/Desktop
// * REDIRECTION: still echoes string before > or <

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
        // handle environment variables
        if( argv[ i ][ 0 ] == '$' ){
            char *env_path = getenv(argv[i]);
        }
        if (strcmp(argv[i], "&") == 0 || strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0){
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


// int is_symbol (int argc) {
//     int is_in = 0, is_out = 0;

//     for (int i = 0; i < argc; i++){
//             // Redirect OUT
//             if (strcmp(argv[i], ">") == 0){
//                 argv[i] = NULL;
//                 strcpy(command_out, argv[i+1]);
//                 is_out = 1;
//             }
//             if (strcmp(argv[i], "<") == 0){
//                 argv[i] = NULL;
//                 strcpy(command_in, argv[i+1]);
//                 is_in = 1;
//             }
//         }
//     return 0;
// }

// * FIXME: not working yet 
void redirect(int argc) {
    int is_in = 0, is_out = 0;
    int redirect_index = 0;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0) {
            redirect_index = i;
            is_out = 1;
        } else if (strcmp(argv[i], "<") == 0) {
            redirect_index = i;
            is_in = 1;
        }
    }

    pid_t pid = fork();

    if (pid == 0) {
        int fd_in, fd_out;
        if (is_out) {
            fd_out = open(argv[redirect_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        } else if (is_in) {
            fd_in = open(argv[redirect_index + 1], O_RDONLY);
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        // Modify the argv array to exclude the "<" or ">" symbol and the filename
        argv[redirect_index] = NULL;
        for (int i = 0; i < argc; i++) {
            printf("argv[%d]: %s\n", i, argv[i]);
        }
        if (execvp(argv[0], argv) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        wait(NULL);
    }
}

void run_pipe(char firstcommand[],char secondcommand[],char lhs[],char rhs[]){
    int pipe_fd[2]; // Pipe file descriptors

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t child_pid1 = fork();
    pid_t child_pid2 = fork();

    if (child_pid1 == -1 || child_pid2 == -1) {
        perror("fork");
        exit(1);
    }

    if (child_pid1 == 0) {

        

        // Redirect the read end of the pipe to standard input (stdin)
        dup2(pipe_fd[1], STDOUT_FILENO);
        
        // Close the read end of the pipe

        execvp(firstcommand[0],lhs);
    }
    if (child_pid2 == 0){
        // Parent process        
        // Redirect standard output (stdout) to the write end of the pipe
        dup2(pipe_fd[0], STDIN_FILENO);

        execvp(secondcommand[0],rhs);
    }
    close(pipe_fd[1]);
    close(pipe_fd[0]);
        
    
}
void make_pipe(int argc) {
    int num_pipes = 0;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "|") == 0) {
            num_pipes++;
        }
    }

    int command_start = 0;
    int command_end;

    for (int pipe_num = 0; pipe_num <= num_pipes; pipe_num++) {
        // Find the end of the current command
        command_end = command_start;
        while (command_end < argc && strcmp(argv[command_end], "|") != 0) {
            command_end++;
        }

        // Create a new pipe
        int fd[2];
        pipe(fd);

        pid_t child_pid = fork();

        if (child_pid == 0) {
            // In the child process
            if (pipe_num > 0) {
                // If this is not the first command, redirect the stdin from the previous pipe
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]); // Close the read end of the pipe
            }
            if (pipe_num < num_pipes) {
                // If this is not the last command, redirect the stdout to the current pipe
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]); // Close the write end of the pipe
            }

            // Execute the command
            argv[command_end] = NULL; // Null-terminate the current command
            execvp(argv[command_start], &argv[command_start]);
            perror("Command execution failed");
            exit(1);
        } else {
            // In the parent process
            if (pipe_num > 0) {
                close(fd[0]); // Close the read end of the previous pipe
            }
            if (pipe_num < num_pipes) {
                close(fd[1]); // Close the write end of the current pipe
            }

            // Move to the next command
            command_start = command_end + 1;
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i <= num_pipes; i++) {
        wait(NULL);
    }
}



/* **********************************************************************************
   *                                      JOBS                                      *
   ********************************************************************************** */

void run_kill(int signal, int pid){
    if (kill(pid, signal) == 0) {
        printf("Signal %d sent to process with ID %d\n", signal, pid);
        return 0;
    } else {
        perror("Signal sending failed");
        return -1;
    }
}
void run_jobs(){
    // printf("Current Running Jobs:");
    for (int i = 0; i < 10;i++){
        if (background_jobs[i] != 0){


        
            printf("[%d] %d %s \n",i+1,background_jobs[i],&background_processes[i]);
        }
    }
}

void check_background_jobs() {
    for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
        if (background_jobs[i] != 0) {
            int status;
            pid_t result = waitpid(background_jobs[i], &status, WNOHANG);
            if (result > 0) {
                // The background job with PID result has completed
                printf("Completed: [%d] %d %s\n", job_num, result, &background_processes[i]); // You can replace [JOBID] and COMMAND as needed.
                background_jobs[i] = 0; // Clear the job from the array

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
        // Child process
        // Execute the command in the child process
        if (execvp(argv[0], argv) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        // Parent process
        // In the parent process, you can print the background job started message
        job_num++;
        printf("Background job started: [%d] %d\n", job_num, pid);

        for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
            if (background_jobs[i] == 0) {
                // char input[] = argv[0];
                strncpy(&background_processes[i],input,sizeof(input));
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
        // Child process
        // Execute the command in the child process
        if (execvp(argv[0], argv) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        // Parent process
        int status;
        // Wait for the child process to complete
        if (waitpid(pid, &status, 0) == -1) {
            perror("Wait failed");
        } 
    }
}


/* COMMAND LINE PARSER 
 *   stores argc and argv values to access
 *   command line input outside of main
*/
// void parse_command_line() {
//     argc = 0;
//     char *token = strtok(input, " ");
//     while (token != NULL) {
//         argv[argc++] = token;
//         token = strtok(NULL, " ");
//     }
//     argv[argc] = NULL;
    
// }

/* Strips $ off environment variables in order to pass on*/


/* COMMAND LINE PARSER 
 *   stores argc and argv values to access
 *   command line input outside of main
*/
char **tokenize(char *input) {
    argv = (char **)malloc(sizeof(char *));
    if (!argv) {
        perror("Memory allocation error");
        exit(1);
    }

    int argc = 0;
    char *token = strtok(input, " \t\n");
    
    while (token != NULL) {
        // Reallocate memory for argv to accommodate a new argument
        char **new_argv = (char **)realloc(argv, (argc + 2) * sizeof(char *));
        if (!new_argv) {
            perror("Memory reallocation error");
            exit(1);
        }
        argv = new_argv;

        // Allocate memory for the token and copy it
        argv[argc] = (char *)malloc(strlen(token) + 1);
        if (!argv[argc]) {
            perror("Memory allocation error");
            exit(1);
        }
        strcpy(argv[argc], token);

        argc++;
        token = strtok(NULL, " \t\n");
    }

    // Null-terminate the argv array
    argv[argc] = NULL;

    return argv;
}


int run_commands(){

    argv = tokenize(input);
    int argc = 0;
    while (argv[argc] != NULL) {
        argc++;
    }

    // printf("Input: %s\n", input);
    // printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
        // printf(argv[i][0]);
    }
    argv[argc] = NULL;
    if (argc > 3){
        // try to use argc to compare the length of the command so that it will not run the fist command but will run the pipe 
        for (int i = 0; i < argc; i++) {
            if (strcmp("|", argv[i]) == 0) {
                char lhs[10][100];
                char rhs[10][100];
                char firstcommand[128];
                strncpy(&firstcommand[0], argv[0],sizeof(firstcommand));
                char secondcommand[128];
                strncpy(&secondcommand[0], argv[i+1],sizeof(secondcommand));

                for (int j = 1; j < i; j++) {
                    if (j < 10) {
                        strncpy(lhs[j], argv[j], sizeof(lhs[j]) - 1);
                        lhs[j][sizeof(lhs[j]) - 1] = '\0';
                    }
                }

                for (int k = i + 2; k < argc; k++) {
                    int rhsIndex = k - i - 1;
                    if (rhsIndex < 10) {
                        strncpy(rhs[rhsIndex], argv[k], sizeof(rhs[rhsIndex]) - 1);
                        rhs[rhsIndex][sizeof(rhs[rhsIndex]) - 1] = '\0';
                    }
                }
                printf(lhs);
                printf(rhs);
                run_pipe(firstcommand,secondcommand,lhs, rhs);    
    
        }
    }
    }
    
    // printf("argv: %s\n", argv);
    // exit or quit prompt
    // environment_vars(argc);

    if (strcmp("exit", argv[0]) == 0 || strcmp(argv[0], "quit") == 0 ){
        printf("Exiting quash...\n\n");
        exit(0);
    }

    // ignore comments
    if (strcmp("#", argv[0]) == 0){
        return 0;
    }

    // prints cd
    if (strcmp("cd", argv[0]) == 0) {
        run_cd();
        // run_pwd();
        return 0;
    }

    if (strcmp("kill", argv[0]) == 0) {
        int signalNumber = atoi(argv[1]); // Convert signal number to integer
        int targetPID = atoi(argv[2]);
        run_kill(signalNumber, targetPID);
        return 0;
    }
    if (strcmp("jobs", argv[0]) == 0) {
        run_jobs();
        return 0;
    }

    // prints pwd
    if (strcmp("pwd", argv[0]) == 0) {
        run_pwd();
        return 0;
    }
    if (strcmp("echo", argv[0]) == 0) {
        run_echo(argc);
        return 0;
    }
    if (strcmp("export", argv[0]) == 0) {
        run_export();
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
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0) {
            // printf("argv[%d]: %s\n", i, argv[i]);
            redirect(argc);
            return 0;
        }
        if (strcmp(argv[i], "|") == 0) {
            // printf("argv[%d]: %s\n", i, argv[i]);
            make_pipe(argc);
            return 0;
        }
    }


}


// FIXME: DOEs not work
// CTRL Z still kills 

void ignore_signals() {

        signal( SIGINT, SIG_IGN );
        signal( SIGTSTP, SIG_IGN );
        signal( SIGQUIT, SIG_IGN );       
}

int main (int argc, char *argv[])  {
    printf("\n\nWelcome to Quash!\n\n");
    while (1) {
        ignore_signals();

        print_quash();  // Print the shell prompt
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';  // Remove the newline character

        // Check for empty input
        if (strlen(input) == 0) {
            continue;
        }

        // Process the user input
        run_commands();
        check_background_jobs();
    }

    return 0;
}