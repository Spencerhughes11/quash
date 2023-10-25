#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>


static char cmd_char = '\0';
static char buf[1024];
int buffer_count = 0;


char input[1024];


char *argv[];
int argc;

DIR *dir;
struct dirent *dirent;


/* COMMAND LINE PARSER 
 *    stores argc and argv values to access
 *   command line input outside of main
*/
void parse_command_line() {
    while (argc != 0) {
        argv[argc] = NULL; 
        argc--;
    }

    buffer_count = 0;
    char *new_buf;

    while (cmd_char != '\n') {
        buf[buffer_count++] = cmd_char;
        cmd_char = getchar();
    }
    buf[buffer_count] = 0x00;
    new_buf = strtok(buf, " ");         // tokenize, separated by spaces
    
    while(new_buf != NULL) {
        argv[argc] = new_buf;
        new_buf = strtok(NULL, " "); 
        argc++;
    }

}

// prints shell line
void print_quash(){
    printf("\n[QUASH]$ ");
}

// prints pwd
void run_pwd() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

void run_cd() {
    if (argv[1] == NULL || argv[1] == '~'){
        chdir(getenv("HOME"));
    }
}

void run_ls() {
    dir = opendir(".");         // opens current directory

    while ((dirent = readdir(dir))) {
        printf("%s\n", dirent->d_name);         // while in current directory, print all content
    }

    closedir(dir);      // closes directory
}

void run_echo(){
    
}

int run_commands(){
    fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';

    // exit or quit prompt
    
    if (strcmp("exit", argv[0]) == 0 || strcmp(argv[0], "quit") == 0 ){
        printf("Exiting quash...\n\n");
        exit(0);
    }

    // if (strcmp("#", argv[0]) == 0){
    //     return 0;
    // }

    // prints cd
    if (strcmp("cd", argv[0]) == 0) {
        run_cd();
        run_pwd();
        return 0;
    }
    if (strcmp("ls", input) == 0) {
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

}

int main (int argc, char *argv[]) 
{

    printf("\n\nWelcome to Quash!\n\n");
    while(1){
        print_quash();
        
        // cmd_char = getchar();
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        
        if (input == ' '){
            print_quash();
        } else {
            // parse_command_line();
            run_commands();

            print_quash();
        }
        printf(argv[0]);
    }
    return 0;
}