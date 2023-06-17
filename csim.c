#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cachelab.h"
#include <limits.h>

#define DECIMAL_BASE 10
#define HEX_BASE 16
#define LINELEN  21

typedef struct{
    bool valid;
    bool dirty;
    unsigned long tag;
    unsigned long time;

}cache_line;

cache_line** cache;



unsigned long set_number;
unsigned long block_size ;
unsigned long associativity;
unsigned long set_bits;
unsigned long block_bits;
char *file_name = NULL;
csim_stats_t *stats;
bool is_v_mode = false;

unsigned long LRU_timer=0;

/**
 * @brief Initialize statistics
*/
void initStats(void){
    stats = malloc(sizeof(csim_stats_t));
    stats->hits=0;
    stats->misses = 0;
    stats->evictions = 0;
    stats->dirty_bytes = 0;
    stats->dirty_evictions = 0;

    
}
/**
 * @brief free stats
*/
void freeStats(void){
    free(stats);
}

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
            cache[i][j].valid=false;
            cache[i][j].tag = 0;
            cache[i][j].time = 0;
            cache[i][j].dirty=false;
        }
    }



    


}

/**
 * free cache memory
*/
void freeCache(void){
    for (unsigned long i = 0;i<set_number;i++){
        free(cache[i]);
    }

    free(cache);

}
/**
 * @brief access data and do the statistics
*/
void processData(char operation,unsigned long address,unsigned long size){
    unsigned long tag = address >> (set_bits+block_bits);

    unsigned setIndex = (address >> block_bits) &  ((1<<set_bits)-1);
    //extract bits

    // hits option
    for (unsigned long i=0;i<associativity;i++){

        if(cache[setIndex][i].valid==true && cache[setIndex][i].tag == tag){
            stats->hits++;
            LRU_timer++;
            cache[setIndex][i].time = LRU_timer;

            if(operation == 'S'){
                if(cache[setIndex][i].dirty==false){
                    stats->dirty_bytes+=block_size;
                    cache[setIndex][i].dirty=true;

                }

                

            }

            if(is_v_mode)
                printf("hits\n");
            
            
            

            return;

        }
    }
    

    // miss part
    stats->misses++;

    for (unsigned long i=0;i<associativity;i++){
        if(cache[setIndex][i].valid==false){
            cache[setIndex][i].valid = true;
            cache[setIndex][i].tag = tag;
            LRU_timer++;
            cache[setIndex][i].time = LRU_timer;
           if(operation == 'S'){
                 cache[setIndex][i].dirty = true;
                 stats->dirty_bytes+=block_size;
                
            }
            if(is_v_mode)
                printf("miss\n");

            return;

        }
    }

    // eviction operation

    unsigned long min = cache[setIndex][0].time;
    unsigned long min_timer_index = 0;

    for (unsigned long i =0;i<associativity;i++){
        if(cache[setIndex][i].time < min){
            min = cache[setIndex][i].time;
            min_timer_index = i;
        }
    }

    if(operation=='S'&&cache[setIndex][min_timer_index].dirty){
        stats->dirty_evictions++;
        stats->dirty_bytes--;
        cache[setIndex][min_timer_index].dirty=false;

    }


    cache[setIndex][min_timer_index].tag = tag;
    cache[setIndex][min_timer_index].valid = true;
    LRU_timer++;
    cache[setIndex][min_timer_index].time = LRU_timer;


   
    stats->evictions++;

    if(is_v_mode)
                printf("eviction\n");





    
    
    

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
        char operation = *strtok(linebuf," ,");
        unsigned long address = strtoul(strtok(NULL," ,"),NULL,HEX_BASE);
        unsigned long size = strtoul(strtok(NULL," ,"),NULL,HEX_BASE);

        if(is_v_mode)
                printf("%c %lu, %lu ", operation, address, size);

        processData(operation,address,size);



    
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
                printf("This is v mode\n");
                is_v_mode = true;
            
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
    initStats();
    process_trace_file(file_name);

    printSummary(stats);


    freeCache();
    freeStats();


    
    
    return 0;
}
