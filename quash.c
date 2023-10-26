#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

// Define a maximum number of background jobs
#define MAX_BACKGROUND_JOBS 10

char input[1024];


char *argv[];
int argc;

DIR *dir;
struct dirent *entry;


// Array to store the PIDs of background jobs
pid_t background_jobs[MAX_BACKGROUND_JOBS] = {0};
char background_processes[MAX_BACKGROUND_JOBS];


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

// * FIXME: implement path echoes

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
        if( argv[ i ][ 0 ] == '$' ){
            env = argv[i] + 1;
            path = getenv(env);
            printf(path);
            is_env = 1;
        }
        if ((argv[i][0] == *single_quote) || (argv[i][0] == '"') ){       // handles if string begins with single or double quote
            // argv[i] = argv[i+1];
            argv[i] = argv[i] + 1;      // skips
        }
        if ((argv[i][strlen(argv[i]) - 1] == *single_quote) || (argv[i][strlen(argv[i]) - 1] == '"') ){       // handles if string ends with single or double quote
            argv[i][strlen(argv[i]) - 1] ='\0';         // sets to null
        }
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
    // printf("Environment env %s set to %s\n", env, new_path);
    // } else {
    //     perror("setenv");
    // }
    

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


        
            printf("JobID: %d process: %s \n",background_jobs[i],&background_processes[i]);
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
                printf("Completed: [JOBID] %d %s\n", result, &background_processes[i]); // You can replace [JOBID] and COMMAND as needed.
                background_jobs[i] = 0; // Clear the job from the array
                // free(background_processes[i]);
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
                strncpy(&background_processes[i],argv[0],sizeof(argv[0]+3));
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
    if (strcmp("&", argv[argc - 1]) == 0){

        argv[argc - 1] = NULL;
        run_execution();
        return 0;
     
    } else{
        run_foreground();

    }
    // clear input arr for each new iteration
}

int main (int argc, char *argv[]) 
{

    printf("\n\nWelcome to Quash!\n\n");
    while(1){
        // print_quash();
        
        // cmd_char = getchar();
        // fgets(input, sizeof(input), stdin);
        // input[strlen(input) - 1] = '\0';
        if (argc == 0){
            continue;
        }
        if (argv[0] == "\n"){
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