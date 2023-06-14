#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>



void printHelp(){
    printf("Mandatory arguments missing or zero.\n");
    printf("Usage: ./csim [-v] -s <s> -b <b> -E <E> -t <trace>\n");
    printf("       ./csim -h\n");
    printf("    -h          Print this help message and exit\n");
    printf("    -v          Verbose mode: report effects of each memory operation\n");
    printf("    -s <s>      Number of set index bits (there are 2**s sets)\n");
    printf("    -b <b>      Number of block bits (there are 2**b blocks)\n");
    printf("    -E <E>      Number of lines per set (associativity)\n");
    printf("    -t <trace>  File name of the memory trace to process\n\n");
    printf("The -s, -b, -E, and -t options must be supplied for all simulations.\n");

}

/**
 * @brief main function
 * 
 * @param [in] argv Command Line argument,  [-v] -s <s> -E <E> -b <b> -t <trace>
*/
int main(int argc, char**argv[]){
    int opt;

    while((opt = getopt(argc, argv,"vhs:E:b:t:"))!=-1){
        switch (opt){
            case 'v':
            
            break;

            case 'h':
            break;

            case 's':
            break;

            case 'E':
            break;

            case 'b':
            break;
            
            case 't':
            break;

            case ':':
            break;

            case '?':
            break;

            
            default:

            break;
        }
    }
    
    
    return 0;
}
