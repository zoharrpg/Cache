#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define DECIMAL_BASE 10
#define LINELEN  21

typedef struct{
    bool dirty;
    unsigned long tag;
    int count;

}cache_line;

cache_line** cache;


unsigned long set_number;
unsigned long block_size ;
unsigned long associativity;
unsigned long set_bits;
unsigned long block_bits;
char *file_name = NULL;


/**
 * Initialize Cache
*/
void initCache(void){
    set_number = 1 << set_bits;
    block_size = 1 << block_bits;

    cache = malloc(set_number*sizeof(cache_line*));

    for (unsigned long i=0;i<set_number;i++){
        cache[i] = malloc(associativity * sizeof(cache_line));
        for (unsigned long j=0;j<associativity;j++){
            cache[i][j].dirty=false;
            cache[i][j].tag = -1;
            cache[i][j].count = 0;
        }
    }
    


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
        char operation = *strtok(linebuf," ");
        unsigned long address = strtoul(strtok(linebuf," "),NULL,DECIMAL_BASE);
        unsigned long size = strtoul(strtok(linebuf,","),NULL,DECIMAL_BASE);
    
    }
    fclose(tfp);
    return parse_error;




}

/**
 * @brief print help message
*/
void printHelp(void){
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
int main(int argc, char*argv[]){
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
                set_bits = strtoul(optarg,NULL,DECIMAL_BASE);
                break;

            case 'E':
                associativity = strtoul(optarg,NULL,DECIMAL_BASE);
                break;

            case 'b':
                block_bits = strtoul(optarg,NULL,DECIMAL_BASE);

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
    initCache();
    process_trace_file(file_name);


    
    
    return 0;
}
