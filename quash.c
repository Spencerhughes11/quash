#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/wait.h>

// Define a maximum number of background jobs
#define MAX_BACKGROUND_JOBS 10

char input[1024];


char *argv[];
int argc;

DIR *dir;
struct dirent *entry;


// Array to store the PIDs of background jobs
pid_t background_jobs[MAX_BACKGROUND_JOBS] = {0};


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

void run_ls() {
    dir = opendir(".");         // opens current directory

    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){  
            printf("%s\t", entry->d_name);         // while in current directory, print all content
        }
    }
    printf("\n");
    closedir(dir);      // closes directory
}

// * FIXME: won't run multilevel env echoes
// * ex: echo $HOME/Desktop

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
            env = argv[i] + 1;
            path = getenv(env);
            printf(path);
            is_env = 1;         // is env case
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

/* **********************************************************************************
   *                                      JOBS                                      *
   ********************************************************************************** */

void check_background_jobs() {
    for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
        if (background_jobs[i] != 0) {
            int status;
            pid_t result = waitpid(background_jobs[i], &status, WNOHANG);
            if (result > 0) {
                // The background job with PID result has completed
                printf("Completed: [JOBID] %d %s\n", result, argv[0]); // You can replace [JOBID] and COMMAND as needed.
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
        if (execvp(argv[0], NULL) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        // Parent process
        // In the parent process, you can print the background job started message
        printf("Background job started: PID %d\n", pid);


        
        for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
            if (background_jobs[i] == 0) {
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
        } else {
            // Child process has completed
            // if (WIFEXITED(status)) {
            //     printf("Foreground job %d exited with status %d\n", pid, WEXITSTATUS(status));
            // }
        }
    }
}

/* COMMAND LINE PARSER 
 *   stores argc and argv values to access
 *   command line input outside of main
*/
void parse_command_line() {
    argc = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    
}

int run_commands(){
    fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';
    
    // parse_command_line();
    int argc = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    
    // exit or quit prompt
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
    if (strcmp("ls", argv[0]) == 0) {
        run_ls();
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
}

int main (int argc, char *argv[]) 
{

    printf("\n\nWelcome to Quash!\n\n");
    while(1){

        if (argc == 0){
            continue;
        }
        if (*input == ' '){
            print_quash();
        } else {
            parse_command_line();
            print_quash();
            run_commands();
            

        }
        check_background_jobs();

    }
    return 0;
}