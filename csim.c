/**
 *  A cache simulator that gives the statistics of hits, misses,
 * evictions, dirty bytes, and dirty evictions。 This command line program
 * also handle invalid input and invalid address operation
 *
 */

#include "cachelab.h"
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** @brief Lenth of maximum string length for reading from trace file */
#define LINELEN 22
/** @brief Decimal base number */
#define DECIMAL_BASE 10
/** @brief Hex base number */
#define HEX_BASE 16

/**
 * @brief cache_line struct
 */
typedef struct {
    bool valid; /*valid bit in cache,if there is a address in the cache_line,
                   true, otherwise false*/
    bool dirty; /*dirty bit in cache, true if it is dirty, otherwist false*/
    unsigned long tag;  /*tag in cache_line*/
    unsigned long time; /*LRU_information*/

} cache_line;

cache_line **cache; /*cache 2D matrix*/

long set_number;          /*number of set*/
unsigned long block_size; /*size of block*/
long associativity = 0;   /*number of cache_line in one set*/
long set_bits;            /*number of set bits*/
long block_bits;          /*number of block bits*/
char *file_name = NULL;   /*trace file name*/
csim_stats_t
    *stats; /*stat struct to give the statists about hit, miss,evictions, dirty
               bytes and byte evicted from dirty lines*/

bool is_v_mode = false; /* Enable verbose mode, true if it is in verbose mode,
                           by defalue it is false*/

unsigned long LRU_timer = 0; /*global LRU_counter*/

/**
 * @brief Initialize statistics variable to 0
 */
void initStats(void) {
    stats = malloc(sizeof(csim_stats_t));
    if (stats == NULL) {
        printf("Failed to allocate memory\n");
        exit(1);
    }
    stats->hits = 0;
    stats->misses = 0;
    stats->evictions = 0;
    stats->dirty_bytes = 0;
    stats->dirty_evictions = 0;
}
/**
 * @brief free stats memory
 */
void freeStats(void) {
    free(stats);
}

/**
 * @brief Initialize Cache 2d matrix
 *
 * Get set number and block size. Allocate memory for the cache based on the set
 * number and block, and initialize the cache
 */
void initCache(void) {
    set_number = 1 << set_bits;
    block_size = 1 << block_bits;

    cache = malloc((unsigned long)set_number * sizeof(cache_line *));

    /* Error handling for memory allication failed */
    if (cache == NULL) {
        printf("Failed to allocate memory\n");
        exit(1);
    }

    for (long i = 0; i < set_number; i++) {
        cache[i] = malloc((unsigned long)associativity * sizeof(cache_line));
        if (cache == NULL) {
            printf("Failed to allocate memory\n");
            exit(1);
        }

        for (long j = 0; j < associativity; j++) {
            cache[i][j].valid = false;
            cache[i][j].tag = 0;
            cache[i][j].time = 0;
            cache[i][j].dirty = false;
        }
    }
}

/**
 * @brief free cache memory
 */
void freeCache(void) {
    for (long i = 0; i < set_number; i++) {
        free(cache[i]);
    }

    free(cache);
}
/**
 * @brief check if the operation is hit
 *
 * Use tag and set_index to return the index of address if it is in the cache
 *
 * @param tag       tag information
 * @param set_index set index information
 * @return          -1 if it is not a hit, otherwise return index of the hit
 *
 *
 */
long findHit(unsigned long tag, long set_index) {
    for (long i = 0; i < associativity; i++) {

        if (cache[set_index][i].valid == true &&
            cache[set_index][i].tag == tag) {
            return i;
        }
    }
    return -1;
}
/**
 * @brief check if the operation is miss
 *
 * Use tag and set_index to get the index of address if it is in the cache
 *
 * @param tag       tag information
 * @param set_index set index information
 * @return          -1 if it is not a miss, otherwise return index of the miss
 *
 *
 */
