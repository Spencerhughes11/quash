#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char char_in;
// prints shell line
void print_quash(){
    printf("[QUASH]$ ");
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
    printf("\n\nWelcome to Quash!\n\nPress [Enter] to begin...\n\n");
    while(1){
        char_in = getchar();
        
        if (char_in == '\n'){
            print_quash();
            fflush(stdout);
        } else {
            // print_quash();
            printf("\n");
        }
    }
    return 0;
}