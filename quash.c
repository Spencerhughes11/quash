#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>


char input[1024];


char *argv[];
int argc;

DIR *dir;
struct dirent *entry;


// prints shell line
void print_quash(){
    printf("[QUASH]$ ");
}

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

void run_echo(){
    
}

/* COMMAND LINE PARSER 
 *    stores argc and argv values to access
 *   command line input outside of main
*/
void parse_command_line() {
    int argc = 0;
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

    parse_command_line();
    // exit or quit prompt
    
    if (strcmp("exit", argv[0]) == 0 || strcmp(argv[0], "quit") == 0 ){
        printf("Exiting quash...\n\n");
        exit(0);
    }

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
        run_echo();
        return 0;
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
        if (*input == ' '){
            print_quash();
        } else {
            parse_command_line();
            print_quash();
            run_commands();
            

        }
        for (int i = 0; i < argc; i++) {
                argv[i] = NULL;
        }
    }
    return 0;
}