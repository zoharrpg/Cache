#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

unsigned int LINELEN = 21;

typedef struct{
    bool dirty;
    unsigned long tag;
    int count;

}cache_line;

cache_line** cache;

long set_numbe = -1;
long block_size = -1;
long associativity =-1;
long set_bits =-1;
long block_bits = -1;
char *file_name = NULL;



initCache(){

}

/** Process a memory-access trace file.
 * 
 * @param trace Name of the trace file to process
 * @return 0 if successful, 1 if there were error
*/
int process_trace_file(const char *trace){
    
    FILE *tfp = fopen(trace,"rt");
    if(!tfp){
        fprintf(stderr,"Error opening '%s': %s\n",
                trace,strerror(errno));
        return 1;
    }
    char linebuf[LINELEN];
    int parse_error = 0;
    while (fgets(linebuf, LINELEN,tfp)){


    }
    fclose(tfp);
    return parse_error;




}

/**
 * @brief print help message
*/
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
                printf("This is v mode");
            
                break;

            case 'h':
                printHelp();
                break;

            case 's':
                set_bits = strtoul(optarg,NULL,10);
                break;

            case 'E':
                associativity = strtoul(optarg,NULL,10);
                break;

            case 'b':
                block_bits = strtoul(optarg,NULL,10);

                break;
            
            case 't':
                file_name = optarg;
                break;

            case ':':
                printf("This is : option\n");
                printHelp();
                break;

            case '?':
                printf("This is ? option\n");
                printHelp();

                break;

            
            default:
                printf("This is defalut\n");
                printHelp();

                break;
        }
    }

    
    
    return 0;
}
