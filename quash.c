#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static char char_in;
char input[1024];

char *argv[];
int argc;

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

int run_commands(){
    fgets(input, sizeof(input), stdin);
    input[strlen(input) - 1] = '\0';

    // exit or quit prompt
    
    if (strcmp("exit", input) == 0 || strcmp(input, "quit") == 0 ){
        printf("Exiting quash...\n\n");
        exit(0);
    }

    // if (strcmp("#", input) == 0){
    //     return 0;
    // }

    // prints cd
    if (strcmp("cd", input) == 0) {
        run_cd();
        run_pwd();
        return 0;
    }
    // prints pwd
    if (strcmp("pwd", input) == 0) {
        run_pwd();
        return 0;
    }
}

int main (int argc, char *argv[]) 
{
//     printf(" -----------------------------------------------------------------------------------------------------------"                                                                                                           
//    "  QQQQQQQQQ     UUUUUUUU     UUUUUUUU           AAA                 SSSSSSSSSSSSSSS HHHHHHHHH     HHHHHHHHH"
//    "QQ:::::::::QQ   U::::::U     U::::::U          A:::A              SS:::::::::::::::SH:::::::H     H:::::::H"
//  "QQ:::::::::::::QQ U::::::U     U::::::U         A:::::A            S:::::SSSSSS::::::SH:::::::H     H:::::::H"
// "Q:::::::QQQ:::::::QUU:::::U     U:::::UU        A:::::::A           S:::::S     SSSSSSSHH::::::H     H::::::HH"
// "Q::::::O   Q::::::Q U:::::U     U:::::U        A:::::::::A          S:::::S              H:::::H     H:::::H"  
// "Q:::::O     Q:::::Q U:::::D     D:::::U       A:::::A:::::A         S:::::S              H:::::H     H:::::H"  
// "Q:::::O     Q:::::Q U:::::D     D:::::U      A:::::A A:::::A         S::::SSSS           H::::::HHHHH::::::H"  
// "Q:::::O     Q:::::Q U:::::D     D:::::U     A:::::A   A:::::A         SS::::::SSSSS      H:::::::::::::::::H"  
// "Q:::::O     Q:::::Q U:::::D     D:::::U    A:::::A     A:::::A          SSS::::::::SS    H:::::::::::::::::H"  
// "Q:::::O     Q:::::Q U:::::D     D:::::U   A:::::AAAAAAAAA:::::A            SSSSSS::::S   H::::::HHHHH::::::H"  
// "Q:::::O  QQQQ:::::Q U:::::D     D:::::U  A:::::::::::::::::::::A                S:::::S  H:::::H     H:::::H" 
// "Q::::::O Q::::::::Q U::::::U   U::::::U A:::::AAAAAAAAAAAAA:::::A               S:::::S  H:::::H     H:::::H"  
// "Q:::::::QQ::::::::Q U:::::::UUU:::::::UA:::::A             A:::::A  SSSSSSS     S:::::SHH::::::H     H::::::HH"
//  "QQ::::::::::::::Q   UU:::::::::::::UUA:::::A               A:::::A S::::::SSSSSS:::::SH:::::::H     H:::::::H"
//    "QQ:::::::::::Q      UU:::::::::UU A:::::A                 A:::::AS:::::::::::::::SS H:::::::H     H:::::::H"
//      "QQQQQQQQ::::QQ      UUUUUUUUU  AAAAAAA                   AAAAAAASSSSSSSSSSSSSSS   HHHHHHHHH     HHHHHHHHH"
//              "Q:::::Q"                                                                                         
//               "QQQQQQ\n\n\r");
    printf("\n\nWelcome to Quash!\n\n");
    while(1){
        print_quash();
        
        // char_in = getchar();
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        
        if (input == ' '){
            print_quash();
        } else {
            print_quash();
            run_commands();
        }
    }
    return 0;
}