long findMiss(unsigned long tag, long set_index) {
    for (long i = 0; i < associativity; i++) {

        if (cache[set_index][i].valid == false) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief get the index of evicted address
 *
 * Use tag and set_index to get the index of address if it need to be evicted
 *
 * @param tag       tag information
 * @param set_index set index information
 * @return          index of the address that need to be evicted
 *
 *
 */
long findEviction(unsigned long tag, long set_index) {
    unsigned long min = cache[set_index][0].time;
    long min_timer_index = 0;
    /* use LRU information to find the least use address */
    for (long i = 0; i < associativity; i++) {
        if (cache[set_index][i].time < min) {
            min = cache[set_index][i].time;
            min_timer_index = i;
        }
    }
    return min_timer_index;
}

/**
 * @brief upate the statist information for the cache hit based on store and
 * load operation
 *
 * @param set_index set index information
 * @param operation operation char: S for store, L for read
 * @param hit_index hit index in the cache
 *
 *
 *
 */
void handleHit(long set_index, char operation, long hit_index) {
    /* update statist information */
    stats->hits++;
    /* update LRU information*/
    LRU_timer++;
    cache[set_index][hit_index].time = LRU_timer;

    /*If the operation is store, update dirty byte statistic*/
    if (operation == 'S') {
        if (cache[set_index][hit_index].dirty == false) {
            stats->dirty_bytes += block_size;
            cache[set_index][hit_index].dirty = true;
        }
    }
}

/**
 * @brief upate the statist information for the cache miss based on store and
 * load operation
 *
 * save the address in the cache
 *
 * @param set_index set index information
 * @param operation operation char: S for store, L for read
 * @param hit_index hit index in the cache
 *
 *
 *
 */
void handleMiss(long set_index, unsigned long tag, char operation,
                long miss_index) {
    cache[set_index][miss_index].valid = true;
    cache[set_index][miss_index].tag = tag;
    LRU_timer++;
    cache[set_index][miss_index].time = LRU_timer;
    /* if the operation is store, update the dirty byte statistics*/
    if (operation == 'S') {
        cache[set_index][miss_index].dirty = true;
        stats->dirty_bytes += block_size;
    }
}

/**
 * @brief upate the statist information for the cache eciction based on store
 * and load operation
 *
 * @param set_index set index information
 * @param operation operation char: S for store, L for read
 * @param hit_index hit index in the cache
 *
 *
 *
 */
void handleEviction(long set_index, unsigned long tag, char operation) {
    long eviction_index = findEviction(tag, set_index);

    /*if the address that need to be evicted is dirty, update the statistics,*/
    if (cache[set_index][eviction_index].dirty) {
        stats->dirty_evictions += block_size;
        stats->dirty_bytes -= block_size;
    }
    if (operation == 'S') {
        cache[set_index][eviction_index].dirty = true;
        stats->dirty_bytes += block_size;

    } else {
        cache[set_index][eviction_index].dirty = false;
    }
    /*put new address in the cache at position of evicted address*/
    cache[set_index][eviction_index].tag = tag;
    cache[set_index][eviction_index].valid = true;
    LRU_timer++;
    cache[set_index][eviction_index].time = LRU_timer;

    stats->evictions++;
}

/**
 * @brief process data and do the statistics based on different operation
 * Process data address and generate the set index and tag
 * and check if it is a hit, miss, or eviction,
 * and update statistics based on the previous result
 * @param operation operation char: S for store, L for read
 * @param address unsiged 64 bits address
 *
 */
void processData(char operation, unsigned long address) {

    /* extract tag and set index from address*/
    unsigned long tag = address >> (set_bits + block_bits);

    long set_index = (address >> block_bits) & ((1 << set_bits) - 1);

    /*check if it is hit*/
    long hit_index = findHit(tag, set_index);

    if (hit_index != -1) {
        handleHit(set_index, operation, hit_index);

        if (is_v_mode)
            printf("hits\n");

        return;
    }
    /*if the code run this section, it means a miss so update the miss
     * information*/
    stats->misses++;

    long miss_index = findMiss(tag, set_index);

    if (miss_index != -1) {
        handleMiss(set_index, tag, operation, miss_index);

        if (is_v_mode)
            printf("miss\n");

        return;
    }

    /*if the code run this section, it means a eviction so update the eviction
     * information*/
    handleEviction(set_index, tag, operation);

    if (is_v_mode)
        printf("eviction\n");
}

/** @brief Process a memory-access trace file.
 *
 * @param trace Name of the trace file to process
 * @return 0 if successful, 1 if there were error
 */
int process_trace_file(const char *trace) {

    FILE *tfp = fopen(trace, "rt");
    if (!tfp) {
        fprintf(stderr, "Error opening '%s': %s\n", trace, strerror(errno));
        exit(1);
    }
    char linebuf[LINELEN];
    int parse_error = 0;
    while (fgets(linebuf, LINELEN, tfp)) {
        /*Check Invalid operation otherthan store or read*/
        if (linebuf[0] != 'S' && linebuf[0] != 'L') {
            printf("%c\n", linebuf[0]);
            printf("Invalid operation or address in trace file\n");

            exit(1);
        }

        /*read operation, address, and size*/
        char operation = linebuf[0];

        unsigned long address;

        unsigned long size;

        int ret = sscanf(linebuf + 1, " %lx,%lu", &address, &size);

        /*If the number of return value is not 2, meaning there is an error for
         * reading files*/
        if (ret != 2) {
            printf("Error reading trace file\n");
            exit(1);
        }
        /*The size should not greater than 64*/
        if (size > 64) {
            printf("Invalid size\n");
            exit(1);
        }
        /*enable verobase mode for debug using, show the each operatio hit, miss
         * or eviction*/
        if (is_v_mode)
            printf("%c %lu, %lu ", operation, address, size);

        processData(operation, address);
    }
    fclose(tfp);
    return parse_error;
}

/**
 * @brief print help message
 */
void printHelp(void) {
    printf("Usage: ./csim [-v] -s <s> -b <b> -E <E> -t <trace>\n");
    printf("       ./csim -h\n");
    printf("    -h          Print this help message and exit\n");
    printf("    -v          Verbose mode: report effects of each memory "
           "operation\n");
    printf("    -s <s>      Number of set index bits (there are 2**s sets)\n");
    printf("    -b <b>      Number of block bits (there are 2**b blocks)\n");
    printf("    -E <E>      Number of lines per set (associativity)\n");
    printf("    -t <trace>  File name of the memory trace to process\n\n");
    printf("The -s, -b, -E, and -t options must be supplied for all "
           "simulations.\n");
}

/**
 * @brief main function
 *
 * @param [in] argv Command Line argument,  [-v] -s <s> -E <E> -b <b> -t <trace>
 */
int main(int argc, char *argv[]) {
    int opt;
    /*read commamd line argument, -s for set bits, -E for asssociativity,
     -b for block bits, -t for file name*/
    while ((opt = getopt(argc, argv, "vhs:E:b:t:")) != -1) {
        switch (opt) {
        case 'v':
            printf("This is v mode\n");
            is_v_mode = true;

            break;

        case 'h':
            printHelp();
            break;

        case 's':
            set_bits = strtol(optarg, NULL, DECIMAL_BASE);
            break;

        case 'E':
            associativity = strtol(optarg, NULL, DECIMAL_BASE);
            break;

        case 'b':
            block_bits = strtol(optarg, NULL, DECIMAL_BASE);

            break;

        case 't':
            file_name = optarg;
            break;

        case ':':
            printf("Mandatory arguments missing or zero.\n");
            printHelp();
            exit(1);
            break;

        case '?':
            printf("Error while parsing arguments.");
            printHelp();
            exit(1);

            break;
        }
    }

    /*associativity and file name cannot be zero or null, exit the program if it
     * does not meet the requirment*/
    if (associativity == 0 || file_name == NULL) {
        printf("Mandatory arguments missing or zero.\n");
        printHelp();
        exit(1);
    }

    /*All set_bits, block bits, should not be negative,
    and the number of set_block and block bits cannot be greater than address,
    and there must be a tag bits */
    if (set_bits < 0 || block_bits < 0 || set_bits + block_bits > 63) {
        printf("Error: s + b is too large (s = %lu, b = %lu)\n", set_bits,
               block_bits);
        exit(1);
    }
    /*associativity can not be less 1*/
    if (associativity < 1) {

        printf("Failed to allocate memory\n");
        exit(1);
    }
    /*Do the simulation*/

    initCache();
    initStats();
    process_trace_file(file_name);

    printSummary(stats);

    freeCache();
    freeStats();

    return 0;
}
